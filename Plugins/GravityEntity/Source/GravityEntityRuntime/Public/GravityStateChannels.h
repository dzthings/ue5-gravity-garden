#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GravityStateChannels.generated.h"

// Named runtime signals exposed by the solver — the shared interface that
// geometry offsets, materials, breath, and effects all consume.
// Consumers read channels; they never recompute another subsystem's state.
UCLASS(BlueprintType)
class GRAVITYENTITYRUNTIME_API UGravityStateChannels : public UObject
{
	GENERATED_BODY()

public:
	// Entity-level aggregated signals, written by solvers each tick.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Channels")
	float Tension = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Channels")
	float Charge = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Channels")
	float Speed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Channels")
	float ResonancePhase = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Channels")
	float Instability = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Channels")
	float Attention = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Channels")
	float BreathPhase = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Channels")
	float BreathAmplitude = 0.f;

	// Generic named access for extensibility.
	UFUNCTION(BlueprintCallable, Category = "GravityEntity|Channels")
	void   SetChannel(FName ChannelName, float Value);

	UFUNCTION(BlueprintCallable, Category = "GravityEntity|Channels")
	float  GetChannel(FName ChannelName) const;

	void Reset();

private:
	// Overflow map for channels not listed above.
	TMap<FName, float> ExtraChannels;
};
