#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GravityEntityTypes.h"
#include "GravityPartGenerator.generated.h"

// Generates procedural geometry for a node role via Geometry Script.
// Generate once at construction, cache by param hash, instance via ISM/HISM — never per frame.
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravityPartGenerator : public UObject
{
	GENERATED_BODY()

public:
	// Roles this generator handles. Profile maps roles to generators.
	UPROPERTY(EditAnywhere, Category = "Part")
	TArray<EGravityNodeRole> Roles;

	// Returns a stable hash of the parameters that affect mesh shape.
	virtual uint32 GetParamHash() const { return 0; }

	// Populate mesh data arrays for a ProceduralMeshComponent section.
	// Called once at entity init; the component applies transforms per-tick.
	virtual void GenerateMeshData(TArray<FVector>& OutVerts, TArray<int32>& OutTris,
	                               TArray<FVector>& OutNormals, TArray<FVector2D>& OutUVs) {}
};
