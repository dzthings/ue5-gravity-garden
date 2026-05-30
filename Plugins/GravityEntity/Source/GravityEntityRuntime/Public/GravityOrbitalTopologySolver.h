#pragma once

#include "CoreMinimal.h"
#include "GravityTopologySolver.h"
#include "GravityOrbitalTopologySolver.generated.h"

// Arranges nodes on a Fibonacci sphere around a central Core node.
// Node 0 = Core; remaining nodes = Orbital, evenly distributed on the sphere surface.
// Links: Core -> each Orbital (spoke pattern).
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravityOrbitalTopologySolver : public UGravityTopologySolver
{
	GENERATED_BODY()

public:
	// Radius of the orbital sphere (cm).
	UPROPERTY(EditAnywhere, Category = "Topology", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
	float OrbitRadius = 150.f;

	virtual void BuildTopology(TArray<FGravityNode>& OutNodes, TArray<FGravityLink>& OutLinks,
	                           const UGravityEntityProfile* Profile) override;
};
