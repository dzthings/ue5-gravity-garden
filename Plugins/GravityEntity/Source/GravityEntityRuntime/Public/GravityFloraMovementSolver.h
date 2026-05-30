#pragma once

#include "CoreMinimal.h"
#include "GravityMovementSolver.h"
#include "GravityEntityTypes.h"
#include "GravityFloraMovementSolver.generated.h"

// Reactive movement solver for flora (rooted entities).
// Node 0 is pinned to its spawn position — it never moves.
// Higher nodes spring toward their rest positions but are displaced
// by a proximity field from nearby entity nodes registered each tick.
// The stalk bends toward passing entities and slowly settles back.
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravityFloraMovementSolver : public UGravityMovementSolver
{
	GENERATED_BODY()

public:
	// Spring stiffness pulling nodes toward rest position.
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float Stiffness = 120.f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Damping = 18.f;

	// How strongly nearby entity nodes pull the stalk tips (N per entity node within radius).
	UPROPERTY(EditAnywhere, Category = "Reaction", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float AttractionStrength = 80.f;

	// Distance at which an entity node starts influencing this stalk (cm).
	UPROPERTY(EditAnywhere, Category = "Reaction", meta = (ClampMin = "10.0", ClampMax = "2000.0"))
	float AttractionRadius = 400.f;

	// How much influence falls off with distance (1 = linear, 2 = quadratic).
	UPROPERTY(EditAnywhere, Category = "Reaction", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float FalloffExponent = 1.5f;

	// Sub-steps per tick.
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "1", ClampMax = "8"))
	int32 SubSteps = 2;

	// Called each tick by the spawner or world system with positions of nearby fauna nodes.
	void SetNearbyEntityPositions(const TArray<FVector>& Positions) { NearbyPositions = Positions; }

	virtual void Solve(TArray<FGravityNode>& Nodes, const TArray<FGravityLink>& Links,
	                   float DeltaTime, UGravityStateChannels* Channels) override;

	virtual void Reset() override { NearbyPositions.Reset(); }

private:
	TArray<FVector> NearbyPositions;
	TArray<FVector> RestPositions; // set on first Solve from initial node positions

	void StepSimulation(TArray<FGravityNode>& Nodes, float Dt);
};
