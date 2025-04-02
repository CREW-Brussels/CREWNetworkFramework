#include "AnimNode_Replicate.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"
#include "CREWNetworkSubsystem.h"

FAnimNode_Replicate::FAnimNode_Replicate() : network(nullptr) {

}

void FAnimNode_Replicate::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
    network = nullptr;
    if (Context.AnimInstanceProxy)
    {
        if (const UAnimInstance* AnimInstance = Cast<UAnimInstance>(Context.AnimInstanceProxy->GetAnimInstanceObject()))
        {
            AActor *Owner = AnimInstance->GetOwningComponent()->GetOwner();
            if (Owner && IsValid(Owner->GetGameInstance()))
            {
                network = Owner->GetGameInstance()->GetSubsystem<UCREWNetworkSubsystem>();
            }
            else
            {
                return;
            }
        }
    }

}

void FAnimNode_Replicate::Evaluate_AnyThread(FPoseContext& Output)
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
