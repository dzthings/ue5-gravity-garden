#include "GravityWormMovementSolver.h"
#include "GravityStateChannels.h"

void UGravityWormMovementSolver::Solve(TArray<FGravityNode>& Nodes, const TArray<FGravityLink>& Links,
                                        float DeltaTime, UGravityStateChannels* Channels)
{
	if (Nodes.Num() < 2) return;

	if (bAutonomousLocomotion && !bManualTarget)
	{
		UpdateLocomotionTarget(DeltaTime);
	}

	const float SubDt = DeltaTime / FMath::Max(SubSteps, 1);
	for (int32 s = 0; s < SubSteps; ++s)
	{
		StepSimulation(Nodes, SubDt);
	}

	WriteChannels(Nodes, Links, Channels);
}

void UGravityWormMovementSolver::UpdateLocomotionTarget(float DeltaTime)
{
	TurnTime += DeltaTime;

	// Sinusoidal turn rate → natural S-curves; never locks into a circle
	HeadingAngle += TurnAmplitude * FMath::Sin(TurnTime * TurnFrequency * UE_TWO_PI) * DeltaTime;

	// Soft boundary: bias heading back toward spawn when wandering too far
	FVector ToSpawn = SpawnOrigin - AttentionTarget;
	ToSpawn.Z = 0.f;
	float Dist = ToSpawn.Size2D();
	if (Dist > WanderRadius)
	{
		float ReturnAngle = FMath::Atan2(ToSpawn.Y, ToSpawn.X);
		float Delta       = FMath::Fmod(ReturnAngle - HeadingAngle + UE_PI, UE_TWO_PI) - UE_PI;
		HeadingAngle     += Delta * FMath::Clamp((Dist - WanderRadius) / WanderRadius, 0.f, 1.f) * DeltaTime * 2.f;
	}

	// Advance target forward along heading, stay at ground level
	FVector Forward  = FVector(FMath::Cos(HeadingAngle), FMath::Sin(HeadingAngle), 0.f);
	AttentionTarget += Forward * ForwardSpeed * DeltaTime;
	AttentionTarget.Z = GroundZ;
}

void UGravityWormMovementSolver::StepSimulation(TArray<FGravityNode>& Nodes, float Dt)
{
	const int32 N = Nodes.Num();
	TArray<FVector> Forces;
	Forces.SetNumZeroed(N);

	// Gravity
	for (int32 i = 0; i < N; ++i)
	{
		Forces[i].Z += Gravity;
	}

	// Attention spring on lead node
	{
		FGravityNode& Lead = Nodes[0];
		Forces[0] += (AttentionTarget - Lead.Position) * AttentionStrength
		           - Lead.Velocity * AttentionDamping;
	}

	// Spring-damper between consecutive nodes
	for (int32 i = 0; i < N - 1; ++i)
	{
		FGravityNode& A = Nodes[i];
		FGravityNode& B = Nodes[i + 1];

		FVector Delta   = B.Position - A.Position;
		float   Len     = Delta.Size();
		FVector Dir     = (Len > UE_KINDA_SMALL_NUMBER) ? Delta / Len : FVector::DownVector;
		float   Stretch = Len - RestSpacing;
		float   DampDot = FVector::DotProduct(B.Velocity - A.Velocity, Dir);
		FVector Spring  = Dir * (Stiffness * Stretch + Damping * DampDot);

		Forces[i]     += Spring;
		Forces[i + 1] -= Spring;
	}

	// Node-node repulsion — prevents self-intersection on tight bends.
	// Skip adjacent pairs (i, i+1) — already handled by the spring above.
	if (NodeRepulsionStrength > 0.f)
	{
		for (int32 i = 0; i < N; ++i)
		{
			for (int32 j = i + 2; j < N; ++j)
			{
				FVector Delta = Nodes[j].Position - Nodes[i].Position;
				float   Dist  = Delta.Size();
				if (Dist < NodeRepulsionRadius && Dist > UE_KINDA_SMALL_NUMBER)
				{
					FVector Dir   = Delta / Dist;
					float   Force = NodeRepulsionStrength * (1.f - Dist / NodeRepulsionRadius);
					Forces[i] -= Dir * Force;
					Forces[j] += Dir * Force;
				}
			}
		}
	}

	// Integrate
	for (int32 i = 0; i < N; ++i)
	{
		FGravityNode& Node = Nodes[i];
		Node.Velocity += Forces[i] * Dt;
		Node.Position += Node.Velocity * Dt;

		// Ground constraint — each entity type owns its locomotion style in its solver
		if (bGroundConstrained && Node.Position.Z < GroundZ)
		{
			Node.Position.Z = GroundZ;
			if (Node.Velocity.Z < 0.f)
			{
				Node.Velocity.Z  = 0.f;
				Node.Velocity.X *= (1.f - GroundFriction);
				Node.Velocity.Y *= (1.f - GroundFriction);
			}
		}
	}
}

void UGravityWormMovementSolver::WriteChannels(const TArray<FGravityNode>& Nodes,
                                                const TArray<FGravityLink>& Links,
                                                UGravityStateChannels* Channels)
{
	if (!Channels || Nodes.Num() == 0) return;

	float TotalSpeed  = 0.f;
	float MaxTension  = 0.f;
	float TotalInstability = 0.f;

	for (int32 i = 0; i < Nodes.Num() - 1; ++i)
	{
		float Len     = FVector::Dist(Nodes[i].Position, Nodes[i + 1].Position);
		float Stretch = FMath::Abs(Len - RestSpacing) / FMath::Max(RestSpacing, 1.f);
		MaxTension    = FMath::Max(MaxTension, Stretch);
		TotalSpeed   += Nodes[i].Velocity.Size();
		TotalInstability += FMath::Abs(Nodes[i].Velocity.Z);
	}
	TotalSpeed += Nodes.Last().Velocity.Size();

	const float InvN      = 1.f / Nodes.Num();
	Channels->Tension     = MaxTension;
	Channels->Speed       = TotalSpeed * InvN;
	Channels->Instability = TotalInstability * InvN / FMath::Max(RestSpacing, 1.f);
}
