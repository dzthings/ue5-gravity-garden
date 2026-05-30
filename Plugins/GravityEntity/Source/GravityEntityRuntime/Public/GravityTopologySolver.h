#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GravityEntityTypes.h"
#include "GravityTopologySolver.generated.h"

class UGravityEntityProfile;

// Builds the initial node graph and link topology for an entity.
// Subclasses implement different arrangement families (spine, cluster, lattice…).
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravityTopologySolver : public UObject
{
	GENERATED_BODY()

public:
	virtual void BuildTopology(TArray<FGravityNode>& OutNodes, TArray<FGravityLink>& OutLinks, const UGravityEntityProfile* Profile) {}
};
