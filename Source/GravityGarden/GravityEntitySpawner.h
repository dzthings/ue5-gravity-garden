#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GravityEntitySpawner.generated.h"

class UGravityEntityProfile;
class AGravityEntityPawn;

// One entry in the spawner — a profile and how many instances to create.
USTRUCT(BlueprintType)
struct FGravitySpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TObjectPtr<UGravityEntityProfile> Profile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "1", ClampMax = "20"))
	int32 Count = 3;

	// Z offset from the spawner's location (cm). Use for floating families (orbitals, flora elevated above ground).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnHeight = 0.f;
};

// Place one of these in the level and configure Entries.
// At BeginPlay it spawns all entities at random positions within SpawnRadius,
// replacing the need to manually place and configure each pawn.
UCLASS()
class GRAVITYGARDEN_API AGravityEntitySpawner : public AActor
{
	GENERATED_BODY()

public:
	AGravityEntitySpawner();

	// Profiles to spawn and how many of each.
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TArray<FGravitySpawnEntry> Entries;

	// Random placement radius around this actor (cm).
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "100.0"))
	float SpawnRadius = 1000.f;

	// Minimum spacing between spawn points — keeps entities from starting on top of each other (cm).
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "50.0"))
	float MinSpacing = 300.f;

	// Material applied to every spawned entity's node and link meshes.
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TObjectPtr<UMaterialInterface> EntityMaterial;

	// Deterministic seed — same seed always produces the same layout.
	UPROPERTY(EditAnywhere, Category = "Spawner")
	int32 Seed = 42;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot;

	FVector PickSpawnPoint(FRandomStream& Rng, const TArray<FVector>& Placed) const;
};
