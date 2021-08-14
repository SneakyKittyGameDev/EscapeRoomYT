// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/UnlockingKey.h"
#include "EscapeRoomYT/EscapeRoomYTCharacter.h"

#include "Net/UnrealNetwork.h"

// Sets default values
AUnlockingKey::AUnlockingKey()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Key Mesh"));
	RootComponent = KeyMesh;

	bReplicates = true;
	
	bPickedUp = false;
}

// Called when the game starts or when spawned
void AUnlockingKey::BeginPlay()
{
	Super::BeginPlay();
	
}

void AUnlockingKey::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUnlockingKey, bPickedUp);
}

void AUnlockingKey::OnRep_PickedUp()
{
	KeyMesh->SetHiddenInGame(true);
	KeyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool AUnlockingKey::Interact(AEscapeRoomYTCharacter* Character)
{
	if (HasAuthority() && Character)
	{
		Character->AddItemToInventory(this);
		bPickedUp = true;
		OnRep_PickedUp();
	}

	return false;
}