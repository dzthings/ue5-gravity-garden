#include "GravitySpineSegmentPart.h"
#include "UDynamicMesh.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"

void UGravitySpineSegmentPart::GenerateMesh(UDynamicMesh* TargetMesh)
{
	if (!TargetMesh) return;
	FGeometryScriptPrimitiveOptions Opts;
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCapsule(
		TargetMesh, Opts, FTransform::Identity,
		Radius, LineLength,
		RadialSegments, HemisphereSteps,
		EGeometryScriptPrimitiveOriginMode::Center, nullptr);
}

uint32 UGravitySpineSegmentPart::GetParamHash() const
{
	return HashCombine(HashCombine(GetTypeHash(Radius), GetTypeHash(LineLength)), GetTypeHash(RadialSegments));
}
