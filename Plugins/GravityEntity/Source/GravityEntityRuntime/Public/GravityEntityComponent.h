#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GravityEntityTypes.h"
#include "GravityEntityComponent.generated.h"

class UGravityEntityProfile;
class UGravityStateChannels;
class UGravityPartCache;
class UProceduralMeshComponent;

// Drives entity simulation: owns nodes/links, ticks solvers, owns state channels.
// Renders nodes via UProceduralMeshComponent (one per node) with hand-computed shapes,
// and link tubes (one per link). Both use CustomPrimitiveData[0] for per-primitive glow.
UCLASS(ClassGroup = "GravityEntity", meta = (BlueprintSpawnableComponent))
class GRAVITYENTITYRUNTIME_API UGravityEntityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGravityEntityComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GravityEntity")
	TObjectPtr<UGravityEntityProfile> Profile;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GravityEntity|State")
	TObjectPtr<UGravityStateChannels> StateChannels;

	TArray<FGravityNode> Nodes;
	TArray<FGravityLink> Links;
	TArray<FVector>      DisplayPositions;

	// Material applied to node meshes. Must use CustomPrimitiveData node (index 0) for glow.
	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering")
	TObjectPtr<UMaterialInterface> NodeMaterial;

	// Material applied to link tubes. Falls back to NodeMaterial if unset.
	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering")
	TObjectPtr<UMaterialInterface> LinkMaterial;

	// Radius of link tube cylinders (cm).
	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering", meta = (ClampMin = "0.5", ClampMax = "20.0"))
	float LinkTubeRadius = 3.f;

	// Glow phase lead over geometry breath (radians). Positive = glow brightens before node moves.
	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering", meta = (ClampMin = "0.0", ClampMax = "3.14"))
	float GlowLeadAngle = 0.35f;

	UFUNCTION(BlueprintCallable, Category = "GravityEntity")
	void SetAttentionTarget(FVector WorldTarget);

	// Full reinit including mesh components — call only at BeginPlay or from game code.
	void ReinitializeEntity();

	// Logical-only rebuild: resets solver + topology without touching UObject creation.
	// Safe to call from PostEditChangeProperty.
	void RebuildLogicOnly();

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

	UPROPERTY()
	TArray<TObjectPtr<UProceduralMeshComponent>> NodeMeshComponents;

	UPROPERTY()
	TArray<TObjectPtr<UProceduralMeshComponent>> LinkMeshComponents;

	void InitializeEntity();
	void DestroyMeshComponents();
	void CreateNodeMeshComponents();
	void CreateLinkMeshComponents();
	void ComputeDisplayPositions();
	void UpdateNodeMeshes();
	void DebugDraw() const;
};
