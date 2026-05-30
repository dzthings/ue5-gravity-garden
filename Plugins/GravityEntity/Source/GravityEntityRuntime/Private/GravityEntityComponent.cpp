#include "GravityEntityComponent.h"
#include "GravityEntityProfile.h"
#include "GravityStateChannels.h"
#include "GravityPartCache.h"
#include "GravityTopologySolver.h"
#include "GravityMovementSolver.h"
#include "GravityBreathSignal.h"

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
	StateChannels->Reset();

	if (Profile->TopologySolver)
	{
		Profile->TopologySolver->BuildTopology(Nodes, Links, Profile);
	}
}

void UGravityEntityComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Profile) return;

	if (Profile->MovementSolver)
	{
		Profile->MovementSolver->Solve(Nodes, Links, DeltaTime, StateChannels);
	}

	if (Profile->BreathSignal)
	{
		Profile->BreathSignal->Update(Nodes.Num(), DeltaTime);
	}
}
