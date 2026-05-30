#include "GravityEntityComponent.h"
#include "GravityEntityProfile.h"
#include "GravityStateChannels.h"
#include "GravityPartCache.h"
#include "GravityTopologySolver.h"
#include "GravityMovementSolver.h"
#include "GravityBreathSignal.h"
#include "GravityWormMovementSolver.h"
#include "GravityOrbitalMovementSolver.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

// ---------------------------------------------------------------------------
// CVars
// ---------------------------------------------------------------------------
static TAutoConsoleVariable<int32> CVarDrawNodes(
	TEXT("ge.Debug.DrawNodes"), 0,
	TEXT("Draw entity nodes as debug spheres (1=on)."), ECVF_Default);

static TAutoConsoleVariable<int32> CVarDrawLinks(
	TEXT("ge.Debug.DrawLinks"), 1,
	TEXT("Draw entity field links as debug lines (1=on)."), ECVF_Default);

static TAutoConsoleVariable<int32> CVarLogChannels(
	TEXT("ge.Debug.LogChannels"), 0,
	TEXT("Log state channel values each tick (1=on)."), ECVF_Default);

// ---------------------------------------------------------------------------
// Role colors (debug draw)
// ---------------------------------------------------------------------------
static FColor RoleColor(EGravityNodeRole Role)
{
	switch (Role)
	{
	case EGravityNodeRole::Lead:    return FColor::Red;
	case EGravityNodeRole::Core:    return FColor::Yellow;
	case EGravityNodeRole::Anchor:  return FColor::White;
	case EGravityNodeRole::Shield:  return FColor::Blue;
	case EGravityNodeRole::Orbital: return FColor::Magenta;
	case EGravityNodeRole::ScoutTip:return FColor::Orange;
	default:                        return FColor::Cyan;
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

	// Create ISM for node rendering — attached to owner root, lives for the actor's lifetime.
	if (AActor* Owner = GetOwner())
	{
		NodeISM = NewObject<UInstancedStaticMeshComponent>(Owner, TEXT("NodeISM"));
		NodeISM->NumCustomDataFloats = 1; // [0] = glow intensity
		NodeISM->SetupAttachment(Owner->GetRootComponent());
		NodeISM->RegisterComponent();

		if (NodeMesh)     NodeISM->SetStaticMesh(NodeMesh);
		if (NodeMaterial) NodeISM->SetMaterial(0, NodeMaterial);
	}

	InitializeEntity();
}

void UGravityEntityComponent::InitializeEntity()
{
	if (!Profile) return;

	Nodes.Reset();
	Links.Reset();
	DisplayPositions.Reset();

	if (StateChannels) StateChannels->Reset();
	if (Profile->MovementSolver) Profile->MovementSolver->Reset();

	if (Profile->TopologySolver)
	{
		Profile->TopologySolver->BuildTopology(Nodes, Links, Profile);

		const FVector Origin = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
		for (FGravityNode& Node : Nodes)
		{
			Node.Position       += Origin;
			Node.TargetPosition += Origin;
		}
	}

	DisplayPositions.SetNum(Nodes.Num());

	const FVector Origin = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	if (UGravityWormMovementSolver* S = Cast<UGravityWormMovementSolver>(Profile->MovementSolver))
	{
		S->SetSpawnOrigin(Origin);
	}
	else if (UGravityOrbitalMovementSolver* S = Cast<UGravityOrbitalMovementSolver>(Profile->MovementSolver))
	{
		S->SetSpawnOrigin(Origin);
	}

	RebuildISMInstances();
}

void UGravityEntityComponent::RebuildISMInstances()
{
	if (!NodeISM) return;

	NodeISM->ClearInstances();

	const FVector Scale(NodeScale);
	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		FTransform T(FQuat::Identity, DisplayPositions.IsValidIndex(i) ? DisplayPositions[i] : Nodes[i].Position, Scale);
		NodeISM->AddInstance(T, /*bWorldSpace=*/true);
	}
}

void UGravityEntityComponent::ReinitializeEntity()
{
	if (!StateChannels) StateChannels = NewObject<UGravityStateChannels>(this);
	if (!PartCache)     PartCache     = NewObject<UGravityPartCache>(this);
	InitializeEntity();
}

void UGravityEntityComponent::SetAttentionTarget(FVector WorldTarget)
{
	if (!Profile) return;
	if (UGravityWormMovementSolver* S = Cast<UGravityWormMovementSolver>(Profile->MovementSolver))
	{
		S->SetAttentionTarget(WorldTarget);
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
		StateChannels->BreathPhase     = Profile->BreathSignal->GetPhase(0);
		StateChannels->BreathAmplitude = Profile->BreathSignal->GetAmplitude(0);
	}

	ComputeDisplayPositions();
	UpdateISM();
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
	const bool bHasBreath = Profile && Profile->BreathSignal != nullptr;

	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		FVector Offset = FVector::ZeroVector;
		if (bHasBreath)
		{
			float Phase = Profile->BreathSignal->GetPhase(i);
			float Amp   = Profile->BreathSignal->GetAmplitude(i);
			Offset.Z    = Amp * (Profile->RestSpacing * 0.25f) * FMath::Sin(Phase);
		}
		DisplayPositions[i] = Nodes[i].Position + Offset;
	}
}

void UGravityEntityComponent::UpdateISM()
{
	if (!NodeISM || NodeISM->GetInstanceCount() != Nodes.Num()) return;

	const bool bHasBreath = Profile && Profile->BreathSignal != nullptr;
	const FVector Scale(NodeScale);

	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		// Update world-space transform
		FTransform T(FQuat::Identity, DisplayPositions[i], Scale);
		NodeISM->UpdateInstanceTransform(i, T, /*bWorldSpace=*/true, /*bMarkDirty=*/false);

		// CPD[0]: glow = 0.5 + 0.5*sin(breathPhase + leadAngle) → maps to [0,1]
		float GlowPhase = (bHasBreath ? Profile->BreathSignal->GetPhase(i) : 0.f) + GlowLeadAngle;
		float Glow      = 0.5f + 0.5f * FMath::Sin(GlowPhase);
		NodeISM->SetCustomDataValue(i, 0, Glow, /*bMarkDirty=*/false);
	}

	NodeISM->MarkRenderStateDirty();
}

void UGravityEntityComponent::DebugDraw() const
{
	UWorld* W = GetWorld();
	if (!W) return;

	const bool bDrawNodes = CVarDrawNodes.GetValueOnGameThread() != 0;
	const bool bDrawLinks = CVarDrawLinks.GetValueOnGameThread() != 0;
	if (!bDrawNodes && !bDrawLinks) return;

	for (int32 i = 0; i < DisplayPositions.Num(); ++i)
	{
		if (bDrawNodes)
		{
			DrawDebugSphere(W, DisplayPositions[i], 14.f, 8,
				RoleColor(Nodes[i].Role), false, -1.f, 0, 1.f);
		}
		if (bDrawLinks && i < DisplayPositions.Num() - 1)
		{
			float    T  = FMath::Clamp(Nodes[i].Tension * 4.f, 0.f, 1.f);
			FColor   LC = FColor(FMath::RoundToInt(T * 255.f), FMath::RoundToInt((1.f - T) * 200.f), 0);
			DrawDebugLine(W, DisplayPositions[i], DisplayPositions[i + 1], LC, false, -1.f, 0, 2.f);
		}
	}
}

#if WITH_EDITOR
void UGravityEntityComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName Prop = PropertyChangedEvent.GetPropertyName();
	if (Prop == GET_MEMBER_NAME_CHECKED(UGravityEntityComponent, Profile) ||
	    Prop == GET_MEMBER_NAME_CHECKED(UGravityEntityComponent, NodeMesh) ||
	    Prop == GET_MEMBER_NAME_CHECKED(UGravityEntityComponent, NodeMaterial) ||
	    Prop == GET_MEMBER_NAME_CHECKED(UGravityEntityComponent, NodeScale))
	{
		ReinitializeEntity();
		if (NodeISM)
		{
			if (NodeMesh)     NodeISM->SetStaticMesh(NodeMesh);
			if (NodeMaterial) NodeISM->SetMaterial(0, NodeMaterial);
		}
	}
}
#endif
