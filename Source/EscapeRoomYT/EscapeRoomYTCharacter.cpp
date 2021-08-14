// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeRoomYTCharacter.h"
#include "Interfaces/InteractableInterface.h"
#include "Actors/Door.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

AEscapeRoomYTCharacter::AEscapeRoomYTCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh(), FName("head"));
	CameraBoom->TargetArmLength = 0.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->SetRelativeLocation(FVector(5.0f, 20.0f, 0.0f));
	
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	bUseControllerRotationYaw = true;

	InteractionDistance = 500.0f;
	HeldItemDistance = 0.0f;

	CharacterMass = 90.0f;

	bIsExamining = false;
}

void AEscapeRoomYTCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AEscapeRoomYTCharacter::Interact);
	PlayerInputComponent->BindAction("MoveItemCloser", IE_Pressed, this, &AEscapeRoomYTCharacter::MoveItemCloser);
	PlayerInputComponent->BindAction("MoveItemFurther", IE_Pressed, this, &AEscapeRoomYTCharacter::MoveItemFurther);

	PlayerInputComponent->BindAction("Examine", IE_Pressed, this, &AEscapeRoomYTCharacter::SetExamineItem);
	PlayerInputComponent->BindAction("Examine", IE_Released, this, &AEscapeRoomYTCharacter::StopExamineItem);

	PlayerInputComponent->BindAxis("MoveForward", this, &AEscapeRoomYTCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AEscapeRoomYTCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &AEscapeRoomYTCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("TurnRate", this, &AEscapeRoomYTCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AEscapeRoomYTCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AEscapeRoomYTCharacter::LookUpAtRate);
}

void AEscapeRoomYTCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		GetMesh()->HideBoneByName(FName("head"), EPhysBodyOp::PBO_None);
	}
}

void AEscapeRoomYTCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEscapeRoomYTCharacter, HeldItem);
	DOREPLIFETIME_CONDITION(AEscapeRoomYTCharacter, Inventory, COND_OwnerOnly);
}

void AEscapeRoomYTCharacter::AddItemToInventory(AActor* Item)
{
	if (HasAuthority() && Item)
	{
		Inventory.Add(Item);
	}
}

void AEscapeRoomYTCharacter::OpenDoor(ADoor* Door)
{
	if (!Door)
	{
		return;
	}
	
	if (HasAuthority())
	{
		Door->ForceInteract();
	}
	else
	{
		Server_OpenDoor(Door);
	}
}

bool AEscapeRoomYTCharacter::Server_OpenDoor_Validate(ADoor* Door)
{
	return true;
}

void AEscapeRoomYTCharacter::Server_OpenDoor_Implementation(ADoor* Door)
{
	OpenDoor(Door);
}

void AEscapeRoomYTCharacter::Interact()
{
	if (HeldItem)
	{
		DropItem(HeldItem->GetActorRotation());
	}
	else
	{
		FVector Start = GetFollowCamera()->GetComponentLocation();
		FVector End = Start + GetFollowCamera()->GetForwardVector() * InteractionDistance;
		Interact(Start, End);
	}
}

void AEscapeRoomYTCharacter::Interact(FVector LineStart, FVector LineEnd)
{
	FHitResult HitResult;
	
	if (PerformLineTrace(HitResult, LineStart, LineEnd, true))
	{
		if (HasAuthority())
		{
			if (IInteractableInterface* Interface = Cast<IInteractableInterface>(HitResult.GetActor()))
			{
				if (Interface->Interact(this))
				{
					OpenKeyCode(Cast<ADoor>(HitResult.GetActor()));
				}
			}
		}
		else
		{
			if (IInteractableInterface* Interface = Cast<IInteractableInterface>(HitResult.GetActor()))
			{
				if (Interface->Interact(this))
				{
					OpenKeyCode(Cast<ADoor>(HitResult.GetActor()));
				}
				else
				{
					Server_Interact(LineStart, LineEnd);
				}
			}
		}
	}
}

void AEscapeRoomYTCharacter::DropItem(FRotator NewRotation)
{
	if (HeldItem)
	{
		if (HasAuthority())
		{
			HeldItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			HeldItem->SetActorRotation(NewRotation);
			if (IInteractableInterface* Interface = Cast<IInteractableInterface>(HeldItem))
			{
				Interface->EnablePhysics();
			}
			HeldItem = nullptr;
		}
		else
		{
			Server_Drop(HeldItem->GetActorRotation());
		}
	}
}

bool AEscapeRoomYTCharacter::Server_Interact_Validate(FVector LineStart, FVector LineEnd)
{ // Camera Location is within distance of Servers Follow Camera
	return UKismetMathLibrary::Vector_Distance(LineStart, GetFollowCamera()->GetComponentLocation()) < 150.0f;
}

void AEscapeRoomYTCharacter::Server_Interact_Implementation(FVector LineStart, FVector LineEnd)
{
	Interact(LineStart, LineEnd);
}

bool AEscapeRoomYTCharacter::Server_Drop_Validate(FRotator NewRotation)
{
	return true;
}

void AEscapeRoomYTCharacter::Server_Drop_Implementation(FRotator NewRotation)
{
	DropItem(NewRotation);
}

bool AEscapeRoomYTCharacter::PerformLineTrace(FHitResult& HitResult, FVector Start, FVector End, bool DrawDebug)
{
	if (DrawDebug)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 3.0f, 0, 3.0f);
	}
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	return GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_WorldStatic, Params);
}

void AEscapeRoomYTCharacter::MoveItemCloser()
{
	if (HeldItem)
	{
		HeldItemDistance += 5.0f;
		FVector SocketLocation = GetMesh()->GetSocketLocation(FName("s_itemAttach"));
		FRotator ToCameraRotation = UKismetMathLibrary::FindLookAtRotation(SocketLocation, GetFollowCamera()->GetComponentLocation());
		NewItemLocation = SocketLocation + ToCameraRotation.Vector() * HeldItemDistance;
		//HeldItem->SetActorLocation(NewItemLocation);
		Server_MoveHeldItem(HeldItemDistance);
		
		GetWorldTimerManager().ClearTimer(TInterpItem);
		//GetWorldTimerManager().SetTimer(TInterpItem, this, &AEscapeRoomYTCharacter::InterpolateHeldItem, 0.1f, true);
	}
}

void AEscapeRoomYTCharacter::MoveItemFurther()
{
	if (HeldItem)
	{
		HeldItemDistance -= 5.0f;
		FVector SocketLocation = GetMesh()->GetSocketLocation(FName("s_itemAttach"));
		FRotator ToCameraRotation = UKismetMathLibrary::FindLookAtRotation(SocketLocation, GetFollowCamera()->GetComponentLocation());
		NewItemLocation = SocketLocation + ToCameraRotation.Vector() * HeldItemDistance;
		//HeldItem->SetActorLocation(NewItemLocation);
		Server_MoveHeldItem(HeldItemDistance);
		
		GetWorldTimerManager().ClearTimer(TInterpItem);
		//GetWorldTimerManager().SetTimer(TInterpItem, this, &AEscapeRoomYTCharacter::InterpolateHeldItem, 0.005f, true);
	}
}

void AEscapeRoomYTCharacter::SetExamineItem()
{
	if (HeldItem)
	{
		bIsExamining = true;
	}
}

void AEscapeRoomYTCharacter::StopExamineItem()
{
	bIsExamining = false;
}

void AEscapeRoomYTCharacter::InterpolateHeldItem()
{
	if (HeldItem)
	{
		FVector NewLocation = UKismetMathLibrary::VInterpTo(HeldItem->GetActorLocation(), NewItemLocation, GetWorld()->GetDeltaSeconds(), 8.0f);
		HeldItem->SetActorLocation(NewLocation);
		if (FVector::Distance(NewLocation, NewItemLocation) < 1.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("FINISHED INTERPOLATION"));
			GetWorldTimerManager().ClearTimer(TInterpItem);
		}
	}
}

bool AEscapeRoomYTCharacter::Server_MoveHeldItem_Validate(float NewItemDistance)
{
	return true;
}

void AEscapeRoomYTCharacter::Server_MoveHeldItem_Implementation(float NewItemDistance)
{
	if (HeldItem)
	{
		FVector SocketLocation = GetMesh()->GetSocketLocation(FName("s_itemAttach"));
		FRotator ToCameraRotation = UKismetMathLibrary::FindLookAtRotation(SocketLocation, GetFollowCamera()->GetComponentLocation());
		NewItemLocation = SocketLocation + ToCameraRotation.Vector() * NewItemDistance;
		HeldItem->SetActorLocation(NewItemLocation);
	}
}

void AEscapeRoomYTCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AEscapeRoomYTCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AEscapeRoomYTCharacter::TurnAtRate(float Rate)
{
	if (bIsExamining)
	{
		if (HeldItem && Rate != 0.0f)
		{
			FRotator CurrentRotation = HeldItem->GetActorRotation();
			CurrentRotation.Yaw += Rate * 2;
			HeldItem->SetActorRotation(CurrentRotation);
		}
	}
	else
	{
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AEscapeRoomYTCharacter::LookUpAtRate(float Rate)
{
	if (bIsExamining)
	{
		if (HeldItem && Rate != 0.0f)
		{
			FRotator CurrentRotation = FRotator(Rate * 2, 0.0f, 0.0f);
			UE_LOG(LogTemp, Warning, TEXT("PITCH: %f"), CurrentRotation.Pitch);
			HeldItem->AddActorLocalRotation(CurrentRotation);
		}
	}
	else
	{
		AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
}