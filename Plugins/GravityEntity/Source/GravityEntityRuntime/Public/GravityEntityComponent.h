#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GravityEntityTypes.h"
#include "GravityEntityComponent.generated.h"

class UGravityEntityProfile;
class UGravityStateChannels;
class UGravityPartCache;
class UInstancedStaticMeshComponent;

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

	// World-space display positions (physics position + breath offset).
	TArray<FVector> DisplayPositions;

	// --- Rendering ---
	// Mesh used for each node. Assign Engine/BasicShapes/Sphere in Details.
	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering")
	TObjectPtr<UStaticMesh> NodeMesh;

	// Material applied to NodeISM. Assign M_GravityNode. Must read CustomPrimitiveData[0] for glow.
	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering")
	TObjectPtr<UMaterialInterface> NodeMaterial;

	// Uniform scale of each node mesh instance.
	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering", meta = (ClampMin = "0.01", ClampMax = "10.0"))
	float NodeScale = 0.25f;

	// Glow phase lead over geometry breath (radians). Positive = glow brightens before the node bobs.
	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering", meta = (ClampMin = "0.0", ClampMax = "3.14"))
	float GlowLeadAngle = 0.35f;

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

	// Runtime ISM — one instance per node, updated each tick.
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> NodeISM;

	void InitializeEntity();
	void RebuildISMInstances();
	void UpdateISM();
	void ComputeDisplayPositions();
	void DebugDraw() const;
};
