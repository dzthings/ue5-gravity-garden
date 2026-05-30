#include "GravitySpineSegmentPart.h"

// Builds a capsule: top hemisphere → cylinder → bottom hemisphere, centred at origin, Z-up.
void UGravitySpineSegmentPart::GenerateMeshData(TArray<FVector>& OutVerts, TArray<int32>& OutTris,
                                                 TArray<FVector>& OutNormals, TArray<FVector2D>& OutUVs)
{
	OutVerts.Reset();
	OutTris.Reset();
	OutNormals.Reset();
	OutUVs.Reset();

	const int32 RS   = FMath::Max(RadialSegments, 4);
	const int32 HS   = FMath::Max(HemisphereSteps, 1);
	const float Half = LineLength * 0.5f;

	auto AddRing = [&](float Z, float RingRadius, float NormalZ)
	{
		for (int32 j = 0; j <= RS; ++j)
		{
			float Phi = UE_TWO_PI * j / RS;
			float Cx  = FMath::Cos(Phi);
			float Cy  = FMath::Sin(Phi);
			OutVerts.Add(FVector(Cx * RingRadius, Cy * RingRadius, Z));
			FVector N = FVector(Cx * FMath::Sqrt(1.f - NormalZ * NormalZ),
			                    Cy * FMath::Sqrt(1.f - NormalZ * NormalZ), NormalZ).GetSafeNormal();
			OutNormals.Add(N);
			OutUVs.Add(FVector2D((float)j / RS, 0.f));
		}
	};

	// Top hemisphere rings (lat 0 = pole → lat HS = equator)
	for (int32 i = 0; i <= HS; ++i)
	{
		float Theta = UE_HALF_PI * i / HS;  // 0 (pole) → PI/2 (equator)
		float RingR = Radius * FMath::Sin(Theta);
		float RingZ = Half + Radius * FMath::Cos(Theta);
		float NZ    = FMath::Cos(Theta);
		AddRing(RingZ, RingR, NZ);
	}

	// Bottom hemisphere rings (lat 0 = equator → lat HS = pole)
	for (int32 i = 1; i <= HS; ++i)
	{
		float Theta = UE_HALF_PI + UE_HALF_PI * i / HS;  // PI/2 → PI
		float RingR = Radius * FMath::Sin(Theta);
		float RingZ = -Half + Radius * FMath::Cos(Theta);
		float NZ    = FMath::Cos(Theta);
		AddRing(RingZ, RingR, NZ);
	}

	// Triangles connecting consecutive rings
	const int32 TotalRings = (HS + 1) + HS; // top hemisphere rings + bottom hemisphere rings
	for (int32 ring = 0; ring < TotalRings - 1; ++ring)
	{
		for (int32 j = 0; j < RS; ++j)
		{
			int32 A = ring * (RS + 1) + j;
			int32 B = A + RS + 1;
			OutTris.Add(A);   OutTris.Add(B);   OutTris.Add(A + 1);
			OutTris.Add(B);   OutTris.Add(B + 1); OutTris.Add(A + 1);
		}
	}

	// Top pole cap
	{
		int32 PoleIdx = OutVerts.Num();
		OutVerts.Add(FVector(0.f, 0.f, Half + Radius));
		OutNormals.Add(FVector::UpVector);
		OutUVs.Add(FVector2D(0.5f, 0.f));
		for (int32 j = 0; j < RS; ++j)
		{
			OutTris.Add(PoleIdx);
			OutTris.Add(j + 1);
			OutTris.Add(j);
		}
	}

	// Bottom pole cap
	{
		const int32 LastRingStart = (TotalRings - 1) * (RS + 1);
		int32 PoleIdx = OutVerts.Num();
		OutVerts.Add(FVector(0.f, 0.f, -Half - Radius));
		OutNormals.Add(-FVector::UpVector);
		OutUVs.Add(FVector2D(0.5f, 1.f));
		for (int32 j = 0; j < RS; ++j)
		{
			OutTris.Add(PoleIdx);
			OutTris.Add(LastRingStart + j);
			OutTris.Add(LastRingStart + j + 1);
		}
	}
}

uint32 UGravitySpineSegmentPart::GetParamHash() const
{
	return HashCombine(HashCombine(GetTypeHash(Radius), GetTypeHash(LineLength)), GetTypeHash(RadialSegments));
}
