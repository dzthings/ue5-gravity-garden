#pragma once

#include "CoreMinimal.h"
#include "GravityPartGenerator.h"
#include "GravitySpineSegmentPart.generated.h"

// Generates a capsule mesh (hemisphere + cylinder + hemisphere) via Geometry Script.
// Oriented along local Z. The component rotates it to align with each link direction.
// Adjacent segments overlap at the joints, creating a vertebrae-like spine appearance.
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class GRAVITYENTITYRUNTIME_API UGravitySpineSegmentPart : public UGravityPartGenerator
{
	GENERATED_BODY()

public:
	// Cross-section radius of the capsule (cm).
	UPROPERTY(EditAnywhere, Category = "Shape", meta = (ClampMin = "2.0", ClampMax = "200.0"))
	float Radius = 16.f;

	// Length of the straight cylinder section between the two hemispheres (cm).
	// Total segment height = LineLength + 2 * Radius.
	UPROPERTY(EditAnywhere, Category = "Shape", meta = (ClampMin = "0.0", ClampMax = "500.0"))
	float LineLength = 28.f;

	UPROPERTY(EditAnywhere, Category = "Shape", meta = (ClampMin = "4", ClampMax = "32"))
	int32 RadialSegments = 10;

	UPROPERTY(EditAnywhere, Category = "Shape", meta = (ClampMin = "1", ClampMax = "8"))
	int32 HemisphereSteps = 3;

	virtual void   GenerateMesh(UDynamicMesh* TargetMesh) override;
	virtual uint32 GetParamHash() const override;
};
