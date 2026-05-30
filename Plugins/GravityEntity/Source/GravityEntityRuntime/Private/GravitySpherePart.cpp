#include "GravitySpherePart.h"

void UGravitySpherePart::GenerateMeshData(TArray<FVector>& OutVerts, TArray<int32>& OutTris,
                                           TArray<FVector>& OutNormals, TArray<FVector2D>& OutUVs)
{
	const int32 Lat = FMath::Max(Steps, 4);
	const int32 Lon = FMath::Max(Steps, 4);

	OutVerts.Reset();
	OutTris.Reset();
	OutNormals.Reset();
	OutUVs.Reset();

	// Vertices: (Lat+1) x (Lon+1) grid
	for (int32 i = 0; i <= Lat; ++i)
	{
		float Theta = UE_PI * i / Lat;         // 0 (north) → PI (south)
		float SinT  = FMath::Sin(Theta);
		float CosT  = FMath::Cos(Theta);

		for (int32 j = 0; j <= Lon; ++j)
		{
			float Phi  = UE_TWO_PI * j / Lon;  // 0 → 2PI around equator
			FVector N(SinT * FMath::Cos(Phi), SinT * FMath::Sin(Phi), CosT);
			OutVerts.Add(N * Radius);
			OutNormals.Add(N);
			OutUVs.Add(FVector2D((float)j / Lon, (float)i / Lat));
		}
	}

	// Triangles: two per quad cell
	for (int32 i = 0; i < Lat; ++i)
	{
		for (int32 j = 0; j < Lon; ++j)
		{
			int32 A = i * (Lon + 1) + j;
			int32 B = A + Lon + 1;
			OutTris.Add(A);   OutTris.Add(A + 1); OutTris.Add(B);
			OutTris.Add(B);   OutTris.Add(A + 1); OutTris.Add(B + 1);
		}
	}
}

uint32 UGravitySpherePart::GetParamHash() const
{
	return HashCombine(GetTypeHash(Radius), GetTypeHash(Steps));
}
