// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Door.h"
#include "EscapeRoomYT/EscapeRoomYTCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ADoor::ADoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));

	bReplicates = true;
	bOpen = false;
	bStartLocked = true;
	bLocked = bStartLocked;

	DoorKeyCode = "1234";
}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();
	bLocked = bStartLocked;

	if (HasAuthority() && LinkedStoredItem && MeshComponent->DoesSocketExist(FName("s_Item")))
	{
		LinkedStoredItem->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("s_Item"));
	}
}

void ADoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADoor, bOpen);
	DOREPLIFETIME(ADoor, OpenPercentage);
}

void ADoor::SetDoorPercentage(float Weight)
{
	if (HasAuthority())
	{
		OpenPercentage = Weight;
	}
}

bool ADoor::Interact(AEscapeRoomYTCharacter* Character)
{
	if (!DoorKeyCode.IsEmpty() && bLocked)
	{
		return true;
	}
	
	if (HasAuthority() && Character)
	{
		if (bLocked && LinkedKey)
		{
			for (AActor* Key : Character->GetInventory())
			{
				if (Key == LinkedKey)
				{
					bLocked = false;
					bOpen = !bOpen;
				}
			}
		}
		else if (!bLocked)
		{
			bOpen = !bOpen;
		}
		
		if (bOpen)
		{
			OpenPercentage = 1.0f;
		}
		else
		{
			OpenPercentage = 0.0f;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("%f"), OpenPercentage);
	return false;
}

bool ADoor::TryDoor(const FString& DoorCode)
{
	if (DoorCode == DoorKeyCode)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ADoor::ForceInteract()
{
	if (HasAuthority())
	{
		bOpen = !bOpen;
		bLocked = false;
		if (bOpen)
		{
			OpenPercentage = 1.0f;
		}
		else
		{
			OpenPercentage = 0.0f;
		}
	}
}
