#pragma once

#include "CoreMinimal.h"
#include "BoneContainer.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_AutoReplicate.generated.h"

USTRUCT(BlueprintType)
struct CREWNETWORK_API FAnimNode_AutoReplicate : public FAnimNode_Base
{
    GENERATED_BODY()

public:
    FAnimNode_AutoReplicate();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Links")
    FPoseLink BasePose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Replication", meta = (PinShownByDefault))
    float FPS = 0.f;

    virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
    virtual void Evaluate_AnyThread(FPoseContext& Output) override;
private:
    UPROPERTY()
    class UCREWNetworkSubsystem *network;

    bool Streaming;
    FName StreamName;
};