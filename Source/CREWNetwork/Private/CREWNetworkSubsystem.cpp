#include "CREWNetworkSubsystem.h"
#include "SocketSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "CREWNetworkSettings.h"

UCREWNetworkSubsystem::UCREWNetworkSubsystem() {
	BroadcastSocket = nullptr;
	BroadcastReceiver = nullptr;
	UdpSocket = nullptr;
	UdpReceiver = nullptr;
	keepRunningCommand = false;
	bufferPose.SetNumUninitialized(60);
	connectionInt = 0;
}

void UCREWNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
#if WITH_LIVE_LINK
	UCustomLiveLinkSource::InitLiveLink();
#endif
	//ProjectName = FName(FApp::GetProjectName());
	isPlaying = false;
	isServer = false;

	const UCREWNetworkSettings* settings = GetDefault<UCREWNetworkSettings>();

	broadcastPort = settings->BroadcastPort;
	if (broadcastPort < 0 || broadcastPort > 65535)
		broadcastPort = 16502;

	communicationPort = settings->CommunicationPort;
	if (communicationPort < 0 || communicationPort > 65535)
		communicationPort = 16503;

	configAppName = settings->AppName;
	bEnableAutoConnect = settings->bEnableAutoConnect;

	bool valid;
	localBroadcastAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, valid);
	localBroadcastAddr->SetPort(broadcastPort);
	localCommAddr = localBroadcastAddr;
	localCommAddr->SetPort(communicationPort);

	MulticastAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	MulticastAddr->SetIp(FIPv4Address(255, 255, 255, 255).Value);
	MulticastAddr->SetPort(broadcastPort);
	BroadcastSocket = FUdpSocketBuilder(TEXT("CREW UDP Broadcast"))
		.AsReusable()
		.AsNonBlocking()
		.WithBroadcast()
		.BoundToEndpoint(FIPv4Endpoint(FIPv4Address::Any, broadcastPort))
		.WithSendBufferSize(0);


	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(500);
	FString ThreadName = FString::Printf(TEXT("CREW UDP Broadcast Receiver"));
	BroadcastReceiver = new FUdpSocketReceiver(BroadcastSocket, ThreadWaitTime, *ThreadName);

	BroadcastReceiver->OnDataReceived().BindLambda([this](const FArrayReaderPtr& DataPtr, const FIPv4Endpoint& Endpoint) {
			FScopeLock Lock(&NetworkCriticalSection);

			FMemoryReader MemoryReader(*DataPtr);
			FName otherName;

			MemoryReader << otherName;
			if (otherName == configAppName) { // Only allow instances of the same project to acknlowledge each other presence
				//FIPv4Address other = Endpoint.Address;
				FIPv4Endpoint other = Endpoint;
				other.Port = communicationPort;
				//TSharedRef<FInternetAddr> other = Endpoint.ToInternetAddr();
				//other->SetPort(communicationPort);
				int* timeout = peersTimeout.Find(other);
				if (timeout == nullptr) {
					peers.Add(other);
					peersTimeout.Add(other, 30);//30 sec timeout
					acks.Add(other);
				}
				else {
					*timeout = 30;//reset timeout
				}
				if (bEnableAutoConnect) {
					bool server;
					MemoryReader << server;
					if (server && !isServer) {
						uint32 token;
						FString command;
						MemoryReader << token;
						MemoryReader << command;
						if (token != connectionInt) {
							//unknown connection string, we attempt to execute it to connect to the server
							connectionInt = token;
							connectionString = command;
							ExecOnGameThread(connectionString);
							/*
							for (const FWorldContext& context : GEngine->GetWorldContexts()) {
								UWorld* world = context.World();
								if (world && !world->IsEditorWorld()) {
									connectionString = command;
									world->Exec(world, *connectionString);
									return;
								}
							}*/
						}
					}
				}
			}
		});
	BroadcastReceiver->Start();

	UdpSocket = FUdpSocketBuilder(TEXT("CREW UDP Data"))
		.AsReusable()
		.AsNonBlocking()
		.BoundToEndpoint(FIPv4Endpoint(FIPv4Address::Any, communicationPort))
		.WithSendBufferSize(0);

	ThreadWaitTime = FTimespan::FromMilliseconds(10);
	ThreadName = FString::Printf(TEXT("CREW UDP Receiver"));
	UdpReceiver = new FUdpSocketReceiver(UdpSocket, ThreadWaitTime, *ThreadName);

	UdpReceiver->OnDataReceived().BindLambda([this](const FArrayReaderPtr& DataPtr, const FIPv4Endpoint& Endpoint) {
		FScopeLock Lock(&NetworkCriticalSection);

		FIPv4Endpoint other = Endpoint;

		FMemoryReader MemoryReader(*DataPtr);

		ECREWNetworkType type;
		MemoryReader << type;
		
		switch (type) {
		case ECREWNetworkType::Pose: //Pose
		{
			FName name;
			TArray<FTransform> data;
			MemoryReader << name;

			int32 index;
			int16 num, offset;
			MemoryReader << index;
			MemoryReader << num;
			MemoryReader << offset;
			double t;
			MemoryReader << t;

			TransformArraySerializer::DeserializeCompressedTransforms(MemoryReader, bufferPose);

			FReplicatedPosePlayHead* found = NamedPoseStreams.Find(name);

			if (found == nullptr) {
				found = &NamedPoseStreams.Add(name, FReplicatedPosePlayHead());
				found->Init(name);
			}
			found->AddFragment(bufferPose, num, offset, index, t);
			break;
		}
		case ECREWNetworkType::Command: //Commands
		{
			FCommand command;
			MemoryReader << command;

			auto ack = acks.Find(other);

			if (ack != nullptr) {
				if (!ack->Contains(command.index)) {
					ack->Add(command.index);
					switch (command.type) {
					case ECommandType::NoValue:
						OnNetworkCommand.Broadcast(command.id);
						break;
					case ECommandType::Bool:
						OnNetworkCommandWithBool.Broadcast(command.id, command.value_bool);
						break;
					case ECommandType::Int:
						OnNetworkCommandWithInt.Broadcast(command.id, command.value_int);
						break;
					case ECommandType::Float:
						OnNetworkCommandWithFloat.Broadcast(command.id, command.value_float);
						break;
					case ECommandType::Vector:
						OnNetworkCommandWithVector.Broadcast(command.id, command.value_vector);
						break;
					case ECommandType::Rotator:
						OnNetworkCommandWithRotator.Broadcast(command.id, command.value_rotator);
						break;
					case ECommandType::Transform:
						OnNetworkCommandWithTransform.Broadcast(command.id, command.value_transform);
						break;
					case ECommandType::String:
						OnNetworkCommandWithString.Broadcast(command.id, command.value_string);
						break;
					default:
						UE_LOG(LogTemp, Error, TEXT("Invalid type in a command"));
						break;
					}
				}
				//allways ack even if we already acked before
				int32 byteSent;
				uint32 num = command.index;
				NetworkBuffer.Empty();
				FMemoryWriter MemoryWriter(NetworkBuffer, true);
				ECREWNetworkType typeOut = ECREWNetworkType::Ack;
				MemoryWriter << typeOut;
				MemoryWriter << num;
				UdpSocket->SendTo(NetworkBuffer.GetData(), NetworkBuffer.Num(), byteSent, *Endpoint.ToInternetAddr());
			}
			break;
		}
		case ECREWNetworkType::Ack: //Commands Ack
		{
			FString ip;
			int32 index;
			MemoryReader << index;

			auto pointer = commandsOut.GetHead();
			while (pointer != nullptr) {
				auto next = pointer->GetNextNode();
				FCommand& command = pointer->GetValue();
				if (command.index == index && command.remaining.Contains(other)) {
					command.remaining.Remove(other);
					if (command.remaining.IsEmpty()) {
						commandsOut.RemoveNode(pointer);
					}
				}
				pointer = next;
			}

			break;
		}
		default:
			UE_LOG(LogTemp, Error, TEXT("Invalid type in a CREW Network packet"));
			break;
		}
		});
	UdpReceiver->Start();

	BroadcastTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UCREWNetworkSubsystem::BroadcastPresence), 1.0f);

	keepRunningCommand = true;
	commandThread = Async(EAsyncExecution::Thread, [this]() {
		while (keepRunningCommand) {
			{
				FScopeLock Lock(&NetworkCriticalSection);
				SendRemainingCommands();
			}
			FPlatformProcess::Sleep(0.01f);
		}
		});
	keepBroadcastTimer = true;
	broadcastThread = Async(EAsyncExecution::Thread, [this]() {
		while (keepBroadcastTimer) {
			{
				FScopeLock Lock(&NetworkCriticalSection);
				TArray<FIPv4Endpoint> timedOut;
				for (auto p : peersTimeout) {
					p.Value -= 1;
					if (p.Value <= 0) {
						timedOut.Add(p.Key);
					}
				}
				for (auto p : timedOut) {
					peersTimeout.Remove(p);
					peers.Remove(p);

					auto pointer = commandsOut.GetHead();
					while (pointer != nullptr) {
						auto next = pointer->GetNextNode();
						FCommand& command = pointer->GetValue();
						if (command.remaining.Contains(p)) {
							command.remaining.Remove(p);
							if (command.remaining.IsEmpty()) {
								commandsOut.RemoveNode(pointer);
							}
						}
						pointer = next;
					}
					acks.Remove(p);
				}
				SendRemainingCommands();
			}
			FPlatformProcess::Sleep(1.f);
		}
		});
}

bool UCREWNetworkSubsystem::BroadcastPresence(float DeltaTime) {
	if (localBroadcastAddr->IsValid() && BroadcastSocket != nullptr) {
		FScopeLock Lock(&NetworkCriticalSection);
		isPlaying = false;
		bool server = false;
		if (bEnableAutoConnect) {
			//Finding play scenario
			for (const FWorldContext& context : GEngine->GetWorldContexts()) {
				UWorld* world = context.World();
				if (world) {
					ENetMode mode = world->GetNetMode();
					isPlaying = true;
					if (mode == NM_DedicatedServer || mode == NM_ListenServer) {
						server = true;
						if (!isServer) {
							//generate connection string
							bool bCanBindAll = false;
							TSharedPtr<FInternetAddr> LocalAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);
							if (LocalAddr.IsValid())
							{
								isServer = true;

								FString ip = LocalAddr->ToString(false);
								int32 port = 17777;//world->URL.Port;
								FString mapPath = world->PersistentLevel->GetOutermost()->GetName().Replace(TEXT("UEDPIE_0_"), TEXT(""));

								connectionString = FString::Printf(TEXT("open %s:%d?game=%s"), *ip, port, *mapPath);
								uint32 newToken = 0;
								do {
									newToken = FMath::Rand();
								} while (newToken == connectionInt);
								connectionInt = newToken;
							}
						}

					}
				}
			}
		}
		if (!server) {
			isServer = false;
		}
		NetworkBuffer.Empty();
		FMemoryWriter MemoryWriter(NetworkBuffer, true);
		MemoryWriter << configAppName;
		MemoryWriter << isServer;
		if (isServer) {
			MemoryWriter << connectionInt;
			MemoryWriter << connectionString;
		}
		int32 bytesSent = 0;
		BroadcastSocket->SendTo(NetworkBuffer.GetData(), NetworkBuffer.Num(), bytesSent, *MulticastAddr);
	}
	return true;
}

void UCREWNetworkSubsystem::SendCommand(FGameplayTag command) {
	FScopeLock Lock(&NetworkCriticalSection);
	commandsOut.AddTail(FCommand(command, peers));
	SendRemainingCommands();
}

void UCREWNetworkSubsystem::SendCommandWithBool(FGameplayTag command, bool value) {
	FScopeLock Lock(&NetworkCriticalSection);
	commandsOut.AddTail(FCommand(command, value, peers));
	SendRemainingCommands();
}

void UCREWNetworkSubsystem::SendCommandWithInt(FGameplayTag command, int value) {
	FScopeLock Lock(&NetworkCriticalSection);
	commandsOut.AddTail(FCommand(command, value, peers));
	SendRemainingCommands();
}

void UCREWNetworkSubsystem::SendCommandWithFloat(FGameplayTag command, float value) {
	FScopeLock Lock(&NetworkCriticalSection);
	commandsOut.AddTail(FCommand(command, value, peers));
	SendRemainingCommands();
}

void UCREWNetworkSubsystem::SendCommandWithVector(FGameplayTag command, FVector value) {
	FScopeLock Lock(&NetworkCriticalSection);
	commandsOut.AddTail(FCommand(command, value, peers));
	SendRemainingCommands();
}

void UCREWNetworkSubsystem::SendCommandWithRotator(FGameplayTag command, FRotator value) {
	FScopeLock Lock(&NetworkCriticalSection);
	commandsOut.AddTail(FCommand(command, value, peers));
	SendRemainingCommands();
}

void UCREWNetworkSubsystem::SendCommandWithTransform(FGameplayTag command, FTransform value) {
	FScopeLock Lock(&NetworkCriticalSection);
	commandsOut.AddTail(FCommand(command, value, peers));
	SendRemainingCommands();
}

void UCREWNetworkSubsystem::SendCommandWithString(FGameplayTag command, FString value) {
	FScopeLock Lock(&NetworkCriticalSection);
	commandsOut.AddTail(FCommand(command, value, peers));
	SendRemainingCommands();
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
	ECREWNetworkType type = ECREWNetworkType::Pose;
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
		MemoryWriter << type;
		MemoryWriter << name;
		MemoryWriter << found->send_index;
		MemoryWriter << num;
		MemoryWriter << i;
        double t = PrecisionTime();
        MemoryWriter << t;
		TransformArraySerializer::SerializeCompressedTransforms(MemoryWriter, bufferPose);
		for (auto p : peers) {
			for (int32 j = 0; j < 4; j++) {
				UdpSocket->SendTo(NetworkBuffer.GetData(), NetworkBuffer.Num(), byteSent, p.ToInternetAddr().Get());
			}
		}
	}
	found->send_index++;
}

bool UCREWNetworkSubsystem::GetReplicatedPose(FName name, FPoseContext& Pose) {
	FScopeLock Lock(&NetworkCriticalSection);
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
	{
		FScopeLock Lock(&NetworkCriticalSection);
		FTSTicker::GetCoreTicker().RemoveTicker(BroadcastTickerHandle);
		if (UdpReceiver) {
			UdpReceiver->Stop();
			delete UdpReceiver;
			UdpReceiver = nullptr;
		}
		if (UdpSocket) {
			UdpSocket->Close();
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(UdpSocket);
			UdpSocket = nullptr;
		}
		if (BroadcastReceiver) {
			BroadcastReceiver->Stop();
			delete BroadcastReceiver;
			BroadcastReceiver = nullptr;
		}
		if (BroadcastSocket) {
			BroadcastSocket->Close();
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(BroadcastSocket);
			BroadcastSocket = nullptr;
		}
		keepRunningCommand = false;
		keepBroadcastTimer = false;
	}
	if (commandThread.IsValid()) {
		commandThread.Wait();
	}
	if (broadcastThread.IsValid()) {
		broadcastThread.Wait();
	}
}

void UCREWNetworkSubsystem::SetSkeletonForStream(USkeletalMesh *skeleton, FName stream)
{
#if WITH_LIVE_LINK
	FReplicatedPosePlayHead* found = NamedPoseStreams.Find(stream);

	if (found != nullptr && found->source != nullptr) {
		found->source->UpdateStaticData(skeleton);
	}
#endif
}


void UCREWNetworkSubsystem::SendRemainingCommands()
{
	//Need to be called from a scoped lock
	int32 byteSent;
	ECREWNetworkType type = ECREWNetworkType::Command;

	for (auto c : commandsOut) {
		commandBuffer.Empty();
		FMemoryWriter MemoryWriter(NetworkBuffer, true);
		MemoryWriter << type;
		MemoryWriter << c;

		for (auto p : c.remaining) {
			UdpSocket->SendTo(NetworkBuffer.GetData(), NetworkBuffer.Num(), byteSent, p.ToInternetAddr().Get());
		}
	}
}

void UCREWNetworkSubsystem::ExecOnGameThread(const FString& command) {
	AsyncTask(ENamedThreads::GameThread, [command]()
	{
		UWorld* target = nullptr;
		for (const FWorldContext& context : GEngine->GetWorldContexts()) {
			if (context.WorldType == EWorldType::Game || context.WorldType == EWorldType::PIE) {
				target = context.World();
				break;
			}
		}
		if (target != nullptr) {
			APlayerController* controller = target->GetFirstPlayerController();
			if (controller != nullptr) {
				controller->ConsoleCommand(command, true);
			}
		}
	});
}
