#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GravityEntityTypes.h"
#include "GravityPartGenerator.generated.h"

class UDynamicMesh;

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

	// Append this part's geometry into TargetMesh (Geometry Script).
	// Mesh is generated once at entity init; transform is applied per-tick by the component.
	virtual void GenerateMesh(UDynamicMesh* TargetMesh) {}
};
