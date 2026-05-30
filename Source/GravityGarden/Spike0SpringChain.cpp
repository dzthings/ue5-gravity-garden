#include "Spike0SpringChain.h"
#include "DrawDebugHelpers.h"

ASpike0SpringChain::ASpike0SpringChain()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void ASpike0SpringChain::BeginPlay()
{
	Super::BeginPlay();
	InitChain();
}

void ASpike0SpringChain::InitChain()
{
	Nodes.Reset();
	const FVector Origin = GetActorLocation();
	for (int32 i = 0; i < NodeCount; ++i)
	{
		FNode N;
		N.Position  = Origin + FVector(0.f, 0.f, -i * RestSpacing); // hang downward
		N.Velocity  = FVector::ZeroVector;
		N.bAnchored = (i == 0);
		Nodes.Add(N);
	}
}

void ASpike0SpringChain::StepSimulation(float Dt)
{
	if (Nodes.Num() < 2) return;

	// Accumulate spring-damper forces then integrate
	TArray<FVector> Forces;
	Forces.SetNumZeroed(Nodes.Num());

	const float Gravity = -980.f; // cm/s² (UE units)

	for (int32 i = 0; i < Nodes.Num() - 1; ++i)
	{
		FNode& A = Nodes[i];
		FNode& B = Nodes[i + 1];

		FVector Delta      = B.Position - A.Position;
		float   Len        = Delta.Size();
		FVector DirAB      = (Len > KINDA_SMALL_NUMBER) ? Delta / Len : FVector::ForwardVector;
		float   Stretch    = Len - RestSpacing;
		FVector RelVel     = B.Velocity - A.Velocity;
		float   DampForce  = FVector::DotProduct(RelVel, DirAB);

		FVector Spring = DirAB * (Stiffness * Stretch + Damping * DampForce);

		if (!A.bAnchored) Forces[i]     += Spring;
		if (!B.bAnchored) Forces[i + 1] -= Spring;
	}

	// Gravity + integrate
	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		FNode& N = Nodes[i];
		if (N.bAnchored) continue;

		Forces[i].Z += Gravity;
		N.Velocity   = N.Velocity + Forces[i] * Dt;
		N.Position   = N.Position + N.Velocity  * Dt;
	}
}

void ASpike0SpringChain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Keep anchor pinned to actor world location
	if (Nodes.Num() > 0)
	{
		Nodes[0].Position = GetActorLocation();
		Nodes[0].Velocity = FVector::ZeroVector;
	}

	// Sub-step for stability at low frame rates
	const int32 Steps   = 4;
	const float SubStep = DeltaTime / Steps;
	for (int32 s = 0; s < Steps; ++s)
	{
		StepSimulation(SubStep);
	}

	// Debug draw
	UWorld* W = GetWorld();
	if (!W) return;

	const FColor NodeColor = FColor::Cyan;
	const FColor LinkColor = FColor::Yellow;
	const FColor AnchorColor = FColor::Red;

	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		const FColor& C = Nodes[i].bAnchored ? AnchorColor : NodeColor;
		DrawDebugSphere(W, Nodes[i].Position, SphereRadius, 8, C, false, -1.f, 0, 1.f);

		if (i < Nodes.Num() - 1)
		{
			DrawDebugLine(W, Nodes[i].Position, Nodes[i + 1].Position, LinkColor, false, -1.f, 0, 1.5f);
		}
	}
}

#if WITH_EDITOR
void ASpike0SpringChain::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropName = PropertyChangedEvent.GetPropertyName();
	if (PropName == GET_MEMBER_NAME_CHECKED(ASpike0SpringChain, NodeCount) ||
	    PropName == GET_MEMBER_NAME_CHECKED(ASpike0SpringChain, RestSpacing))
	{
		InitChain();
	}
}
#endif
