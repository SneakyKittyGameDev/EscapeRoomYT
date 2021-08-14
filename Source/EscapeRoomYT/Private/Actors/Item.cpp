// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Item.h"
#include "EscapeRoomYT/EscapeRoomYTCharacter.h"

#include "Components/StaticMeshComponent.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = MeshComponent;

	bReplicates = true;
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

}

bool AItem::Interact(AEscapeRoomYTCharacter* Character)
{
	if (HasAuthority())
	{
		if (Character)
		{
			if (AEscapeRoomYTCharacter* OwningCharacter = Cast<AEscapeRoomYTCharacter>(GetOwner()))
			{
				if (Character != OwningCharacter)
				{
					OwningCharacter->SetHeldItem(nullptr);
				}
			}
			if (AEscapeRoomYTCharacter* OwningCharacter = Cast<AEscapeRoomYTCharacter>(Character))
			{
				OwningCharacter->SetHeldItem(this);
			}
			
			SetOwner(Character);
			DisablePhysics();
			this->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("s_itemAttach"));
		}
	}
	return false;
}

void AItem::EnablePhysics()
{
	MeshComponent->SetSimulatePhysics(true);
}

void AItem::DisablePhysics()
{
	MeshComponent->SetSimulatePhysics(false);
}
