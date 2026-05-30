#include "GravitySpineSegmentPart.h"

// Capsule: top hemisphere + bottom hemisphere, centred at origin, Z-up.
void UGravitySpineSegmentPart::GenerateMeshData(TArray<FVector>& OutVerts, TArray<int32>& OutTris,
                                                 TArray<FVector>& OutNormals, TArray<FVector2D>& OutUVs)
{
	OutVerts.Reset(); OutTris.Reset(); OutNormals.Reset(); OutUVs.Reset();

	const int32 RS   = FMath::Max(RadialSegments, 4);
	const int32 HS   = FMath::Max(HemisphereSteps, 1);
	const float Half = LineLength * 0.5f;

	// Build one horizontal ring of vertices
	auto AddRing = [&](float Z, float RingRadius, float NZ)
	{
		for (int32 j = 0; j <= RS; ++j)
		{
			float Phi = UE_TWO_PI * j / RS;
			float Cx  = FMath::Cos(Phi);
			float Cy  = FMath::Sin(Phi);
			OutVerts.Add(FVector(Cx * RingRadius, Cy * RingRadius, Z));
			float NXY = FMath::Sqrt(FMath::Max(1.f - NZ * NZ, 0.f));
			OutNormals.Add(FVector(Cx * NXY, Cy * NXY, NZ).GetSafeNormal());
			OutUVs.Add(FVector2D((float)j / RS, 0.f));
		}
	};

	// Top hemisphere: pole (lat=0) → equator (lat=HS)
	for (int32 i = 0; i <= HS; ++i)
	{
		float Theta = UE_HALF_PI * i / HS;
		AddRing(Half + Radius * FMath::Cos(Theta), Radius * FMath::Sin(Theta), FMath::Cos(Theta));
	}

	// Bottom hemisphere: equator (lat=0) → pole (lat=HS)
	for (int32 i = 1; i <= HS; ++i)
	{
		float Theta = UE_HALF_PI + UE_HALF_PI * i / HS;
		AddRing(-Half + Radius * FMath::Cos(Theta), Radius * FMath::Sin(Theta), FMath::Cos(Theta));
	}

	// Side quads connecting consecutive rings — clockwise winding (UE front face)
	const int32 TotalRings = (HS + 1) + HS;
	for (int32 ring = 0; ring < TotalRings - 1; ++ring)
	{
		for (int32 j = 0; j < RS; ++j)
		{
			int32 A = ring * (RS + 1) + j;
			int32 B = A + RS + 1;
			OutTris.Add(A);     OutTris.Add(A + 1); OutTris.Add(B);
			OutTris.Add(B);     OutTris.Add(A + 1); OutTris.Add(B + 1);
		}
	}

	// Top pole cap
	{
		int32 Pole = OutVerts.Num();
		OutVerts.Add(FVector(0.f, 0.f, Half + Radius));
		OutNormals.Add(FVector::UpVector);
		OutUVs.Add(FVector2D(0.5f, 0.f));
		for (int32 j = 0; j < RS; ++j)
		{
			OutTris.Add(Pole); OutTris.Add(j); OutTris.Add(j + 1);
		}
	}

	// Bottom pole cap
	{
		const int32 LastRing = (TotalRings - 1) * (RS + 1);
		int32 Pole = OutVerts.Num();
		OutVerts.Add(FVector(0.f, 0.f, -Half - Radius));
		OutNormals.Add(-FVector::UpVector);
		OutUVs.Add(FVector2D(0.5f, 1.f));
		for (int32 j = 0; j < RS; ++j)
		{
			OutTris.Add(Pole); OutTris.Add(LastRing + j + 1); OutTris.Add(LastRing + j);
		}
	}
}

uint32 UGravitySpineSegmentPart::GetParamHash() const
{
	return HashCombine(HashCombine(GetTypeHash(Radius), GetTypeHash(LineLength)), GetTypeHash(RadialSegments));
}
