#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GravityFieldRegistry.generated.h"

class UGravityFloraMovementSolver;

// World subsystem that brokers fauna node positions to flora solvers each tick.
// Fauna components register their node arrays; flora solvers query positions
// within their attraction radius without needing direct cross-actor references.
UCLASS()
class GRAVITYENTITYRUNTIME_API UGravityFieldRegistry : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Fauna calls this each tick with its current world-space node positions.
	void BroadcastFaunaPositions(const TArray<FVector>& NodePositions);

	// Flora solvers register once at init; receive positions via BroadcastFaunaPositions.
	void RegisterFloraReceiver(UGravityFloraMovementSolver* Solver);
	void UnregisterFloraReceiver(UGravityFloraMovementSolver* Solver);

private:
	TArray<TWeakObjectPtr<UGravityFloraMovementSolver>> FloraReceivers;
	TArray<FVector> AccumulatedPositions;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UGravityFieldRegistry, STATGROUP_Tickables); }
};
