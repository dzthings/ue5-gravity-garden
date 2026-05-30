#include "GravityWormTopologySolver.h"
#include "GravityEntityProfile.h"

void UGravityWormTopologySolver::BuildTopology(TArray<FGravityNode>& OutNodes, TArray<FGravityLink>& OutLinks,
                                                const UGravityEntityProfile* Profile)
{
	OutNodes.Reset();
	OutLinks.Reset();

	if (!Profile || Profile->NodeCount < 2) return;

	const int32 N           = Profile->NodeCount;
	const float Spacing     = Profile->RestSpacing;
	const int32 CoreIndex   = N / 2;

	for (int32 i = 0; i < N; ++i)
	{
		FGravityNode Node;
		Node.NodeID   = i;
		Node.Position = FVector(-i * Spacing, 0.f, 0.f); // lie along -X at ground level; offset by actor in component
		Node.TargetPosition = Node.Position;

		if      (i == 0)         Node.Role = EGravityNodeRole::Lead;
		else if (i == CoreIndex) Node.Role = EGravityNodeRole::Core;
		else                     Node.Role = EGravityNodeRole::Spine;

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
