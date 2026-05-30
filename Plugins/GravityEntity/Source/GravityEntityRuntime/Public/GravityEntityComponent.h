#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GravityEntityTypes.h"
#include "GravityEntityComponent.generated.h"

class UGravityEntityProfile;
class UGravityStateChannels;
class UGravityPartCache;
class UGravityMovementSolver;
class UGravityBreathSignal;
class UProceduralMeshComponent;

// Drives entity simulation: owns nodes/links, ticks solvers, owns state channels.
// Each entity duplicates the profile's solver and breath signal at init so multiple
// entities using the same profile have independent runtime state.
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

	// Material applied to node meshes. Needs a ScalarParameter named 'GlowValue'.
	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering")
	TObjectPtr<UMaterialInterface> NodeMaterial;

	// Material applied to link tubes. Falls back to NodeMaterial if unset.
	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering")
	TObjectPtr<UMaterialInterface> LinkMaterial;

	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering", meta = (ClampMin = "0.5", ClampMax = "20.0"))
	float LinkTubeRadius = 3.f;

	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering", meta = (ClampMin = "0.0", ClampMax = "3.14"))
	float GlowLeadAngle = 0.35f;

	// How much segments shrink toward the tail (0 = uniform, 1 = tail vanishes).
	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TaperAmount = 0.55f;

	// Scale multiplier for the lead (head) node — makes it visibly larger than the body.
	UPROPERTY(EditAnywhere, Category = "GravityEntity|Rendering", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float HeadScale = 1.4f;

	// Per-entity solver instance (duplicated from profile at init — independent runtime state).
	UPROPERTY(VisibleAnywhere, Category = "GravityEntity|State")
	TObjectPtr<UGravityMovementSolver> SolverInstance;

	// Per-entity breath instance (duplicated from profile at init).
	UPROPERTY(VisibleAnywhere, Category = "GravityEntity|State")
	TObjectPtr<UGravityBreathSignal> BreathInstance;

	UFUNCTION(BlueprintCallable, Category = "GravityEntity")
	void SetAttentionTarget(FVector WorldTarget);

	void ReinitializeEntity();
	void RebuildLogicOnly();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> NodeMIDs;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> LinkMIDs;

	void InitializeEntity();
	void DestroyMeshComponents();
	void CreateNodeMeshComponents();
	void CreateLinkMeshComponents();
	void ComputeDisplayPositions();
	void UpdateNodeMeshes();
	void DebugDraw() const;
};
