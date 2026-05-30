#include "GravitySpherePart.h"
#include "UDynamicMesh.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"

void UGravitySpherePart::GenerateMesh(UDynamicMesh* TargetMesh)
{
	if (!TargetMesh) return;
	FGeometryScriptPrimitiveOptions Opts;
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendSphere(
		TargetMesh, Opts, FTransform::Identity,
		Radius, Steps,
		EGeometryScriptPrimitiveOriginMode::Center, nullptr);
}

uint32 UGravitySpherePart::GetParamHash() const
{
	return HashCombine(GetTypeHash(Radius), GetTypeHash(Steps));
}
