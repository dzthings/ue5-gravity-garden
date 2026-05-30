#include "GravityOrbitalMovementSolver.h"
#include "GravityStateChannels.h"

void UGravityOrbitalMovementSolver::Initialize(const TArray<FGravityNode>& Nodes)
{
	const int32 N = Nodes.Num();
	OrbitAxes.SetNum(N);
	InitialOffsets.SetNum(N);
	PhaseOffsets.SetNum(N);

	FRandomStream Rng(OrbitSeed);
	const FVector CorePos = Nodes[0].Position;

	for (int32 i = 1; i < N; ++i)
	{
		FVector ToOrbital = Nodes[i].Position - CorePos;
		InitialOffsets[i] = ToOrbital.GetSafeNormal();

		// Random axis tilted per node — no two orbitals share the same orbit plane
		FVector RandomDir = FVector(Rng.FRandRange(-1.f, 1.f),
		                            Rng.FRandRange(-1.f, 1.f),
		                            Rng.FRandRange(-1.f, 1.f)).GetSafeNormal();
		// Axis is perpendicular to the initial offset so orbit starts correctly
		OrbitAxes[i] = FVector::CrossProduct(InitialOffsets[i], RandomDir).GetSafeNormal();
		if (OrbitAxes[i].IsNearlyZero()) OrbitAxes[i] = FVector::UpVector;

		PhaseOffsets[i] = Rng.FRandRange(0.f, UE_TWO_PI);
	}

	bInitialized = true;
}

void UGravityOrbitalMovementSolver::Solve(TArray<FGravityNode>& Nodes, const TArray<FGravityLink>& Links,
                                           float DeltaTime, UGravityStateChannels* Channels)
{
	if (Nodes.Num() < 2) return;

	if (!bInitialized) Initialize(Nodes);

	OrbitTime += DeltaTime;
	DriftTime += DeltaTime;

	const float SubDt = DeltaTime / FMath::Max(SubSteps, 1);
	for (int32 s = 0; s < SubSteps; ++s)
	{
		StepSimulation(Nodes, SubDt);
	}

	WriteChannels(Nodes, Channels);
}

void UGravityOrbitalMovementSolver::StepSimulation(TArray<FGravityNode>& Nodes, float Dt)
{
	// --- Core drift: Lissajous figure with incommensurable frequencies ---
	const float F  = DriftFrequency;
	const float T  = DriftTime;
	FVector LissajousTarget = SpawnOrigin + FVector(
		DriftAmplitude       * FMath::Sin(F * T),
		DriftAmplitude       * FMath::Sin(F * 2.f * T + 0.4f),
		DriftAmplitude * 0.4f * FMath::Sin(F * 3.f * T + 1.1f)
	);

	FGravityNode& Core  = Nodes[0];
	FVector CoreForce   = (LissajousTarget - Core.Position) * CoreStiffness
	                    - Core.Velocity * CoreDamping;
	Core.Velocity      += CoreForce * Dt;
	Core.Position      += Core.Velocity * Dt;

	// --- Orbitals: spring toward rotating target around current Core position ---
	const FVector CorePos = Core.Position;

	for (int32 i = 1; i < Nodes.Num(); ++i)
	{
		FGravityNode& Orbital = Nodes[i];

		// Rotate initial offset around this node's personal axis
		float  Angle      = OrbitTime * OrbitalSpeed + PhaseOffsets[i];
		FQuat  Rot        = FQuat(OrbitAxes[i], Angle);
		FVector TargetDir = Rot.RotateVector(InitialOffsets[i]);
		FVector TargetPos = CorePos + TargetDir * OrbitRadius;

		FVector Force = (TargetPos - Orbital.Position) * OrbitalStiffness
		              - Orbital.Velocity * OrbitalDamping;
		Orbital.Velocity += Force * Dt;
		Orbital.Position += Orbital.Velocity * Dt;
	}
}

void UGravityOrbitalMovementSolver::WriteChannels(const TArray<FGravityNode>& Nodes,
                                                   UGravityStateChannels* Channels)
{
	if (!Channels || Nodes.Num() < 2) return;

	const FVector CorePos = Nodes[0].Position;
	float MaxStretch  = 0.f;
	float TotalSpeed  = Nodes[0].Velocity.Size();

	for (int32 i = 1; i < Nodes.Num(); ++i)
	{
		float Dist    = FVector::Dist(Nodes[i].Position, CorePos);
		float Stretch = FMath::Abs(Dist - OrbitRadius) / FMath::Max(OrbitRadius, 1.f);
		MaxStretch    = FMath::Max(MaxStretch, Stretch);
		TotalSpeed   += Nodes[i].Velocity.Size();
	}

	Channels->Tension  = MaxStretch;
	Channels->Speed    = TotalSpeed / Nodes.Num();
	Channels->Charge   = FVector::Dist(Nodes[0].Position, SpawnOrigin) / FMath::Max(DriftAmplitude, 1.f);
}
