#include "GravityBreathAuthored.h"
#include "Math/UnrealMathUtility.h"

void UGravityBreathAuthored::RebuildIfNeeded(int32 NodeCount)
{
	if (CachedNodeCount == NodeCount) return;

	CachedNodeCount = NodeCount;
	NodeFrequencies.SetNum(NodeCount);

	FRandomStream Rng(JitterSeed);
	for (int32 i = 0; i < NodeCount; ++i)
	{
		float Jitter = Rng.FRandRange(-FrequencyJitter, FrequencyJitter);
		NodeFrequencies[i] = BaseFrequency * (1.f + Jitter);
	}
}

void UGravityBreathAuthored::Update(int32 NodeCount, float DeltaTime)
{
	RebuildIfNeeded(NodeCount);
	ElapsedTime += DeltaTime;
}

float UGravityBreathAuthored::GetPhase(int32 NodeIndex) const
{
	if (!NodeFrequencies.IsValidIndex(NodeIndex)) return 0.f;

	const float TravelPhase = NodeIndex * PhaseOffsetPerNode * UE_TWO_PI;
	return ElapsedTime * NodeFrequencies[NodeIndex] * UE_TWO_PI + TravelPhase;
}

float UGravityBreathAuthored::GetAmplitude(int32 NodeIndex) const
{
	return BaseAmplitude;
}
