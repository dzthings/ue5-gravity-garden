#include "GravityFloraTopologySolver.h"
#include "GravityEntityProfile.h"

void UGravityFloraTopologySolver::BuildTopology(TArray<FGravityNode>& OutNodes, TArray<FGravityLink>& OutLinks,
                                                 const UGravityEntityProfile* Profile)
{
	OutNodes.Reset();
	OutLinks.Reset();

	if (!Profile || Profile->NodeCount < 2) return;

	const int32 N       = Profile->NodeCount;
	const float Spacing = Profile->RestSpacing;

	FRandomStream Rng(LeanSeed);
	const FVector LeanDir(Rng.FRandRange(-1.f, 1.f), Rng.FRandRange(-1.f, 1.f), 0.f);

	for (int32 i = 0; i < N; ++i)
	{
		FGravityNode Node;
		Node.NodeID = i;

		// Grow upward from origin; lean increases toward the tip
		float LeanFrac = (float)i / FMath::Max(N - 1, 1);
		FVector Lean   = LeanDir.GetSafeNormal() * InitialLeanAmount * LeanFrac * LeanFrac;
		Node.Position  = FVector(Lean.X, Lean.Y, i * Spacing);

		Node.Role      = (i == 0) ? EGravityNodeRole::Anchor : EGravityNodeRole::Spine;
		Node.bAnchored = (i == 0);

		OutNodes.Add(Node);
	}

	for (int32 i = 0; i < N - 1; ++i)
	{
		FGravityLink Link;
		Link.StartNodeID = i;
		Link.EndNodeID   = i + 1;
		OutLinks.Add(Link);
	}
}
