#include "GravityEntityComponent.h"
#include "GravityEntityProfile.h"
#include "GravityStateChannels.h"
#include "GravityPartCache.h"
#include "GravityTopologySolver.h"
#include "GravityMovementSolver.h"
#include "GravityBreathSignal.h"
#include "GravityWormMovementSolver.h"
#include "GravityOrbitalMovementSolver.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

// ---------------------------------------------------------------------------
// CVars
// ---------------------------------------------------------------------------
static TAutoConsoleVariable<int32> CVarDrawNodes(
	TEXT("ge.Debug.DrawNodes"), 1,
	TEXT("Draw entity nodes as debug spheres (1=on)."), ECVF_Default);

static TAutoConsoleVariable<int32> CVarDrawLinks(
	TEXT("ge.Debug.DrawLinks"), 1,
	TEXT("Draw entity field links as debug lines (1=on)."), ECVF_Default);

static TAutoConsoleVariable<int32> CVarLogChannels(
	TEXT("ge.Debug.LogChannels"), 0,
	TEXT("Log state channel values each tick (1=on)."), ECVF_Default);

// ---------------------------------------------------------------------------
// Role colors (used by debug draw)
// ---------------------------------------------------------------------------
static FColor RoleColor(EGravityNodeRole Role)
{
	switch (Role)
	{
	case EGravityNodeRole::Lead:        return FColor::Red;
	case EGravityNodeRole::Core:        return FColor::Yellow;
	case EGravityNodeRole::Anchor:      return FColor::White;
	case EGravityNodeRole::Shield:      return FColor::Blue;
	case EGravityNodeRole::Orbital:     return FColor::Magenta;
	case EGravityNodeRole::ScoutTip:    return FColor::Orange;
	default:                            return FColor::Cyan; // Spine and rest
	}
}

// ---------------------------------------------------------------------------

UGravityEntityComponent::UGravityEntityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGravityEntityComponent::BeginPlay()
{
	Super::BeginPlay();

	StateChannels = NewObject<UGravityStateChannels>(this);
	PartCache     = NewObject<UGravityPartCache>(this);

	InitializeEntity();
}

void UGravityEntityComponent::InitializeEntity()
{
	if (!Profile) return;

	Nodes.Reset();
	Links.Reset();
	DisplayPositions.Reset();

	if (StateChannels)
	{
		StateChannels->Reset();
	}

	if (Profile->TopologySolver)
	{
		Profile->TopologySolver->BuildTopology(Nodes, Links, Profile);

		// Offset nodes to actor world location
		const FVector Origin = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
		for (FGravityNode& Node : Nodes)
		{
			Node.Position       += Origin;
			Node.TargetPosition += Origin;
		}
	}

	DisplayPositions.SetNum(Nodes.Num());

	// Give the movement solver its spawn origin for autonomous locomotion
	const FVector Origin = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	if (UGravityWormMovementSolver* WormSolver = Cast<UGravityWormMovementSolver>(Profile->MovementSolver))
	{
		WormSolver->SetSpawnOrigin(Origin);
	}
	else if (UGravityOrbitalMovementSolver* OrbSolver = Cast<UGravityOrbitalMovementSolver>(Profile->MovementSolver))
	{
		OrbSolver->SetSpawnOrigin(Origin);
	}
}

void UGravityEntityComponent::ReinitializeEntity()
{
	// Called from PostEditChangeProperty (editor, pre-BeginPlay) — create objects if not yet initialized.
	if (!StateChannels)
	{
		StateChannels = NewObject<UGravityStateChannels>(this);
	}
	if (!PartCache)
	{
		PartCache = NewObject<UGravityPartCache>(this);
	}
	InitializeEntity();
}

void UGravityEntityComponent::SetAttentionTarget(FVector WorldTarget)
{
	if (!Profile) return;
	if (UGravityWormMovementSolver* WormSolver = Cast<UGravityWormMovementSolver>(Profile->MovementSolver))
	{
		WormSolver->SetAttentionTarget(WorldTarget);
	}
}

void UGravityEntityComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Profile || Nodes.Num() == 0) return;

	if (Profile->MovementSolver)
	{
		Profile->MovementSolver->Solve(Nodes, Links, DeltaTime, StateChannels);
	}

	if (Profile->BreathSignal)
	{
		Profile->BreathSignal->Update(Nodes.Num(), DeltaTime);

		// Write breath channels from lead node
		StateChannels->BreathPhase     = Profile->BreathSignal->GetPhase(0);
		StateChannels->BreathAmplitude = Profile->BreathSignal->GetAmplitude(0);
	}

	ComputeDisplayPositions();
	DebugDraw();

	if (CVarLogChannels.GetValueOnGameThread())
	{
		UE_LOG(LogTemp, Log, TEXT("[GE] T=%.2f Spd=%.1f Inst=%.2f BrPh=%.2f"),
			StateChannels->Tension, StateChannels->Speed,
			StateChannels->Instability, StateChannels->BreathPhase);
	}
}

void UGravityEntityComponent::ComputeDisplayPositions()
{
	DisplayPositions.SetNum(Nodes.Num());

	const bool bHasBreath = (Profile && Profile->BreathSignal != nullptr);

	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		FVector Offset = FVector::ZeroVector;

		if (bHasBreath)
		{
			float Phase = Profile->BreathSignal->GetPhase(i);
			float Amp   = Profile->BreathSignal->GetAmplitude(i);
			// Vertical bob in world units: Amp is 0-1, scale to a fraction of RestSpacing
			const float BreathScale = Profile->RestSpacing * 0.25f;
			Offset.Z = Amp * BreathScale * FMath::Sin(Phase);
		}

		DisplayPositions[i] = Nodes[i].Position + Offset;
	}
}

void UGravityEntityComponent::DebugDraw() const
{
	UWorld* W = GetWorld();
	if (!W) return;

	const bool bDrawNodes = CVarDrawNodes.GetValueOnGameThread() != 0;
	const bool bDrawLinks = CVarDrawLinks.GetValueOnGameThread() != 0;

	if (!bDrawNodes && !bDrawLinks) return;

	const float NodeRadius = 14.f;

	for (int32 i = 0; i < DisplayPositions.Num(); ++i)
	{
		if (bDrawNodes)
		{
			DrawDebugSphere(W, DisplayPositions[i], NodeRadius, 8,
				RoleColor(Nodes[i].Role), false, -1.f, 0, 1.f);
		}

		if (bDrawLinks && i < DisplayPositions.Num() - 1)
		{
			// Tint link from green (relaxed) toward red (high tension)
			float T = FMath::Clamp(Nodes[i].Tension * 4.f, 0.f, 1.f);
			FColor LinkColor = FColor(
				FMath::RoundToInt(T * 255.f),
				FMath::RoundToInt((1.f - T) * 200.f),
				0);

			DrawDebugLine(W, DisplayPositions[i], DisplayPositions[i + 1],
				LinkColor, false, -1.f, 0, 2.f);
		}
	}
}

#if WITH_EDITOR
void UGravityEntityComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UGravityEntityComponent, Profile))
	{
		ReinitializeEntity();
	}
}
#endif
