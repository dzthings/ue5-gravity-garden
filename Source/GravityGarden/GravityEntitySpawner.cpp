#include "GravityEntitySpawner.h"
#include "GravityEntityPawn.h"
#include "GravityEntityComponent.h"
#include "GravityEntityProfile.h"

AGravityEntitySpawner::AGravityEntitySpawner()
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void AGravityEntitySpawner::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World) return;

	FRandomStream Rng(Seed);
	TArray<FVector> PlacedPoints;

	for (const FGravitySpawnEntry& Entry : Entries)
	{
		if (!Entry.Profile) continue;

		for (int32 i = 0; i < Entry.Count; ++i)
		{
			FVector SpawnPoint = PickSpawnPoint(Rng, PlacedPoints);
			PlacedPoints.Add(SpawnPoint);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AGravityEntityPawn* Pawn = World->SpawnActor<AGravityEntityPawn>(
				AGravityEntityPawn::StaticClass(),
				SpawnPoint,
				FRotator::ZeroRotator,
				Params);

			if (!Pawn) continue;

			if (UGravityEntityComponent* GEC = Pawn->GravityEntityComponent)
			{
				GEC->Profile      = Entry.Profile;
				GEC->NodeMaterial = EntityMaterial;
				GEC->LinkMaterial = EntityMaterial;
			}
		}
	}
}

FVector AGravityEntitySpawner::PickSpawnPoint(FRandomStream& Rng, const TArray<FVector>& Placed) const
{
	const FVector Origin = GetActorLocation();
	const int32 MaxAttempts = 30;

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		float Angle = Rng.FRandRange(0.f, UE_TWO_PI);
		float Dist  = Rng.FRandRange(0.f, SpawnRadius);
		FVector Candidate = Origin + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);

		bool bTooClose = false;
		for (const FVector& P : Placed)
		{
			if (FVector::Dist2D(Candidate, P) < MinSpacing)
			{
				bTooClose = true;
				break;
			}
		}

		if (!bTooClose) return Candidate;
	}

	// Fallback: just use a random point even if too close
	float Angle = Rng.FRandRange(0.f, UE_TWO_PI);
	float Dist  = Rng.FRandRange(0.f, SpawnRadius);
	return Origin + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
}
