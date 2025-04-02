#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNodeUtils.h"
#include "AnimGraphNode_Base.h"
#include "AnimNode_Replicate.h"
#include "AnimGraphNode_Replicate.generated.h"

UCLASS()
class CREWNETWORKEDITOR_API UAnimGraphNode_Replicate : public UAnimGraphNode_Base
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Settings")
    FAnimNode_Replicate Node;

    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override
    {
        return FText::FromString(TEXT("Replicate Node"));
    }

    virtual FText GetTooltipText() const override
    {
        return FText::FromString(TEXT("Replicate the pose to other instances on the same local network."));
    }

    virtual FString GetNodeCategory() const override
    {
        return TEXT("Replication");
    }

    // Ensure array inputs appear as pins
    virtual void CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const override
    {
        if (SourcePropertyName == GET_MEMBER_NAME_CHECKED(FAnimNode_Replicate, StreamName))
        {
            Pin->bHidden = false;
        }
        if (SourcePropertyName == GET_MEMBER_NAME_CHECKED(FAnimNode_Replicate, Streaming))
        {
            Pin->bHidden = false;
        }
        if (SourcePropertyName == GET_MEMBER_NAME_CHECKED(FAnimNode_Replicate, FPS))
        {
            Pin->bHidden = false;
        }
    }
};