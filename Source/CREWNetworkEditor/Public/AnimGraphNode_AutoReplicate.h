#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNodeUtils.h"
#include "AnimGraphNode_Base.h"
#include "AnimNode_AutoReplicate.h"
#include "AnimGraphNode_AutoReplicate.generated.h"

UCLASS()
class CREWNETWORKEDITOR_API UAnimGraphNode_AutoReplicate : public UAnimGraphNode_Base
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Settings")
    FAnimNode_AutoReplicate Node;

    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override
    {
        return FText::FromString(TEXT("Auto Replicate Node"));
    }

    virtual FText GetTooltipText() const override
    {
        return FText::FromString(TEXT("Replicate the pose to other instances on the same local network. Automatically replicates from the locally own version of the pawn to the replicated versions of that pawn. Only works on user controller pawns."));
    }

    virtual FString GetNodeCategory() const override
    {
        return TEXT("Replication");
    }

    // Ensure array inputs appear as pins
    virtual void CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const override
    {
        if (SourcePropertyName == GET_MEMBER_NAME_CHECKED(FAnimNode_AutoReplicate, FPS))
        {
            Pin->bHidden = false;
        }
    }
};