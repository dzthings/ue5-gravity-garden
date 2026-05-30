#pragma once

#include "CoreMinimal.h"
#include "GravityBreathSignal.h"
#include "GravityBreathAuthored.generated.h"

// Default breath producer: sine wave with per-segment phase offset + seeded per-node
// jitter in rate/phase, creating a traveling peristaltic wave rather than lockstep pulses.
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravityBreathAuthored : public UGravityBreathSignal
{
	GENERATED_BODY()

public:
	// Cycles per second for the base breath sine
	UPROPERTY(EditAnywhere, Category = "Breath", meta = (ClampMin = "0.05", ClampMax = "4.0"))
	float BaseFrequency = 0.4f;

	// Peak displacement amplitude (relative — applied by consumers)
	UPROPERTY(EditAnywhere, Category = "Breath", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseAmplitude = 0.15f;

	// Phase offset added per node index — creates the traveling wave along the chain
	UPROPERTY(EditAnywhere, Category = "Breath", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PhaseOffsetPerNode = 0.25f;

	// Per-node jitter in frequency (fraction of BaseFrequency) — breaks lockstep
	UPROPERTY(EditAnywhere, Category = "Breath", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float FrequencyJitter = 0.05f;

	// Seed for per-node jitter so variants are deterministic
	UPROPERTY(EditAnywhere, Category = "Breath")
	int32 JitterSeed = 42;

	virtual void  Update(int32 NodeCount, float DeltaTime) override;
	virtual float GetPhase(int32 NodeIndex)     const override;
	virtual float GetAmplitude(int32 NodeIndex) const override;

private:
	float ElapsedTime = 0.f;
	TArray<float> NodeFrequencies; // seeded per-node frequency offsets
	int32 CachedNodeCount = 0;

	void RebuildIfNeeded(int32 NodeCount);
};
