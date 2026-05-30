#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GravityMaterialProfile.generated.h"

// Maps state-channel values to material parameters.
// The surface is another readout of the field — not a separately animated skin.
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravityMaterialProfile : public UObject
{
	GENERATED_BODY()

public:
	// Minimum glow when breath is at its lowest point.
	UPROPERTY(EditAnywhere, Category = "Glow", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseGlow = 0.1f;

	// Tension multiplier: high stretch between nodes boosts glow (field under stress = brighter).
	UPROPERTY(EditAnywhere, Category = "Glow", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float TensionGlowScale = 1.2f;

	// Compute CPD[0] for a node: combines breath wave with tension boost.
	float ComputeNodeGlow(float BreathCPD, float Tension) const
	{
		return FMath::Clamp(BaseGlow + BreathCPD + Tension * TensionGlowScale, 0.f, 1.f);
	}

	// Compute CPD[0] for a link: tension-driven, no breath offset on links.
	float ComputeLinkGlow(float Tension) const
	{
		return FMath::Clamp(BaseGlow + Tension * TensionGlowScale, 0.f, 1.f);
	}
};
