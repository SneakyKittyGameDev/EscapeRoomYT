// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractableInterface.h"
#include "UnlockingKey.generated.h"

UCLASS()
class ESCAPEROOMYT_API AUnlockingKey : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUnlockingKey();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* KeyMesh;

	UPROPERTY(ReplicatedUsing = OnRep_PickedUp)
	bool bPickedUp;

	UFUNCTION()
	void OnRep_PickedUp();

public:
	virtual bool Interact(class AEscapeRoomYTCharacter* Character) override;
};
