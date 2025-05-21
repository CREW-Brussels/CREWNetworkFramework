#pragma once

#include "CoreMinimal.h"
#include "BoneContainer.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_Replicate.generated.h"

USTRUCT(BlueprintType)
struct CREWNETWORK_API FAnimNode_Replicate : public FAnimNode_Base
{
    GENERATED_BODY()

public:
    FAnimNode_Replicate();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Links")
    FPoseLink BasePose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Replication", meta = (PinShownByDefault))
    FName StreamName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Replication", meta = (PinShownByDefault))
    bool Streaming = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Replication", meta = (PinShownByDefault))
    float FPS = 0.f;

    virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
    virtual void Evaluate_AnyThread(FPoseContext& Output) override;
    virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
    virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
private:
    UPROPERTY()
    class UCREWNetworkSubsystem *network;
};