#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GravityEntityProfile.generated.h"

class UGravityTopologySolver;
class UGravityMovementSolver;
class UGravityPartGenerator;
class UGravityBreathSignal;

// The variant asset — source of truth for one creature type.
// All solvers, generators, and breath config are Instanced UObjects edited inline in the Details panel.
UCLASS(BlueprintType)
class GRAVITYENTITYRUNTIME_API UGravityEntityProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- Topology ---
	UPROPERTY(EditAnywhere, Instanced, Category = "Topology")
	TObjectPtr<UGravityTopologySolver> TopologySolver;

	// --- Motion ---
	UPROPERTY(EditAnywhere, Instanced, Category = "Movement")
	TObjectPtr<UGravityMovementSolver> MovementSolver;

	// --- Parts (M4) ---
	// Part generators keyed by role — generate once, cache by param hash, instance via ISM/HISM.
	UPROPERTY(EditAnywhere, Instanced, Category = "Parts")
	TArray<TObjectPtr<UGravityPartGenerator>> PartGenerators;

	// --- Breath ---
	UPROPERTY(EditAnywhere, Instanced, Category = "Breath")
	TObjectPtr<UGravityBreathSignal> BreathSignal;

	// --- Physics params ---
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "2", ClampMax = "64"))
	int32 NodeCount = 8;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "10.0", ClampMax = "500.0"))
	float RestSpacing = 80.f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
	float Stiffness = 400.f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "500.0"))
	float Damping = 20.f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float NodeSize = 1.f;

	// --- Identity ---
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("GravityEntityProfile"), GetFName());
	}
};
