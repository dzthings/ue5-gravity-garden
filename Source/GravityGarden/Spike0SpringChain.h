#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spike0SpringChain.generated.h"

// Throwaway Spike 0 actor — delete after the motion gate visual is confirmed.
// Spring-chain of mass nodes connected by spring-dampers; debug-drawn each frame.
UCLASS()
class GRAVITYGARDEN_API ASpike0SpringChain : public AActor
{
	GENERATED_BODY()

public:
	ASpike0SpringChain();

	virtual void Tick(float DeltaTime) override;

	// Number of nodes in the chain
	UPROPERTY(EditAnywhere, Category = "Spike0", meta = (ClampMin = "2", ClampMax = "32"))
	int32 NodeCount = 8;

	// Rest spacing between consecutive nodes (cm)
	UPROPERTY(EditAnywhere, Category = "Spike0", meta = (ClampMin = "10.0", ClampMax = "500.0"))
	float RestSpacing = 80.f;

	// Spring stiffness (force per unit stretch)
	UPROPERTY(EditAnywhere, Category = "Spike0", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float Stiffness = 400.f;

	// Spring damping coefficient
	UPROPERTY(EditAnywhere, Category = "Spike0", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float Damping = 20.f;

	// Debug sphere radius (cm)
	UPROPERTY(EditAnywhere, Category = "Spike0|Debug", meta = (ClampMin = "1.0"))
	float SphereRadius = 16.f;

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	struct FNode
	{
		FVector Position  = FVector::ZeroVector;
		FVector Velocity  = FVector::ZeroVector;
		bool    bAnchored = false; // first node is pinned
	};

	TArray<FNode> Nodes;

	void InitChain();
	void StepSimulation(float Dt);
};
