#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "AEnemySpawner.generated.h"

class UBoxComponent;
class AAuraEnemySpawnPoint;

/**
 * A simple enemy spawner that can be placed in the level.
 * Spawns a limited number of enemies, and can replenish if desired.
 */
UCLASS()
class AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

protected:
	/** The trigger volume used to detect when the player enters and activate the spawner. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	UBoxComponent* TriggerVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	UGeometryCollectionComponent* GC_EnemySpawnerMesh;

	/** The maximum number of enemies this spawner should maintain alive at once. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	int32 MaxEnemies;

	/** How many enemies to initially spawn when triggered (up to MaxEnemies). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	int32 InitialSpawnCount;

	/** If true, we replenish enemies as they are killed, never exceeding MaxEnemies. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Spawner")
	bool bReplenishOnDeath;

	/** Track the number of currently alive enemies from this spawner. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	int32 NumEnemiesAlive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TArray<AAuraEnemySpawnPoint*> SpawnPoints;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Called when something overlaps the trigger volume. */
	UFUNCTION()
	void OnTriggerVolumeOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void SpawnOneEnemy();


	/** Called when a spawned enemy is destroyed (so we can replenish or track counts). */
	UFUNCTION()
	void OnSpawnedEnemyDestroyed(AActor* DestroyedEnemy);

public:
	/** Called every frame (if needed). */
	virtual void Tick(float DeltaTime) override;
};
