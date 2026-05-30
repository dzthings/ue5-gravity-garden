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
#include "ProceduralMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

// ---------------------------------------------------------------------------
// CVars
// ---------------------------------------------------------------------------
static TAutoConsoleVariable<int32> CVarDrawLinks(
	TEXT("ge.Debug.DrawLinks"), 0,
	TEXT("Overlay debug link lines (1=on; tube meshes are rendered by default)."), ECVF_Default);

static TAutoConsoleVariable<int32> CVarLogChannels(
	TEXT("ge.Debug.LogChannels"), 0,
	TEXT("Log state channel values each tick (1=on)."), ECVF_Default);

// ---------------------------------------------------------------------------
// Unit cylinder geometry (height=1, radius=1, Z-centred) for link tubes.
// Component scale (XY=tubeRadius, Z=linkLength) stretches it to fit each tick.
// ---------------------------------------------------------------------------
static void BuildUnitCylinder(int32 RadialSeg,
	TArray<FVector>& Verts, TArray<int32>& Tris,
	TArray<FVector>& Normals, TArray<FVector2D>& UVs)
{
	Verts.Reset(); Tris.Reset(); Normals.Reset(); UVs.Reset();

	// Side vertices: top ring then bottom ring
	for (int32 ring = 0; ring < 2; ++ring)
	{
		float Z = (ring == 0) ? 0.5f : -0.5f;
		for (int32 j = 0; j <= RadialSeg; ++j)
		{
			float Phi = UE_TWO_PI * j / RadialSeg;
			float Cx  = FMath::Cos(Phi);
			float Cy  = FMath::Sin(Phi);
			Verts.Add(FVector(Cx, Cy, Z));
			Normals.Add(FVector(Cx, Cy, 0.f));
			UVs.Add(FVector2D((float)j / RadialSeg, ring));
		}
	}

	// Side triangles
	for (int32 j = 0; j < RadialSeg; ++j)
	{
		int32 A = j;
		int32 B = j + RadialSeg + 1;
		Tris.Add(A);     Tris.Add(B);     Tris.Add(A + 1);
		Tris.Add(B);     Tris.Add(B + 1); Tris.Add(A + 1);
	}

	// Top cap
	int32 TopCentre = Verts.Num();
	Verts.Add(FVector(0, 0, 0.5f)); Normals.Add(FVector::UpVector); UVs.Add(FVector2D(0.5f, 0.f));
	for (int32 j = 0; j < RadialSeg; ++j)
	{
		Tris.Add(TopCentre); Tris.Add(j + 1); Tris.Add(j);
	}

	// Bottom cap
	int32 BotCentre = Verts.Num();
	int32 BotRing   = RadialSeg + 1;
	Verts.Add(FVector(0, 0, -0.5f)); Normals.Add(-FVector::UpVector); UVs.Add(FVector2D(0.5f, 1.f));
	for (int32 j = 0; j < RadialSeg; ++j)
	{
		Tris.Add(BotCentre); Tris.Add(BotRing + j); Tris.Add(BotRing + j + 1);
	}
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static UProceduralMeshComponent* SpawnPMC(AActor* Owner, UMaterialInterface* Mat)
{
	UProceduralMeshComponent* PMC = NewObject<UProceduralMeshComponent>(Owner);
	PMC->SetupAttachment(Owner->GetRootComponent());
	PMC->bUseAsyncCooking = false;
	PMC->RegisterComponent();
	if (Mat) PMC->SetMaterial(0, Mat);
	return PMC;
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

void UGravityEntityComponent::RebuildLogicOnly()
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
}

void UGravityEntityComponent::DestroyMeshComponents()
{
	for (auto& C : NodeMeshComponents) { if (C) C->DestroyComponent(); }
	NodeMeshComponents.Reset();
	for (auto& C : LinkMeshComponents) { if (C) C->DestroyComponent(); }
	LinkMeshComponents.Reset();
}

void UGravityEntityComponent::CreateNodeMeshComponents()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Profile) return;

	// Pre-build fallback sphere data once
	TArray<FVector> FbVerts, FbNorms; TArray<int32> FbTris; TArray<FVector2D> FbUVs;
	bool bFallbackBuilt = false;

	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		UProceduralMeshComponent* PMC = SpawnPMC(Owner, NodeMaterial);

		TArray<FVector> V, N; TArray<int32> T; TArray<FVector2D> UV;

		if (UGravityPartGenerator* Gen = Profile->FindGeneratorForRole(Nodes[i].Role))
		{
			Gen->GenerateMeshData(V, T, N, UV);
		}
		else
		{
			// Fallback: small UV sphere, built once and reused
			if (!bFallbackBuilt)
			{
				// 8-step UV sphere, radius 15cm
				const int32 S = 8;
				for (int32 a = 0; a <= S; ++a)
				{
					float Theta = UE_PI * a / S;
					for (int32 b = 0; b <= S; ++b)
					{
						float Phi = UE_TWO_PI * b / S;
						FVector Nrm(FMath::Sin(Theta)*FMath::Cos(Phi), FMath::Sin(Theta)*FMath::Sin(Phi), FMath::Cos(Theta));
						FbVerts.Add(Nrm * 15.f); FbNorms.Add(Nrm);
						FbUVs.Add(FVector2D((float)b/S, (float)a/S));
					}
				}
				for (int32 a = 0; a < S; ++a)
					for (int32 b = 0; b < S; ++b)
					{
						int32 A = a*(S+1)+b, B = A+S+1;
						FbTris.Add(A); FbTris.Add(B); FbTris.Add(A+1);
						FbTris.Add(B); FbTris.Add(B+1); FbTris.Add(A+1);
					}
				bFallbackBuilt = true;
			}
			V = FbVerts; T = FbTris; N = FbNorms; UV = FbUVs;
		}

		PMC->CreateMeshSection_LinearColor(0, V, T, N, UV, {}, {}, false);
		NodeMeshComponents.Add(PMC);
	}
}

void UGravityEntityComponent::CreateLinkMeshComponents()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Profile) return;

	UMaterialInterface* Mat = LinkMaterial ? LinkMaterial : NodeMaterial;

	TArray<FVector> V, N; TArray<int32> T; TArray<FVector2D> UV;
	BuildUnitCylinder(8, V, T, N, UV);

	for (int32 i = 0; i < Links.Num(); ++i)
	{
		UProceduralMeshComponent* PMC = SpawnPMC(Owner, Mat);
		PMC->CreateMeshSection_LinearColor(0, V, T, N, UV, {}, {}, false);
		LinkMeshComponents.Add(PMC);
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
	const bool bHasBreath = Profile && Profile->BreathSignal;

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

	const bool bHasBreath   = Profile && Profile->BreathSignal;
	const bool bHasMaterial = Profile && Profile->MaterialProfile;

	// Node transforms + glow
	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		UProceduralMeshComponent* PMC = NodeMeshComponents[i];
		if (!PMC) continue;

		// Orient +Z along spine direction
		FVector Dir = FVector::UpVector;
		if (i < DisplayPositions.Num() - 1)
			Dir = (DisplayPositions[i + 1] - DisplayPositions[i]).GetSafeNormal();
		else if (i > 0)
			Dir = (DisplayPositions[i] - DisplayPositions[i - 1]).GetSafeNormal();
		if (Dir.IsNearlyZero()) Dir = FVector::UpVector;

		FQuat Rot = FQuat::FindBetweenNormals(FVector::UpVector, Dir);
		PMC->SetWorldTransform(FTransform(Rot, DisplayPositions[i]));

		float BreathCPD = bHasBreath
			? 0.5f + 0.5f * FMath::Sin(Profile->BreathSignal->GetPhase(i) + GlowLeadAngle)
			: 0.5f;
		float Glow = bHasMaterial
			? Profile->MaterialProfile->ComputeNodeGlow(BreathCPD, Nodes[i].Tension)
			: BreathCPD;
		PMC->SetCustomPrimitiveDataFloat(0, Glow);
	}

	// Link tube transforms + glow
	for (int32 i = 0; i < Links.Num(); ++i)
	{
		UProceduralMeshComponent* PMC = LinkMeshComponents.IsValidIndex(i) ? LinkMeshComponents[i] : nullptr;
		if (!PMC) continue;

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
		// Unit cylinder: radius 1, height 1. Scale XY = tube radius, Z = link length.
		PMC->SetWorldTransform(FTransform(Rot, Mid, FVector(LinkTubeRadius, LinkTubeRadius, Len)));

		float Stretch = FMath::Abs(Len - Profile->RestSpacing) / FMath::Max(Profile->RestSpacing, 1.f);
		float Glow    = bHasMaterial
			? Profile->MaterialProfile->ComputeLinkGlow(Stretch)
			: FMath::Clamp(0.2f + Stretch * 2.f, 0.f, 1.f);
		PMC->SetCustomPrimitiveDataFloat(0, Glow);
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

void UGravityEntityComponent::DebugDraw() const
{
	if (!CVarDrawLinks.GetValueOnGameThread()) return;

	UWorld* W = GetWorld();
	if (!W) return;

	for (int32 i = 0; i < DisplayPositions.Num() - 1; ++i)
	{
		float T   = FMath::Clamp(Nodes[i].Tension * 4.f, 0.f, 1.f);
		FColor LC = FColor(FMath::RoundToInt(T * 255.f), FMath::RoundToInt((1.f - T) * 200.f), 0);
		DrawDebugLine(W, DisplayPositions[i], DisplayPositions[i + 1], LC, false, -1.f, 0, 2.f);
	}
}

#if WITH_EDITOR
void UGravityEntityComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Rebuild logical state only — never create/destroy UObjects from here.
	// ProceduralMeshComponents are created at BeginPlay (game world).
	// Creating UObjects during PostEditChangeProperty crashes because the
	// editor may be iterating the UObject hash table at call time.
	const FName Prop = PropertyChangedEvent.GetPropertyName();
	if (Prop == GET_MEMBER_NAME_CHECKED(UGravityEntityComponent, Profile)     ||
	    Prop == GET_MEMBER_NAME_CHECKED(UGravityEntityComponent, NodeMaterial) ||
	    Prop == GET_MEMBER_NAME_CHECKED(UGravityEntityComponent, LinkMaterial) ||
	    Prop == GET_MEMBER_NAME_CHECKED(UGravityEntityComponent, LinkTubeRadius))
	{
		RebuildLogicOnly();
	}
}
#endif
