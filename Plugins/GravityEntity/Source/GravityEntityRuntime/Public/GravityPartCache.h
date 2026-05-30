#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GravityPartCache.generated.h"

class UGravityPartGenerator;

// Caches generated meshes by param hash so identical parts share one mesh + one ISM/HISM.
// M4: wired to Geometry Script generation and ISM management.
UCLASS()
class GRAVITYENTITYRUNTIME_API UGravityPartCache : public UObject
{
	GENERATED_BODY()

public:
	// Returns a cached mesh for this generator's param hash, or nullptr if not yet generated.
	UStaticMesh* FindCachedMesh(uint32 ParamHash) const;

	void StoreMesh(uint32 ParamHash, UStaticMesh* Mesh);

	void Clear();

private:
	TMap<uint32, TObjectPtr<UStaticMesh>> Cache;
};
