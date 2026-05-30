#include "GravityEntityComponent.h"
#include "GravityEntityProfile.h"
#include "GravityStateChannels.h"
#include "GravityPartCache.h"
#include "GravityPartGenerator.h"
#include "GravityTopologySolver.h"
#include "GravityMovementSolver.h"
#include "GravityBreathSignal.h"
#include "GravityMaterialProfile.h"
#include "GravityWormMovementSolver.h"
#include "GravityOrbitalMovementSolver.h"
#include "Components/DynamicMeshComponent.h"
#include "UDynamicMesh.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

// ---------------------------------------------------------------------------
// CVars
// ---------------------------------------------------------------------------
static TAutoConsoleVariable<int32> CVarDrawLinks(
	TEXT("ge.Debug.DrawLinks"), 0,
	TEXT("Draw field links as debug lines (1=on; link tubes are rendered by default)."), ECVF_Default);

static TAutoConsoleVariable<int32> CVarLogChannels(
	TEXT("ge.Debug.LogChannels"), 0,
	TEXT("Log state channel values each tick (1=on)."), ECVF_Default);

// ---------------------------------------------------------------------------

UGravityEntityComponent::UGravityEntityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

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
	if (UGravityWormMovementSolver* WormS = Cast<UGravityWormMovementSolver>(Profile->MovementSolver))
	{
		WormS->SetSpawnOrigin(Origin);
	}
	else if (UGravityOrbitalMovementSolver* OrbS = Cast<UGravityOrbitalMovementSolver>(Profile->MovementSolver))
	{
		OrbS->SetSpawnOrigin(Origin);
	}

	DestroyMeshComponents();
	CreateNodeMeshComponents();
	CreateLinkMeshComponents();
}

void UGravityEntityComponent::ReinitializeEntity()
{
	if (!StateChannels) StateChannels = NewObject<UGravityStateChannels>(this);
	if (!PartCache)     PartCache     = NewObject<UGravityPartCache>(this);
	InitializeEntity();
}

// ---------------------------------------------------------------------------
// Mesh component management
// ---------------------------------------------------------------------------

void UGravityEntityComponent::DestroyMeshComponents()
{
	for (TObjectPtr<UDynamicMeshComponent>& C : NodeMeshComponents)
	{
		if (C) { C->DestroyComponent(); }
	}
	NodeMeshComponents.Reset();

	for (TObjectPtr<UDynamicMeshComponent>& C : LinkMeshComponents)
	{
		if (C) { C->DestroyComponent(); }
	}
	LinkMeshComponents.Reset();
}

static UDynamicMeshComponent* SpawnDMC(AActor* Owner, UMaterialInterface* Mat)
{
	UDynamicMeshComponent* DMC = NewObject<UDynamicMeshComponent>(Owner);
	DMC->SetupAttachment(Owner->GetRootComponent());
	DMC->RegisterComponent();
	if (Mat) DMC->SetMaterial(0, Mat);
	return DMC;
}

void UGravityEntityComponent::CreateNodeMeshComponents()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Profile) return;

	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		UDynamicMeshComponent* DMC = SpawnDMC(Owner, NodeMaterial);

		UDynamicMesh* Mesh = DMC->GetDynamicMesh();
		Mesh->Reset();

		if (UGravityPartGenerator* Gen = Profile->FindGeneratorForRole(Nodes[i].Role))
		{
			Gen->GenerateMesh(Mesh);
		}
		else
		{
			// Fallback: small sphere
			FGeometryScriptPrimitiveOptions Opts;
			UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendSphere(
				Mesh, Opts, FTransform::Identity, 15.f, 8,
				EGeometryScriptPrimitiveOriginMode::Center, nullptr);
		}

		NodeMeshComponents.Add(DMC);
	}
}

void UGravityEntityComponent::CreateLinkMeshComponents()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Profile) return;

	UMaterialInterface* Mat = LinkMaterial ? LinkMaterial : NodeMaterial;

	for (int32 i = 0; i < Links.Num(); ++i)
	{
		UDynamicMeshComponent* DMC = SpawnDMC(Owner, Mat);

		// Unit cylinder (1 cm radius, 1 cm height, Z-centred). Scaled to inter-node distance each tick.
		UDynamicMesh* Mesh = DMC->GetDynamicMesh();
		Mesh->Reset();
		FGeometryScriptPrimitiveOptions Opts;
		UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCylinder(
			Mesh, Opts, FTransform::Identity,
			1.f, 1.f, 8, 1, true,
			EGeometryScriptPrimitiveOriginMode::Center, nullptr);

		LinkMeshComponents.Add(DMC);
	}
}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

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
	UpdateNodeMeshes();
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

void UGravityEntityComponent::UpdateNodeMeshes()
{
	if (NodeMeshComponents.Num() != Nodes.Num()) return;

	const bool bHasBreath    = Profile && Profile->BreathSignal;
	const bool bHasMaterial  = Profile && Profile->MaterialProfile;

	// --- Node meshes ---
	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		UDynamicMeshComponent* DMC = NodeMeshComponents[i];
		if (!DMC) continue;

		// Orientation: align local +Z to spine direction
		FVector Dir = FVector::UpVector;
		if (i < DisplayPositions.Num() - 1)
			Dir = (DisplayPositions[i + 1] - DisplayPositions[i]).GetSafeNormal();
		else if (i > 0)
			Dir = (DisplayPositions[i] - DisplayPositions[i - 1]).GetSafeNormal();
		if (Dir.IsNearlyZero()) Dir = FVector::UpVector;

		FQuat Rot = FQuat::FindBetweenNormals(FVector::UpVector, Dir);
		DMC->SetWorldTransform(FTransform(Rot, DisplayPositions[i]));

		// Glow: breath wave + tension boost
		float BreathCPD = bHasBreath
			? 0.5f + 0.5f * FMath::Sin(Profile->BreathSignal->GetPhase(i) + GlowLeadAngle)
			: 0.5f;
		float Tension = Nodes[i].Tension;
		float Glow    = bHasMaterial
			? Profile->MaterialProfile->ComputeNodeGlow(BreathCPD, Tension)
			: BreathCPD;
		DMC->SetCustomPrimitiveDataFloat(0, Glow);
	}

	// --- Link tubes ---
	for (int32 i = 0; i < Links.Num(); ++i)
	{
		UDynamicMeshComponent* DMC = LinkMeshComponents.IsValidIndex(i) ? LinkMeshComponents[i] : nullptr;
		if (!DMC) continue;

		int32 A = Links[i].StartNodeID;
		int32 B = Links[i].EndNodeID;
		if (!DisplayPositions.IsValidIndex(A) || !DisplayPositions.IsValidIndex(B)) continue;

		FVector PosA = DisplayPositions[A];
		FVector PosB = DisplayPositions[B];
		FVector Mid  = (PosA + PosB) * 0.5f;
		float   Len  = FVector::Dist(PosA, PosB);
		FVector Dir  = (PosB - PosA).GetSafeNormal();
		if (Dir.IsNearlyZero()) Dir = FVector::UpVector;

		FQuat Rot = FQuat::FindBetweenNormals(FVector::UpVector, Dir);
		// Scale: XY = tube radius (unit cylinder has radius 1), Z = link length (unit cylinder height 1)
		DMC->SetWorldTransform(FTransform(Rot, Mid, FVector(LinkTubeRadius, LinkTubeRadius, Len)));

		// Link glow = stretch ratio (compressed or stretched links glow brighter)
		float Stretch = FMath::Abs(Len - Profile->RestSpacing) / FMath::Max(Profile->RestSpacing, 1.f);
		float Glow    = bHasMaterial
			? Profile->MaterialProfile->ComputeLinkGlow(Stretch)
			: FMath::Clamp(0.2f + Stretch * 2.f, 0.f, 1.f);
		DMC->SetCustomPrimitiveDataFloat(0, Glow);
	}
}

void UGravityEntityComponent::SetAttentionTarget(FVector WorldTarget)
{
	if (!Profile) return;
	if (UGravityWormMovementSolver* WormS = Cast<UGravityWormMovementSolver>(Profile->MovementSolver))
	{
		WormS->SetAttentionTarget(WorldTarget);
	}
}

// ---------------------------------------------------------------------------
// Debug draw (optional overlay, off by default now that meshes are rendered)
// ---------------------------------------------------------------------------

void UGravityEntityComponent::DebugDraw() const
{
	if (!CVarDrawLinks.GetValueOnGameThread()) return;

	UWorld* W = GetWorld();
	if (!W) return;

	for (int32 i = 0; i < DisplayPositions.Num() - 1; ++i)
	{
		float T    = FMath::Clamp(Nodes[i].Tension * 4.f, 0.f, 1.f);
		FColor LC  = FColor(FMath::RoundToInt(T * 255.f), FMath::RoundToInt((1.f - T) * 200.f), 0);
		DrawDebugLine(W, DisplayPositions[i], DisplayPositions[i + 1], LC, false, -1.f, 0, 2.f);
	}
}

// ---------------------------------------------------------------------------
// Editor
// ---------------------------------------------------------------------------

#if WITH_EDITOR
void UGravityEntityComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName Prop = PropertyChangedEvent.GetPropertyName();
	if (Prop == GET_MEMBER_NAME_CHECKED(UGravityEntityComponent, Profile)     ||
	    Prop == GET_MEMBER_NAME_CHECKED(UGravityEntityComponent, NodeMaterial) ||
	    Prop == GET_MEMBER_NAME_CHECKED(UGravityEntityComponent, LinkMaterial) ||
	    Prop == GET_MEMBER_NAME_CHECKED(UGravityEntityComponent, LinkTubeRadius))
	{
		ReinitializeEntity();
	}
}
#endif
