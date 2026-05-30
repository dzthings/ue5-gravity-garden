#include "GravityWormMovementSolver.h"
#include "GravityStateChannels.h"

void UGravityWormMovementSolver::Solve(TArray<FGravityNode>& Nodes, const TArray<FGravityLink>& Links,
                                        float DeltaTime, UGravityStateChannels* Channels)
{
	if (Nodes.Num() < 2) return;

	// Advance autonomous locomotion target
	if (bAutonomousLocomotion && !bManualTarget)
	{
		LocomotionTime += DeltaTime;
		const float Angle = LocomotionTime * LocomotionFrequency * UE_TWO_PI;
		AttentionTarget = SpawnOrigin + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * LocomotionRadius;
	}

	// Sub-step for stability
	const float SubDt = DeltaTime / FMath::Max(SubSteps, 1);
	for (int32 s = 0; s < SubSteps; ++s)
	{
		StepSimulation(Nodes, SubDt);
	}

	// Update link state for rendering / channels
	// (Links array is const — write derived state onto nodes instead)
	WriteChannels(Nodes, Links, Channels);
}

void UGravityWormMovementSolver::StepSimulation(TArray<FGravityNode>& Nodes, float Dt)
{
	const int32 N = Nodes.Num();
	TArray<FVector> Forces;
	Forces.SetNumZeroed(N);

	// Gravity on all nodes
	for (int32 i = 0; i < N; ++i)
	{
		Forces[i].Z += Gravity;
	}

	// Attention spring: pull lead node toward AttentionTarget
	{
		FGravityNode& Lead = Nodes[0];
		FVector Delta      = AttentionTarget - Lead.Position;
		FVector AttnForce  = Delta * AttentionStrength - Lead.Velocity * AttentionDamping;
		Forces[0] += AttnForce;
	}

	// Spring-damper between consecutive nodes
	for (int32 i = 0; i < N - 1; ++i)
	{
		FGravityNode& A = Nodes[i];
		FGravityNode& B = Nodes[i + 1];

		FVector Delta    = B.Position - A.Position;
		float   Len      = Delta.Size();
		FVector Dir      = (Len > UE_KINDA_SMALL_NUMBER) ? Delta / Len : FVector::DownVector;
		float   Stretch  = Len - RestSpacing;
		float   DampDot  = FVector::DotProduct(B.Velocity - A.Velocity, Dir);
		FVector Spring   = Dir * (Stiffness * Stretch + Damping * DampDot);

		Forces[i]     += Spring;
		Forces[i + 1] -= Spring;

		// Write per-node tension for state channels / debug tint
		float AbsStretch      = FMath::Abs(Stretch);
		A.Tension              = FMath::Max(A.Tension, AbsStretch / FMath::Max(RestSpacing, 1.f));
		B.Tension              = FMath::Max(B.Tension, AbsStretch / FMath::Max(RestSpacing, 1.f));
	}

	// Integrate
	for (int32 i = 0; i < N; ++i)
	{
		FGravityNode& Node = Nodes[i];
		Node.Tension       = 0.f; // reset; written per sub-step above; channels get final value
		Node.Velocity     += Forces[i] * Dt;
		Node.Position     += Node.Velocity * Dt;
	}
}

void UGravityWormMovementSolver::WriteChannels(const TArray<FGravityNode>& Nodes,
                                                const TArray<FGravityLink>& Links,
                                                UGravityStateChannels* Channels)
{
	if (!Channels || Nodes.Num() == 0) return;

	float TotalSpeed     = 0.f;
	float MaxTension     = 0.f;
	float TotalInstability = 0.f;

	for (int32 i = 0; i < Nodes.Num() - 1; ++i)
	{
		const FGravityNode& A = Nodes[i];
		const FGravityNode& B = Nodes[i + 1];

		float Len     = FVector::Dist(A.Position, B.Position);
		float Stretch = FMath::Abs(Len - RestSpacing) / FMath::Max(RestSpacing, 1.f);
		MaxTension    = FMath::Max(MaxTension, Stretch);

		TotalSpeed       += A.Velocity.Size();
		TotalInstability += FMath::Abs(A.Velocity.Z); // vertical jitter = instability proxy
	}
	TotalSpeed += Nodes.Last().Velocity.Size();

	const float InvN       = 1.f / Nodes.Num();
	Channels->Tension      = MaxTension;
	Channels->Speed        = TotalSpeed * InvN;
	Channels->Instability  = TotalInstability * InvN / FMath::Max(RestSpacing, 1.f);
}
