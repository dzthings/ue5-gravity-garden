#include "GravityEntityPawn.h"
#include "GravityEntityComponent.h"
#include "Components/SceneComponent.h"

AGravityEntityPawn::AGravityEntityPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	GravityEntityComponent = CreateDefaultSubobject<UGravityEntityComponent>(TEXT("GravityEntityComponent"));
}
