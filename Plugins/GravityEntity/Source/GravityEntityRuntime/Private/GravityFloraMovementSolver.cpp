#include "GravityFloraMovementSolver.h"
#include "GravityStateChannels.h"

void UGravityFloraMovementSolver::Solve(TArray<FGravityNode>& Nodes, const TArray<FGravityLink>& Links,
                                         float DeltaTime, UGravityStateChannels* Channels)
{
	if (Nodes.Num() < 2) return;

	// Capture rest positions on first tick
	if (RestPositions.Num() != Nodes.Num())
	{
		RestPositions.SetNum(Nodes.Num());
		for (int32 i = 0; i < Nodes.Num(); ++i)
		{
			RestPositions[i] = Nodes[i].Position;
		}
	}

	const float SubDt = DeltaTime / FMath::Max(SubSteps, 1);
	for (int32 s = 0; s < SubSteps; ++s)
	{
		StepSimulation(Nodes, SubDt);
	}

	// Write instability to channels — tips moving a lot = high instability
	if (Channels && Nodes.Num() > 0)
	{
		float TipSpeed = Nodes.Last().Velocity.Size();
		Channels->Instability = FMath::Clamp(TipSpeed / 100.f, 0.f, 1.f);
	}
}

void UGravityFloraMovementSolver::StepSimulation(TArray<FGravityNode>& Nodes, float Dt)
{
	const int32 N = Nodes.Num();

	// Node 0 is always pinned
	Nodes[0].Position = RestPositions[0];
	Nodes[0].Velocity = FVector::ZeroVector;

	for (int32 i = 1; i < N; ++i)
	{
		FGravityNode& Node = Nodes[i];

		// Spring toward rest position (structural stiffness)
		FVector ToRest    = RestPositions[i] - Node.Position;
		// Higher nodes are more flexible — reduce stiffness toward the tip
		float   NodeStiff = Stiffness * (1.f - 0.5f * (float)(i - 1) / FMath::Max(N - 2, 1));
		FVector Force     = ToRest * NodeStiff - Node.Velocity * Damping;

		// Attraction toward nearby entity nodes — tip is most sensitive
		float TipWeight = (float)i / FMath::Max(N - 1, 1); // 0 at root → 1 at tip
		for (const FVector& EntityPos : NearbyPositions)
		{
			FVector ToEntity = EntityPos - Node.Position;
			float   Dist     = ToEntity.Size();
			if (Dist < AttractionRadius && Dist > UE_KINDA_SMALL_NUMBER)
			{
				float Falloff  = FMath::Pow(1.f - Dist / AttractionRadius, FalloffExponent);
				Force         += (ToEntity / Dist) * AttractionStrength * Falloff * TipWeight;
			}
		}

		Node.Velocity += Force * Dt;
		Node.Position += Node.Velocity * Dt;
	}
}
