#pragma once

#include "CoreMinimal.h"
#include "GravityMovementSolver.h"
#include "GravityOrbitalMovementSolver.generated.h"

// Movement solver for the Orbital Cluster family.
//
// Core node drifts on a 3D Lissajous figure — slow, weightless, volumetric.
// Each Orbital node springs toward a rotating target position around the Core,
// with a per-node tilted orbit axis so no two orbitals share the same plane.
// The result — a breathing cluster of nodes drifting and orbiting — is
// intentionally distinct from the Worm's linear serpentine trail.
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravityOrbitalMovementSolver : public UGravityMovementSolver
{
	GENERATED_BODY()

public:
	// --- Orbital spring ---
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float OrbitalStiffness = 300.f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float OrbitalDamping = 18.f;

	// How fast orbitals rotate around the core (radians/second).
	UPROPERTY(EditAnywhere, Category = "Orbital", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float OrbitalSpeed = 0.8f;

	// Orbit radius — should match topology solver's OrbitRadius.
	UPROPERTY(EditAnywhere, Category = "Orbital", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
	float OrbitRadius = 150.f;

	// --- Core drift ---
	// Core drifts on a Lissajous figure: distinct spatial frequencies on each axis
	// so it never repeats exactly, giving an alive, unpredictable quality.
	UPROPERTY(EditAnywhere, Category = "Drift", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float DriftAmplitude = 200.f;

	UPROPERTY(EditAnywhere, Category = "Drift", meta = (ClampMin = "0.01", ClampMax = "2.0"))
	float DriftFrequency = 0.18f;

	// Spring pulling core toward its Lissajous target.
	UPROPERTY(EditAnywhere, Category = "Drift", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float CoreStiffness = 120.f;

	UPROPERTY(EditAnywhere, Category = "Drift", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float CoreDamping = 14.f;

	// Seed for per-node orbit axis jitter.
	UPROPERTY(EditAnywhere, Category = "Orbital")
	int32 OrbitSeed = 7;

	// Sub-steps per tick.
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "1", ClampMax = "16"))
	int32 SubSteps = 4;

	void SetSpawnOrigin(FVector WorldOrigin) { SpawnOrigin = WorldOrigin; }

	virtual void Solve(TArray<FGravityNode>& Nodes, const TArray<FGravityLink>& Links,
	                   float DeltaTime, UGravityStateChannels* Channels) override;

	virtual void Reset() override
	{
		OrbitTime    = 0.f;
		DriftTime    = 0.f;
		bInitialized = false;
		OrbitAxes.Reset();
		InitialOffsets.Reset();
		PhaseOffsets.Reset();
	}

private:
	FVector SpawnOrigin = FVector::ZeroVector;
	float   OrbitTime   = 0.f;
	float   DriftTime   = 0.f;
	bool    bInitialized = false;

	// Per-orbital: unit axis to rotate around, initial offset direction, phase offset.
	TArray<FVector> OrbitAxes;
	TArray<FVector> InitialOffsets; // unit vectors from core to each orbital at spawn
	TArray<float>   PhaseOffsets;

	void Initialize(const TArray<FGravityNode>& Nodes);
	void StepSimulation(TArray<FGravityNode>& Nodes, float Dt);
	void WriteChannels(const TArray<FGravityNode>& Nodes, UGravityStateChannels* Channels);
};
