#include "CREWNetworkSubsystem.h"

#include "Kismet/GameplayStatics.h"

UCREWNetworkSubsystem::UCREWNetworkSubsystem() {
	Socket = nullptr;
	IsServer = false;
	UDPReceiver = nullptr;
	bufferPose.SetNumUninitialized(60);
}

void UCREWNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	MulticastAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	MulticastAddr->SetIp(FIPv4Address(10, 0, 2, 126).Value);
	MulticastAddr->SetPort(16502);
	Socket = FUdpSocketBuilder(TEXT("UDPBroadcaster"))
		.AsReusable()
		.AsNonBlocking()
		.WithBroadcast()
		.BoundToEndpoint(FIPv4Endpoint(FIPv4Address::Any, 16502))
		.WithSendBufferSize(0);


	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	FString ThreadName = FString::Printf(TEXT("UDP PoseReceiver"));
	UDPReceiver = new FUdpSocketReceiver(Socket, ThreadWaitTime, *ThreadName);

	UDPReceiver->OnDataReceived().BindLambda([this](const FArrayReaderPtr& DataPtr, const FIPv4Endpoint& Endpoint) {
			FScopeLock Lock(&NetworkCriticalSection);
			FName name;
			TArray<FTransform> data;

			FMemoryReader MemoryReader(*DataPtr);

			MemoryReader << name;

			int32 index;
			int16 num, offset;
			MemoryReader << index;
			MemoryReader << num;
			MemoryReader << offset;

			TransformArraySerializer::DeserializeCompressedTransforms(MemoryReader, bufferPose);

			FReplicatedPosePlayHead* found = NamedPoseStreams.Find(name);

			if (found == nullptr) {
				found = &NamedPoseStreams.Add(name, FReplicatedPosePlayHead());
			}
			found->AddFragment(bufferPose, num, offset, index, PrecisionTime());
			//found->AddFrame(data, PrecisionTime());
		});

	UDPReceiver->Start();
}

void UCREWNetworkSubsystem::PushReplicatedPose(FName name, FPoseContext& Pose, float fps) {
	FScopeLock Lock(&NetworkCriticalSection);
	FReplicatedPosePlayHead* found = NamedPoseStreams.Find(name);

	if (found == nullptr) {
		found = &NamedPoseStreams.Add(name, FReplicatedPosePlayHead());
	}
	if (fps >= 1.f) {
		double delta = double(1.f / fps);
		double time = PrecisionTime();
		if (time > found->limiter + delta) {
			found->limiter = time;
		}
		else {
			return;
		}
	}
	NetworkBuffer.Empty();
	tempPose = Pose.Pose.MoveBones();
	int16 num = tempPose.Num();
	int32 byteSent;
	for (int16 i = 0; i < num; i += 60) {
		int16 r = num - i;
		if (r > 60) {
			r = 60;
		}
		bufferPose.SetNumUninitialized(r);
		for (int16 j = 0; j < r; j++) {
			bufferPose[j] = tempPose[i + j];
		}
		FMemoryWriter MemoryWriter(NetworkBuffer, true);
		MemoryWriter << name;
		MemoryWriter << found->send_index;
		MemoryWriter << num;
		MemoryWriter << i;
		TransformArraySerializer::SerializeCompressedTransforms(MemoryWriter, bufferPose);
		for (int j = 0; j < 4; j++) {
			Socket->SendTo(NetworkBuffer.GetData(), NetworkBuffer.Num(), byteSent, *MulticastAddr);
		}
	}
	found->send_index++;
}

bool UCREWNetworkSubsystem::GetReplicatedPose(FName name, FPoseContext& Pose) {
	FReplicatedPosePlayHead* found = NamedPoseStreams.Find(name);
	if (found != nullptr) {
		return found->GetFrame(Pose, PrecisionTime());
	}
	else {
		return false;
	}
}

double UCREWNetworkSubsystem::PrecisionTime() {
	return FPlatformTime::ToSeconds64(FPlatformTime::Cycles64());
}

void UCREWNetworkSubsystem::Deinitialize() {
	if (UDPReceiver) {
		UDPReceiver->Stop();
		delete UDPReceiver;
		UDPReceiver = nullptr;
	}
	if (Socket) {
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}
}
