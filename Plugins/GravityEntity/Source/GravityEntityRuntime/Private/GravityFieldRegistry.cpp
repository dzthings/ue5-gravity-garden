#include "GravityFieldRegistry.h"
#include "GravityFloraMovementSolver.h"

void UGravityFieldRegistry::BroadcastFaunaPositions(const TArray<FVector>& NodePositions)
{
	AccumulatedPositions.Append(NodePositions);
}

void UGravityFieldRegistry::RegisterFloraReceiver(UGravityFloraMovementSolver* Solver)
{
	if (Solver) FloraReceivers.AddUnique(Solver);
}

void UGravityFieldRegistry::UnregisterFloraReceiver(UGravityFloraMovementSolver* Solver)
{
	FloraReceivers.RemoveAll([Solver](const TWeakObjectPtr<UGravityFloraMovementSolver>& W)
	{
		return !W.IsValid() || W.Get() == Solver;
	});
}

void UGravityFieldRegistry::Tick(float DeltaTime)
{
	// Distribute accumulated fauna positions to all flora receivers, then clear.
	for (auto& Weak : FloraReceivers)
	{
		if (UGravityFloraMovementSolver* Solver = Weak.Get())
		{
			Solver->SetNearbyEntityPositions(AccumulatedPositions);
		}
	}
	AccumulatedPositions.Reset();

	// Prune dead weak pointers
	FloraReceivers.RemoveAll([](const TWeakObjectPtr<UGravityFloraMovementSolver>& W){ return !W.IsValid(); });
}
