// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EscapeRoomYTCharacter.generated.h"

UCLASS(config=Game)
class AEscapeRoomYTCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AEscapeRoomYTCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:
	UPROPERTY(Replicated)
	AActor* HeldItem;

	UPROPERTY(Replicated)
	TArray<AActor*> Inventory;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintImplementableEvent, Category = "EscapeRoom | Door")
	void OpenKeyCode(class ADoor* Door);

	UFUNCTION(BlueprintCallable, Category = "EscapeRoom | Door")
	void OpenDoor(class ADoor* Door);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_OpenDoor(class ADoor* Door);
	
	UPROPERTY(EditDefaultsOnly, Category = "EscapeRoom Character")
	float InteractionDistance;
	
	void Interact();

	void Interact(FVector LineStart, FVector LineEnd);

	void DropItem(FRotator NewRotation);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact(FVector LineStart, FVector LineEnd);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Drop(FRotator NewRotation);

	bool PerformLineTrace(FHitResult& HitResult, FVector Start, FVector End, bool DrawDebug = false);

	void MoveItemCloser();
	void MoveItemFurther();
	float HeldItemDistance;
	FVector NewItemLocation;
	FTimerHandle TInterpItem;

	void SetExamineItem();
	void StopExamineItem();
	bool bIsExamining;
	
	float CharacterMass;

	void InterpolateHeldItem();

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_MoveHeldItem(float NewItemDistance);

	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	void SetHeldItem(AActor* Item) { HeldItem = Item; }

	void AddItemToInventory(AActor* Item);

	UFUNCTION(BlueprintCallable)
	TArray<AActor*> GetInventory() { return Inventory; }

	float GetMass() const {return CharacterMass;}
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

