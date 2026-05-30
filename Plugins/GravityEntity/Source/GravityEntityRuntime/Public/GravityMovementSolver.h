#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GravityEntityTypes.h"
#include "GravityMovementSolver.generated.h"

class UGravityStateChannels;

// Steps the spring-damper simulation and writes resulting state into channels.
// Subclasses define different motion personalities (worm, cluster, lattice…).
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravityMovementSolver : public UObject
{
	GENERATED_BODY()

public:
	virtual void Solve(TArray<FGravityNode>& Nodes, const TArray<FGravityLink>& Links,
	                   float DeltaTime, UGravityStateChannels* Channels) {}

	// Called by UGravityEntityComponent::InitializeEntity() each session.
	// Subclasses must reset all time accumulators and cached state here.
	virtual void Reset() {}
};
