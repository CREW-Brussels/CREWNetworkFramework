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
#include "ReplicatedPosePlayHead.h"

#include "CREWNetworkSubsystem.generated.h"

UCLASS()
class CREWNETWORK_API UCREWNetworkSubsystem : public UGameInstanceSubsystem//UEngineSubsystem
{
	GENERATED_BODY()

public:
	UCREWNetworkSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	friend FAnimNode_Replicate;
	void PushReplicatedPose(FName name, FPoseContext& Pose, float fps);
	bool GetReplicatedPose(FName name, FPoseContext& Pose);

	static inline double PrecisionTime();
	
	UPROPERTY()
	TMap<FName, FReplicatedPosePlayHead> NamedPoseStreams;
	TArray<FTransform> tempPose;
	TArray<FTransform> bufferPose;

	FSocket* Socket;
	FUdpSocketReceiver* UDPReceiver;
	bool IsServer;

	FCriticalSection NetworkCriticalSection;
	TSharedPtr<FInternetAddr> MulticastAddr;
	TArray<uint8> NetworkBuffer;


	FSocket* UDPSocket;
	FSocket* TCPListenerSocket;
	TMap<FString, FSocket*> ConnectedPeers;

	/*FTimerHandle BroadcastTimerHandle;
	FTimerHandle AcceptTimerHandle;
	FTimerHandle ListenTimerHandle;
	FTimerHandle CheckDataTimerHandle;*/

	FTSTicker::FDelegateHandle BroadcastTickerHandle;
	FTSTicker::FDelegateHandle ListenTickerHandle;
	FTSTicker::FDelegateHandle AcceptTickerHandle;
	FTSTicker::FDelegateHandle CheckDataTickerHandle;

	const int32 UDPPort = 5000;
	const int32 TCPPort = 6000;
	FString LocalIP;

protected:
	void StopNetworking();
	void StartNetworking();
	bool BroadcastPresence(float DeltaTime);
	bool ListenForBroadcasts(float DeltaTime);
	void AttemptConnection(const FString& PeerIP, int32 PeerPort);
	bool AcceptIncomingConnections(float DeltaTime);
	bool CheckForIncomingData(float DeltaTime);
};

