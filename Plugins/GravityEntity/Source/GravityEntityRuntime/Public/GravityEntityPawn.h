#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GravityEntityPawn.generated.h"

class UGravityEntityComponent;
class USceneComponent;

// Shell pawn that hosts a GravityEntityComponent.
// M1: will expose console command ge.SpawnWorm and handle possession.
UCLASS()
class GRAVITYENTITYRUNTIME_API AGravityEntityPawn : public APawn
{
	GENERATED_BODY()

public:
	AGravityEntityPawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GravityEntity")
	TObjectPtr<UGravityEntityComponent> GravityEntityComponent;

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot;
};
