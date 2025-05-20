// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "IPAddress.h"
#include "Common/UdpSocketBuilder.h"
#include "Common/UdpSocketReceiver.h"
#include "Common/UdpSocketSender.h"
#include "Networking.h"
#include "Animation/AnimNodeBase.h"
#include "BoneIndices.h"
#include "AnimNode_Replicate.h"
#include "AnimNode_AutoReplicate.h"
#include "ReplicatedPosePlayHead.h"
#include "Command.h"

#include "CREWNetworkSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNetworkCommandEvent, FGameplayTag, Command);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNetworkCommandWithBoolEvent, FGameplayTag, Command, bool, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNetworkCommandWithIntEvent, FGameplayTag, Command, int, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNetworkCommandWithFloatEvent, FGameplayTag, Command, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNetworkCommandWithVectorEvent, FGameplayTag, Command, FVector, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNetworkCommandWithRotatorEvent, FGameplayTag, Command, FRotator, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNetworkCommandWithTransformEvent, FGameplayTag, Command, FTransform, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNetworkCommandWithStringEvent, FGameplayTag, Command, FString, Value);

enum class ECREWNetworkType : uint8 {
	Invalid,
	Pose,
	Command,
	Ack
};

UCLASS()
class CREWNETWORK_API UCREWNetworkSubsystem : public UEngineSubsystem//UGameInstanceSubsystem//UGameInstanceSubsystem//UEngineSubsystem
{
	GENERATED_BODY()

public:
	UCREWNetworkSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	void SetSkeletonForStream(USkeletalMesh *skeleton, FName stream);

	UPROPERTY(BlueprintAssignable, Category = "Network Command")
	FOnNetworkCommandEvent OnNetworkCommand;

	UPROPERTY(BlueprintAssignable, Category = "Network Command")
	FOnNetworkCommandWithBoolEvent OnNetworkCommandWithBool;
	UPROPERTY(BlueprintAssignable, Category = "Network Command")
	FOnNetworkCommandWithIntEvent OnNetworkCommandWithInt;
	UPROPERTY(BlueprintAssignable, Category = "Network Command")
	FOnNetworkCommandWithFloatEvent OnNetworkCommandWithFloat;
	UPROPERTY(BlueprintAssignable, Category = "Network Command")
	FOnNetworkCommandWithVectorEvent OnNetworkCommandWithVector;
	UPROPERTY(BlueprintAssignable, Category = "Network Command")
	FOnNetworkCommandWithRotatorEvent OnNetworkCommandWithRotator;
	UPROPERTY(BlueprintAssignable, Category = "Network Command")
	FOnNetworkCommandWithTransformEvent OnNetworkCommandWithTransform;
	UPROPERTY(BlueprintAssignable, Category = "Network Command")
	FOnNetworkCommandWithStringEvent OnNetworkCommandWithString;

	UFUNCTION(BlueprintCallable, Category = "Network Command")
	void SendCommand(FGameplayTag command);

	UFUNCTION(BlueprintCallable, Category = "Network Command")
	void SendCommandWithBool(FGameplayTag command, bool value);
	UFUNCTION(BlueprintCallable, Category = "Network Command")
	void SendCommandWithInt(FGameplayTag command, int value);
	UFUNCTION(BlueprintCallable, Category = "Network Command")
	void SendCommandWithFloat(FGameplayTag command, float value);
	UFUNCTION(BlueprintCallable, Category = "Network Command")
	void SendCommandWithVector(FGameplayTag command, FVector value);
	UFUNCTION(BlueprintCallable, Category = "Network Command")
	void SendCommandWithRotator(FGameplayTag command, FRotator value);
	UFUNCTION(BlueprintCallable, Category = "Network Command")
	void SendCommandWithTransform(FGameplayTag command, FTransform value);
	UFUNCTION(BlueprintCallable, Category = "Network Command")
	void SendCommandWithString(FGameplayTag command, FString value);

private:
	friend FAnimNode_Replicate;
	friend FAnimNode_AutoReplicate;
	bool BroadcastPresence(float DeltaTime);

	void PushReplicatedPose(FName name, FPoseContext& Pose, float fps);
	bool GetReplicatedPose(FName name, FPoseContext& Pose);

	void SendRemainingCommands();

	static inline double PrecisionTime();

	void ExecOnGameThread(const FString& command);
	
	UPROPERTY()
	TMap<FName, FReplicatedPosePlayHead> NamedPoseStreams;

	TArray<FTransform> tempPose;
	TArray<FTransform> bufferPose;

	FSocket* BroadcastSocket;
	FUdpSocketReceiver* BroadcastReceiver;
	FThreadSafeBool keepBroadcastTimer;
	TFuture<void> broadcastThread;

	FCriticalSection NetworkCriticalSection;
	TSharedPtr<FInternetAddr> MulticastAddr;
	TArray<uint8> NetworkBuffer;

	FSocket* UdpSocket;
	FUdpSocketReceiver* UdpReceiver;
	
	TSet<FIPv4Endpoint> peers;
	TMap<FIPv4Endpoint, int> peersTimeout;

	TDoubleLinkedList<FCommand> commandsOut;
	TMap<FIPv4Endpoint, TSet<int>> acks;
	TArray<uint8> commandBuffer;
	FThreadSafeBool keepRunningCommand;
	TFuture<void> commandThread;

	TSharedPtr<FInternetAddr> localBroadcastAddr;
	TSharedPtr<FInternetAddr> localCommAddr;
	FTSTicker::FDelegateHandle BroadcastTickerHandle;
	bool clearingCommands;

	//FName ProjectName;
	bool isPlaying;
	bool isServer;
	FString connectionString;
	uint32 connectionInt;

	int broadcastPort;
	int communicationPort;
	FName configAppName;
	bool bEnableAutoConnect;
};

