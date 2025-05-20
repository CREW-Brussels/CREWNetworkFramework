#pragma once



#include "CoreMinimal.h"
#include "UObject/Object.h"
#if WITH_LIVE_LINK
#include "LiveLinkProvider.h"
#include "Roles/LiveLinkAnimationRole.h"
#endif
#include "CustomLiveLinkSource.generated.h"

UCLASS()
class UCustomLiveLinkSource : public UObject
{
    GENERATED_BODY()
#if WITH_LIVE_LINK
public:
    void Initialize(FName SubjectName);
    void UpdateStaticData(USkeletalMesh* mesh);
    void PushSkeletalFrame(const TArray<FTransform>& BoneTransforms);

    static void InitLiveLink();

    static TSharedPtr<ILiveLinkProvider> LiveLinkProvider;
    FLiveLinkSubjectKey SubjectKey;
    FGuid SourceGuid;
    bool bSkeletonPushed = false;
#endif
};
