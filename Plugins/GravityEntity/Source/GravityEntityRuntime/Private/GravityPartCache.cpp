#include "GravityPartCache.h"

UStaticMesh* UGravityPartCache::FindCachedMesh(uint32 ParamHash) const
{
	if (const TObjectPtr<UStaticMesh>* Found = Cache.Find(ParamHash))
	{
		return Found->Get();
	}
	return nullptr;
}

void UGravityPartCache::StoreMesh(uint32 ParamHash, UStaticMesh* Mesh)
{
	Cache.Add(ParamHash, Mesh);
}

void UGravityPartCache::Clear()
{
	Cache.Reset();
}
