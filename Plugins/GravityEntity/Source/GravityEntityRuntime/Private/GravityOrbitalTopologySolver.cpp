#include "GravityOrbitalTopologySolver.h"
#include "GravityEntityProfile.h"

void UGravityOrbitalTopologySolver::BuildTopology(TArray<FGravityNode>& OutNodes, TArray<FGravityLink>& OutLinks,
                                                   const UGravityEntityProfile* Profile)
{
	OutNodes.Reset();
	OutLinks.Reset();

	if (!Profile || Profile->NodeCount < 2) return;

	const int32 TotalNodes  = Profile->NodeCount;
	const int32 OrbitalCount = TotalNodes - 1; // everything except Core

	// Core node at origin (offset to actor location by component)
	{
		FGravityNode Core;
		Core.NodeID  = 0;
		Core.Role    = EGravityNodeRole::Core;
		Core.Position = FVector::ZeroVector;
		OutNodes.Add(Core);
	}

	// Fibonacci sphere distribution — maximally even spacing on sphere surface
	const float GoldenAngle = UE_PI * (3.f - FMath::Sqrt(5.f));

	for (int32 i = 0; i < OrbitalCount; ++i)
	{
		float Y      = 1.f - (i / FMath::Max((float)(OrbitalCount - 1), 1.f)) * 2.f;
		float Radius = FMath::Sqrt(FMath::Max(1.f - Y * Y, 0.f));
		float Theta  = GoldenAngle * i;

		FGravityNode Orbital;
		Orbital.NodeID   = i + 1;
		Orbital.Role     = EGravityNodeRole::Orbital;
		Orbital.Position = FVector(FMath::Cos(Theta) * Radius, FMath::Sin(Theta) * Radius, Y) * OrbitRadius;
		OutNodes.Add(Orbital);

		// Spoke link: Core -> this Orbital
		FGravityLink Link;
		Link.StartNodeID = 0;
		Link.EndNodeID   = i + 1;
		OutLinks.Add(Link);
	}
}
