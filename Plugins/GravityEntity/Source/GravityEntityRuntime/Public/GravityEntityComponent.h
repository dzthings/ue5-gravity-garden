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

	// Physics node + link arrays (populated by topology solver at spawn).
	TArray<FGravityNode> Nodes;
	TArray<FGravityLink> Links;

	// World-space display positions (physics position + breath offset). Used by debug draw and (M4) meshes.
	TArray<FVector> DisplayPositions;

	// Drive the lead node's attention toward a world-space target.
	UFUNCTION(BlueprintCallable, Category = "GravityEntity")
	void SetAttentionTarget(FVector WorldTarget);

	// Rebuild topology + reset simulation state (call after profile change in editor).
	void ReinitializeEntity();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	UPROPERTY()
	TObjectPtr<UGravityPartCache> PartCache;

	void InitializeEntity();
	void ComputeDisplayPositions();
	void DebugDraw() const;
};
