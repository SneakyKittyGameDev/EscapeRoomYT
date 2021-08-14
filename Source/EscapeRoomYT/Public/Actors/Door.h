// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractableInterface.h"

#include "Door.generated.h"

UCLASS()
class ESCAPEROOMYT_API ADoor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere)
	class UAnimationAsset* OpenAnimation;
	UPROPERTY(EditAnywhere)
	class UAnimationAsset* CloseAnimation;

	UPROPERTY(Replicated)
	bool bOpen;

	UPROPERTY(EditAnywhere)
	bool bStartLocked;

	bool bLocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EscapeRoom | Key")
	AActor* LinkedKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EscapeRoom | Key")
	AActor* LinkedStoredItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EscapeRoom | Key")
	FString DoorKeyCode;
	
	UPROPERTY(Replicated)
	float OpenPercentage;

public:
	void SetDoorPercentage(float Weight);

	UFUNCTION(BlueprintCallable, Category = "EscapeRoom | Door")
	float GetOpenPercentage() { return OpenPercentage; }
	
	virtual bool Interact(class AEscapeRoomYTCharacter* Character) override;

	UFUNCTION(BlueprintCallable, Category = "EscapeRoom | Door")
	bool TryDoor(const FString& DoorCode);

	void ForceInteract();
};
