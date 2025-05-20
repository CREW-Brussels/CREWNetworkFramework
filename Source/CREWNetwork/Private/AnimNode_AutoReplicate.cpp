#include "AnimNode_AutoReplicate.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"
#include "CREWNetworkSubsystem.h"

FAnimNode_AutoReplicate::FAnimNode_AutoReplicate() : network(nullptr) {

}

void FAnimNode_AutoReplicate::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
    network = nullptr;
    if (Context.AnimInstanceProxy)
    {
        if (const UAnimInstance* AnimInstance = Cast<UAnimInstance>(Context.AnimInstanceProxy->GetAnimInstanceObject()))
        {
            AActor *Owner = AnimInstance->GetOwningComponent()->GetOwner();
            if (Owner && IsValid(Owner->GetGameInstance()))
            {
                APawn* pawn = Cast<APawn>(Owner);
                if (pawn != nullptr) {
                    network = GEngine->GetEngineSubsystem<UCREWNetworkSubsystem>();//Owner->GetGameInstance()->GetSubsystem<UCREWNetworkSubsystem>();
                    Streaming = pawn->IsLocallyControlled();
                    StreamName = pawn->GetFName();
                }
            }
        }
    }

}

void FAnimNode_AutoReplicate::Evaluate_AnyThread(FPoseContext& Output)
{
    if (network != nullptr) {
        if (Streaming) {
            BasePose.Evaluate(Output);
            network->PushReplicatedPose(StreamName, Output, FPS);
        }
        else {
            if (!network->GetReplicatedPose(StreamName, Output)) {
                BasePose.Evaluate(Output);
            }
        }
    }
}
