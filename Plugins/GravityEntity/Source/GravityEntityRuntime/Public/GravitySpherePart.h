#pragma once

#include "CoreMinimal.h"
#include "GravityPartGenerator.h"
#include "GravitySpherePart.generated.h"

// Generates a sphere mesh via Geometry Script.
// Used by Orbital cluster nodes; Core gets a larger sphere via a separate generator entry.
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravitySpherePart : public UGravityPartGenerator
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Shape", meta = (ClampMin = "2.0", ClampMax = "500.0"))
	float Radius = 15.f;

	UPROPERTY(EditAnywhere, Category = "Shape", meta = (ClampMin = "4", ClampMax = "32"))
	int32 Steps = 10;

	virtual void   GenerateMeshData(TArray<FVector>& OutVerts, TArray<int32>& OutTris,
	                                TArray<FVector>& OutNormals, TArray<FVector2D>& OutUVs) override;
	virtual uint32 GetParamHash() const override;
};
