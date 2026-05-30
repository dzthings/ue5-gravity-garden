#include "GravityEntityPawn.h"
#include "GravityEntityComponent.h"

AGravityEntityPawn::AGravityEntityPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	GravityEntityComponent = CreateDefaultSubobject<UGravityEntityComponent>(TEXT("GravityEntityComponent"));
}
