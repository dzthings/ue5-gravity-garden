#pragma once

#include "CoreMinimal.h"
#include "GravityMovementSolver.h"
#include "GravityWormMovementSolver.generated.h"

// Custom spring-damper movement solver for the Worm family.
// Lead node is attracted to an attention target via a position spring.
// Trailing nodes spring toward the node ahead of them, creating natural
// lag, compression, overshoot, and settle — the primary alive-read.
//
// Autonomous locomotion drives the attention target in a horizontal circle
// around the spawn origin so the spine can be judged in PIE without input.
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

	// Rest spacing between nodes (cm). Should match profile for consistency.
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "10.0", ClampMax = "500.0"))
	float RestSpacing = 80.f;

	// Gravitational acceleration (cm/s²). Negative = downward.
	UPROPERTY(EditAnywhere, Category = "Physics")
	float Gravity = -980.f;

	// --- Attention ---
	// Spring strength pulling the lead node toward AttentionTarget.
	UPROPERTY(EditAnywhere, Category = "Attention", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float AttentionStrength = 600.f;

	// Damping on the lead attention spring (prevents oscillation at target).
	UPROPERTY(EditAnywhere, Category = "Attention", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float AttentionDamping = 40.f;

	// --- Autonomous locomotion ---
	// Drives the attention target in a circle around SpawnOrigin so the spine
	// can be evaluated in PIE without player input.
	UPROPERTY(EditAnywhere, Category = "Locomotion")
	bool bAutonomousLocomotion = true;

	UPROPERTY(EditAnywhere, Category = "Locomotion", meta = (ClampMin = "0.01", ClampMax = "4.0"))
	float LocomotionFrequency = 0.12f; // circles per second

	UPROPERTY(EditAnywhere, Category = "Locomotion", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float LocomotionRadius = 300.f;

	// Sub-steps per tick for numerical stability.
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "1", ClampMax = "16"))
	int32 SubSteps = 4;

	// Called once by the component at BeginPlay so autonomous locomotion
	// has a stable world-space reference.
	void SetSpawnOrigin(FVector WorldOrigin) { SpawnOrigin = WorldOrigin; }

	// Set externally (Blueprint, player input, etc.) to override autonomous target.
	void SetAttentionTarget(FVector WorldTarget) { AttentionTarget = WorldTarget; bManualTarget = true; }

	virtual void Solve(TArray<FGravityNode>& Nodes, const TArray<FGravityLink>& Links,
	                   float DeltaTime, UGravityStateChannels* Channels) override;

	virtual void Reset() override
	{
		LocomotionTime  = 0.f;
		AttentionTarget = FVector::ZeroVector;
		bManualTarget   = false;
	}

private:
	FVector SpawnOrigin     = FVector::ZeroVector;
	FVector AttentionTarget = FVector::ZeroVector;
	bool    bManualTarget   = false;
	float   LocomotionTime  = 0.f;

	void StepSimulation(TArray<FGravityNode>& Nodes, float Dt);
	void WriteChannels(const TArray<FGravityNode>& Nodes, const TArray<FGravityLink>& Links,
	                   UGravityStateChannels* Channels);
};
