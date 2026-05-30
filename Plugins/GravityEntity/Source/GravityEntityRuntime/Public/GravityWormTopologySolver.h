#pragma once

#include "CoreMinimal.h"
#include "GravityTopologySolver.h"
#include "GravityWormTopologySolver.generated.h"

// Builds a linear spine of N nodes hanging downward from the actor origin.
// Node 0 = Lead (top), Node N/2 = Core, rest = Spine.
// Links connect consecutive pairs.
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravityWormTopologySolver : public UGravityTopologySolver
{
	GENERATED_BODY()

public:
	virtual void BuildTopology(TArray<FGravityNode>& OutNodes, TArray<FGravityLink>& OutLinks,
	                           const UGravityEntityProfile* Profile) override;
};
