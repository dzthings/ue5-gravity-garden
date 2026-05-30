#pragma once

#include "CoreMinimal.h"
#include "GravityTopologySolver.h"
#include "GravityFloraTopologySolver.generated.h"

// Builds a rooted vertical chain — node 0 is anchored to the ground,
// remaining nodes extend upward. Each node gets a slight random lean
// seeded per-stalk so stalks look individually distinct.
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravityFloraTopologySolver : public UGravityTopologySolver
{
	GENERATED_BODY()

public:
	// Random lean applied to initial node positions (cm) — makes each stalk unique.
	UPROPERTY(EditAnywhere, Category = "Topology", meta = (ClampMin = "0.0", ClampMax = "40.0"))
	float InitialLeanAmount = 12.f;

	// Seed for per-stalk lean variation.
	UPROPERTY(EditAnywhere, Category = "Topology")
	int32 LeanSeed = 0;

	virtual void BuildTopology(TArray<FGravityNode>& OutNodes, TArray<FGravityLink>& OutLinks,
	                           const UGravityEntityProfile* Profile) override;
};
