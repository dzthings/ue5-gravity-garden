#pragma once

#include "CoreMinimal.h"
#include "GravityEntityTypes.generated.h"

UENUM(BlueprintType)
enum class EGravityNodeRole : uint8
{
	Lead,
	Spine,
	Core,
	Orbital,
	Anchor,
	Shield,
	ScoutTip,
	LeftBranch,
	RightBranch,
	Damaged,
	Detached,
	Recallable,
};

// Per-node simulation state. Roles survive procedural variation and drive identity.
USTRUCT(BlueprintType)
struct GRAVITYENTITYRUNTIME_API FGravityNode
{
	GENERATED_BODY()

	UPROPERTY() int32           NodeID          = INDEX_NONE;
	UPROPERTY() EGravityNodeRole Role           = EGravityNodeRole::Spine;
	UPROPERTY() FVector         Position        = FVector::ZeroVector;
	UPROPERTY() FVector         TargetPosition  = FVector::ZeroVector;
	UPROPERTY() FVector         Velocity        = FVector::ZeroVector;
	UPROPERTY() float           Size            = 1.f;
	UPROPERTY() float           Tension         = 0.f;
	UPROPERTY() float           Charge          = 0.f;
	UPROPERTY() float           BreathPhase     = 0.f;
	UPROPERTY() float           BreathAmplitude = 0.f;
	UPROPERTY() bool            bAnchored       = false;
	UPROPERTY() TArray<FName>   Tags;
};

// Field link — the nervous system connection between two nodes.
USTRUCT(BlueprintType)
struct GRAVITYENTITYRUNTIME_API FGravityLink
{
	GENERATED_BODY()

	UPROPERTY() int32 StartNodeID  = INDEX_NONE;
	UPROPERTY() int32 EndNodeID    = INDEX_NONE;
	UPROPERTY() float Tension      = 0.f;
	UPROPERTY() float Instability  = 0.f;
	UPROPERTY() float GlowIntensity = 0.f;
	UPROPERTY() float Thickness    = 1.f;
};
