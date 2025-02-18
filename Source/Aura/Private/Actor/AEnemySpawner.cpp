// Copyright Vince Bracken

#include "Actor/AEnemySpawner.h"
#include "Character/AuraEnemy.h" 
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h" // for RandomPointInBoundingBox, if desired
#include "GeometryCollection/GeometryCollectionComponent.h"
#include <Actor/AuraEnemySpawnPoint.h>

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true; // If you need Tick; otherwise set to false

	GC_EnemySpawnerMesh = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GC_EnemySpawnerMesh"));
	SetRootComponent(GC_EnemySpawnerMesh);

	// Set up collision for the volume
	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(RootComponent);
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionObjectType(ECC_WorldStatic);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Default values
	MaxEnemies = 5;
	InitialSpawnCount = 3;
	NumEnemiesAlive = 0;
	bReplenishOnDeath = true;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	// Register overlap event
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AEnemySpawner::OnTriggerVolumeOverlap);
}

void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// You can put extra logic here if you'd like to dynamically spawn
	// or check conditions while the game is running.
}

void AEnemySpawner::OnTriggerVolumeOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (bReplenishOnDeath)
	{
		// For example: spawn an initial batch
		int32 NumToSpawn = FMath::Min(InitialSpawnCount, MaxEnemies - NumEnemiesAlive);
		for (int32 i = 0; i < NumToSpawn; ++i)
		{
			SpawnOneEnemy();
		}
	}

}

void AEnemySpawner::OnSpawnedEnemyDestroyed(AActor* DestroyedEnemy)
{
	NumEnemiesAlive--;

	// If bReplenishOnDeath is true, keep re-spawning to maintain the max limit
	if (bReplenishOnDeath && NumEnemiesAlive < MaxEnemies)
	{
		SpawnOneEnemy();
	}
}

void AEnemySpawner::SpawnOneEnemy()
{

	// Spawn an AAuraEnemy via the spawn point
	if (SpawnPoints.Num() > 0)
	{
		// Choose a spawn point and ask it to spawn
		AAuraEnemySpawnPoint* SpawnPoint = SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)]; // or random index

		// Actually spawn the enemy
		AAuraEnemy* SpawnedEnemy = SpawnPoint->SpawnEnemy();
		// (Modify your SpawnEnemy() to return the spawned enemy reference.)

		if (SpawnedEnemy)
		{
			// Now bind the spawner's OnSpawnedEnemyDestroyed
			SpawnedEnemy->OnDestroyed.AddDynamic(this, &AEnemySpawner::OnSpawnedEnemyDestroyed);
			NumEnemiesAlive++;
		}
	}
}
