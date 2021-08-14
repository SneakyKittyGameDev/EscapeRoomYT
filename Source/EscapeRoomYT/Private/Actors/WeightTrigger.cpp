// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/WeightTrigger.h"
#include "Actors/Door.h"
#include "Actors/Item.h"
#include "EscapeRoomYT/EscapeRoomYTCharacter.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AWeightTrigger::AWeightTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TriggerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TriggerMesh"));
	RootComponent = TriggerMesh;
	
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(TriggerMesh);

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AWeightTrigger::OnComponentBeginOverlap);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AWeightTrigger::OnComponentEndOverlap);
}

// Called when the game starts or when spawned
void AWeightTrigger::BeginPlay()
{
	Super::BeginPlay();
	
}

float AWeightTrigger::CalculateDoorPercentage()
{
	float TotalWeight = 0.0f;
	TArray<AActor*> Overlapped;
	BoxComponent->GetOverlappingActors(Overlapped);
	for (AActor* Actor : Overlapped)
	{
		if (AItem* Item = Cast<AItem>(Actor))
		{
			TotalWeight += Item->GetMesh()->GetMass();
		}
		else if (AEscapeRoomYTCharacter* Character = Cast<AEscapeRoomYTCharacter>(Actor))
		{
			TotalWeight += Character->GetMass();
		}
	}
	float DoorPercent = UKismetMathLibrary::NormalizeToRange(TotalWeight, 0.0f, 150.0f);
	return UKismetMathLibrary::FClamp(DoorPercent, 0.0f, 1.0f);
}

void AWeightTrigger::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("OVERLAPPING"));
	if (HasAuthority() && LinkedDoor)
	{
		LinkedDoor->SetDoorPercentage(CalculateDoorPercentage());
	}
}

void AWeightTrigger::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("STOPPED OVERLAPPING"));
	if (HasAuthority() && LinkedDoor)
	{
		LinkedDoor->SetDoorPercentage(CalculateDoorPercentage());
	}
}