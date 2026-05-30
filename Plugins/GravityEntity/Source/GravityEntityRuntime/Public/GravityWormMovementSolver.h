#pragma once

#include "CoreMinimal.h"
#include "GravityMovementSolver.h"
#include "GravityWormMovementSolver.generated.h"

// Custom spring-damper movement solver for the Worm family.
// Lead node is attracted to an attention target via a position spring.
// Trailing nodes spring toward the node ahead, creating natural lag,
// compression, overshoot, and settle.
//
// Locomotion: attention target walks forward along a ground plane with a
// sinusoidal turn rate, producing S-curve crawling paths. A soft boundary
// radius curves the worm back toward its spawn area.
// Ground constraint clamps all nodes above GroundZ and applies friction.
// Entities that float (Orbital, etc.) use their own solver — no shared flag.
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravityWormMovementSolver : public UGravityMovementSolver
{
	GENERATED_BODY()

public:
	// --- Physics ---
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
	float Stiffness = 400.f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "500.0"))
	float Damping = 20.f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "10.0", ClampMax = "500.0"))
	float RestSpacing = 80.f;

	UPROPERTY(EditAnywhere, Category = "Physics")
	float Gravity = -980.f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "1", ClampMax = "16"))
	int32 SubSteps = 4;

	// Push-apart force between non-adjacent nodes — prevents self-intersection on tight bends.
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float NodeRepulsionStrength = 300.f;

	// Distance at which repulsion activates (cm). Set to ~2x your capsule radius.
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float NodeRepulsionRadius = 38.f;

	// --- Attention spring on lead node ---
	UPROPERTY(EditAnywhere, Category = "Attention", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float AttentionStrength = 600.f;

	UPROPERTY(EditAnywhere, Category = "Attention", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float AttentionDamping = 40.f;

	// --- Ground locomotion ---
	// Clamp nodes above GroundZ (set automatically from spawn height).
	UPROPERTY(EditAnywhere, Category = "Locomotion")
	bool bGroundConstrained = true;

	// XY velocity multiplier applied each frame a node is on the ground (0=no friction, 1=full stop).
	UPROPERTY(EditAnywhere, Category = "Locomotion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GroundFriction = 0.15f;

	// How fast the attention target walks forward (cm/s).
	UPROPERTY(EditAnywhere, Category = "Locomotion", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float ForwardSpeed = 120.f;

	// Peak turn rate (radians/s) — sinusoidal, creates S-curve paths.
	UPROPERTY(EditAnywhere, Category = "Locomotion", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float TurnAmplitude = 1.0f;

	// How often the turn direction reverses (Hz).
	UPROPERTY(EditAnywhere, Category = "Locomotion", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float TurnFrequency = 0.12f;

	// Wander radius: how far the worm roams before curving back toward spawn (cm).
	UPROPERTY(EditAnywhere, Category = "Locomotion", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float WanderRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Locomotion")
	bool bAutonomousLocomotion = true;

	void SetSpawnOrigin(FVector WorldOrigin)
	{
		SpawnOrigin     = WorldOrigin;
		GroundZ         = WorldOrigin.Z;
		AttentionTarget = WorldOrigin; // walk forward from here
	}

	void SetAttentionTarget(FVector WorldTarget) { AttentionTarget = WorldTarget; bManualTarget = true; }

	virtual void Solve(TArray<FGravityNode>& Nodes, const TArray<FGravityLink>& Links,
	                   float DeltaTime, UGravityStateChannels* Channels) override;

	virtual void Reset() override
	{
		TurnTime        = 0.f;
		HeadingAngle    = 0.f;
		AttentionTarget = FVector::ZeroVector;
		bManualTarget   = false;
	}

private:
	FVector SpawnOrigin     = FVector::ZeroVector;
	FVector AttentionTarget = FVector::ZeroVector;
	float   GroundZ         = 0.f;
	float   HeadingAngle    = 0.f; // radians in XY, 0 = +X
	float   TurnTime        = 0.f;
	bool    bManualTarget   = false;

	void UpdateLocomotionTarget(float DeltaTime);
	void StepSimulation(TArray<FGravityNode>& Nodes, float Dt);
	void WriteChannels(const TArray<FGravityNode>& Nodes, const TArray<FGravityLink>& Links,
	                   UGravityStateChannels* Channels);
};
