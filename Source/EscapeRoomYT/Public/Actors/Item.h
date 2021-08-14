// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractableInterface.h"
#include "Item.generated.h"

class UStaticMeshComponent;

UCLASS()
class ESCAPEROOMYT_API AItem : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	UPROPERTY(EditAnywhere, Category = "EscapeRoom Item")
	UStaticMeshComponent* MeshComponent;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual bool Interact(class AEscapeRoomYTCharacter* Character) override;
	virtual void EnablePhysics() override;
	virtual void DisablePhysics() override;

	UStaticMeshComponent* GetMesh() const  {return MeshComponent;}
};
