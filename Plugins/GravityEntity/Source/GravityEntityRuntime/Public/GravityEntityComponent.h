#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GravityEntityTypes.h"
#include "GravityEntityComponent.generated.h"

class UGravityEntityProfile;
class UGravityStateChannels;
class UGravityPartCache;

// Drives entity simulation: owns nodes/links, ticks solvers, owns state channels.
// Attach to AGravityEntityPawn (or any actor). Profile is the variant asset.
UCLASS(ClassGroup = "GravityEntity", meta = (BlueprintSpawnableComponent))
class GRAVITYENTITYRUNTIME_API UGravityEntityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGravityEntityComponent();

	// The variant that describes this entity.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GravityEntity")
	TObjectPtr<UGravityEntityProfile> Profile;

	// Live state signals — written each tick, read by materials/effects/breath.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GravityEntity|State")
	TObjectPtr<UGravityStateChannels> StateChannels;

	// Runtime node + link arrays (populated by the topology solver at spawn).
	TArray<FGravityNode> Nodes;
	TArray<FGravityLink> Links;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TObjectPtr<UGravityPartCache> PartCache;

	void InitializeEntity();
};
