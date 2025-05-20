

#include "CustomLiveLinkSource.h"
#if WITH_LIVE_LINK
//#include "ILiveLinkModule.h"
#include "ILiveLinkClient.h"
#include "LiveLinkProvider.h"
#include "LiveLinkTypes.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "LiveLinkSettings.h"
#include "LiveLinkSubjectSettings.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "InterpolationProcessor/LiveLinkAnimationFrameInterpolateProcessor.h"
#include "Modules/ModuleManager.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"

TSharedPtr<ILiveLinkProvider> UCustomLiveLinkSource::LiveLinkProvider;


void UCustomLiveLinkSource::InitLiveLink()
{
    FString provName = "Crew";
    LiveLinkProvider = ILiveLinkProvider::CreateLiveLinkProvider(provName);
}

void UCustomLiveLinkSource::Initialize(FName SubjectName)
{
    

    SubjectKey.Source = FGuid::NewGuid();
    SubjectKey.SubjectName = SubjectName;
}

void UCustomLiveLinkSource::UpdateStaticData(USkeletalMesh* SkeletalMesh)
{
    TArray<FName> BoneNames;
    TArray<int32> BoneParents;

    TArray<int32> BoneReindex;

    TMap<int32, int32> BoneIndexToSubsetIndex;

    const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();

    BoneReindex.Reserve(RefSkeleton.GetNum());
    for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
    {
        BoneReindex.Add(i);
    }

    const FSkeletalMeshLODRenderData& LODRenderData = SkeletalMesh->GetResourceForRendering()->LODRenderData[0];
    const TArray<FBoneIndexType>& RequiredBones = LODRenderData.RequiredBones;

    // Step 1: Map bone index → index in our subset
    for (int32 i = 0; i < RequiredBones.Num(); i++)
    {
        BoneReindex[RequiredBones[i]] = i;
    }

    // Step 2: Fill BoneNames and remapped BoneParents
    for (int32 i = 0; i < RequiredBones.Num(); i++)
    {
        int32 BoneIndex = RequiredBones[i];
        FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
        int32 ParentBoneIndex = RefSkeleton.GetParentIndex(BoneIndex);

        BoneNames.Add(BoneName);
        BoneParents.Add(ParentBoneIndex >= 0 ? BoneReindex[ParentBoneIndex] : -1);
    }

    FLiveLinkStaticDataStruct StaticDataStruct(FLiveLinkSkeletonStaticData::StaticStruct());
    FLiveLinkSkeletonStaticData& StaticData = *StaticDataStruct.Cast<FLiveLinkSkeletonStaticData>();

    StaticData.BoneNames = BoneNames;
    StaticData.BoneParents = BoneParents;

    if (!LiveLinkProvider->UpdateSubjectStaticData(SubjectKey.SubjectName, ULiveLinkAnimationRole::StaticClass(), MoveTemp(StaticDataStruct))) {
        UE_LOG(LogTemp, Warning, TEXT("Update Static error"));
    }
    bSkeletonPushed = true;


    IModularFeatures& ModularFeatures = IModularFeatures::Get();
    if (ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
    {
        ILiveLinkClient& LiveLinkClient = ModularFeatures.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
        ULiveLinkSubjectSettings* Settings = Cast<ULiveLinkSubjectSettings>(LiveLinkClient.GetSubjectSettings(SubjectKey));

        if (Settings) {
            Settings->InterpolationProcessor = NewObject<ULiveLinkAnimationFrameInterpolationProcessor>();
        }
    }
    /*ILiveLinkClient* LiveLinkClient = FModuleManager::LoadModuleChecked<FLiveLinkModule>("LiveLink").GetClient();

    if (LiveLinkClient)
    {
        // Get or create settings
        ULiveLinkSubjectSettings* SubjectSettings = LiveLinkClient->GetSubjectSettings(SubjectKey);
        if (SubjectSettings)
        {
            // Set interpolation type
            SubjectSettings->InterpolationSettings.InterpolationMode = ELiveLinkInterpolationMode::Interpolated; // Or .None, .Latest
            SubjectSettings->InterpolationSettings.InterpolationOffset = 0.05f; // Time offset in seconds
        }
    }*/

    /*ILiveLinkClient* Client = ILiveLinkModule::Get().GetClient();
    if (!Client) return;

    FLiveLinkSubjectSettings* Settings = Client->GetSubjectSettings(SubjectKey);
    if (Settings)
    {
        Settings->Mode = ELiveLinkSourceMode::Animation;
        UE_LOG(LogTemp, Log, TEXT("Live Link interpolation set to Animation for subject: %s"), *SubjectName.ToString());
    }*/
}

void UCustomLiveLinkSource::PushSkeletalFrame(const TArray<FTransform>& BoneTransforms)
{
    if (!LiveLinkProvider.IsValid()) return;

    if (!bSkeletonPushed)
    {
        TArray<FName> BoneNames;
        TArray<int32> BoneParents;
        for (int i = 0; i < BoneTransforms.Num(); i++) {
            BoneNames.Add(FName(*FString::Printf(TEXT("Bone%d"), i)));
            BoneParents.Add(i > 0 ? 0 : -1);
        }
        // Push Static Skeleton
        FLiveLinkStaticDataStruct StaticDataStruct(FLiveLinkSkeletonStaticData::StaticStruct());
        FLiveLinkSkeletonStaticData& StaticData = *StaticDataStruct.Cast<FLiveLinkSkeletonStaticData>();

        StaticData.BoneNames = BoneNames;
        StaticData.BoneParents = BoneParents;

        LiveLinkProvider->UpdateSubjectStaticData(SubjectKey.SubjectName, ULiveLinkAnimationRole::StaticClass(), MoveTemp(StaticDataStruct));
        bSkeletonPushed = true;
    }

    // Push Bone Transforms
    FLiveLinkFrameDataStruct FrameDataStruct(FLiveLinkAnimationFrameData::StaticStruct());
    FLiveLinkAnimationFrameData& FrameData = *FrameDataStruct.Cast<FLiveLinkAnimationFrameData>();

    FrameData.Transforms = BoneTransforms;
    FrameData.WorldTime = FLiveLinkWorldTime(FPlatformTime::Seconds(), 1.0);

    if (!LiveLinkProvider->UpdateSubjectFrameData(SubjectKey.SubjectName, MoveTemp(FrameDataStruct))) {
        UE_LOG(LogTemp, Warning, TEXT("Update error"));
    }
    
}

#endif