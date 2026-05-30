#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GravityBreathSignal.generated.h"

// Interface: per-node breath phase + amplitude. Two concrete sources exist:
//   UGravityBreathAuthored  — sine + phase offset + jitter  (default, ships M1)
//   UGravityBreathEmergent  — coupled oscillators            (experiment, isolated)
// Geometry offsets and glow both read this signal; they never recompute it.
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravityBreathSignal : public UObject
{
	GENERATED_BODY()

public:
	virtual void  Update(int32 NodeCount, float DeltaTime) {}
	virtual float GetPhase(int32 NodeIndex)     const { return 0.f; }
	virtual float GetAmplitude(int32 NodeIndex) const { return 0.f; }
};
