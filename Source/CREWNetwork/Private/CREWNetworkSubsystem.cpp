#include "CREWNetworkSubsystem.h"
#include "SocketSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
<<<<<<< Updated upstream
<<<<<<< Updated upstream
=======
#include "CREWNetworkSettings.h"
>>>>>>> Stashed changes
=======
#include "CREWNetworkSettings.h"
>>>>>>> Stashed changes

UCREWNetworkSubsystem::UCREWNetworkSubsystem() {
	Socket = nullptr;
	IsServer = false;
	UDPReceiver = nullptr;
	bufferPose.SetNumUninitialized(60);
<<<<<<< Updated upstream
<<<<<<< Updated upstream
	UDPSocket = nullptr;
	TCPListenerSocket = nullptr;
}

void UCREWNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection) {

    /*BroadcastTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UCREWNetworkSubsystem::BroadcastPresence), 1.0f);
    ListenTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UCREWNetworkSubsystem::ListenForBroadcasts), 0.1f);
    AcceptTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UCREWNetworkSubsystem::AcceptIncomingConnections), 0.1f);
    CheckDataTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UCREWNetworkSubsystem::CheckForIncomingData), 0.01f);*/
    StartNetworking();

	MulticastAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	MulticastAddr->SetIp(FIPv4Address(255, 255, 255, 255).Value);
	MulticastAddr->SetPort(16502);
	Socket = FUdpSocketBuilder(TEXT("UDPBroadcaster"))
		.AsReusable()
		.AsNonBlocking()
		.WithBroadcast()
		.BoundToEndpoint(FIPv4Endpoint(FIPv4Address::Any, 16502))
=======
=======
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
		.WithSendBufferSize(0);


	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	FString ThreadName = FString::Printf(TEXT("UDP PoseReceiver"));
	UDPReceiver = new FUdpSocketReceiver(Socket, ThreadWaitTime, *ThreadName);

	UDPReceiver->OnDataReceived().BindLambda([this](const FArrayReaderPtr& DataPtr, const FIPv4Endpoint& Endpoint) {
			FScopeLock Lock(&NetworkCriticalSection);
<<<<<<< Updated upstream
=======

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
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
			found->AddFragment(bufferPose, num, offset, index, PrecisionTime());
			//found->AddFrame(data, PrecisionTime());
=======
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
>>>>>>> Stashed changes
		});

<<<<<<< Updated upstream
	UDPReceiver->Start();
}

=======
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



>>>>>>> Stashed changes
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
    int32 byteSent;
	tempPose = Pose.Pose.MoveBones();
    FMemoryWriter MemoryWriter(NetworkBuffer, true);
    MemoryWriter << name;
    double t = PrecisionTime();
    MemoryWriter << t;
    TransformArraySerializer::SerializeCompressedTransforms(MemoryWriter, tempPose);
    for (auto p : ConnectedPeers) {
        p.Value->Send(NetworkBuffer.GetData(), NetworkBuffer.Num(), byteSent);
        //UE_LOG(LogTemp, Log, TEXT("Bytes sent: %d"), byteSent);
    }
	/*int16 num = tempPose.Num();
	int32 byteSent;
<<<<<<< Updated upstream
<<<<<<< Updated upstream
=======
	ECREWNetworkType type = ECREWNetworkType::Pose;
>>>>>>> Stashed changes
=======
	ECREWNetworkType type = ECREWNetworkType::Pose;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
		for (int j = 0; j < 4; j++) {
			Socket->SendTo(NetworkBuffer.GetData(), NetworkBuffer.Num(), byteSent, *MulticastAddr);
=======
		for (auto p : peers) {
			for (int32 j = 0; j < 4; j++) {
				UdpSocket->SendTo(NetworkBuffer.GetData(), NetworkBuffer.Num(), byteSent, p.ToInternetAddr().Get());
			}
>>>>>>> Stashed changes
		}
	}*/
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
<<<<<<< Updated upstream
<<<<<<< Updated upstream
    StopNetworking();
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
=======
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
>>>>>>> Stashed changes
=======
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
>>>>>>> Stashed changes
}


<<<<<<< Updated upstream

void UCREWNetworkSubsystem::StopNetworking()
{
    if (UDPSocket)
    {
        UDPSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(UDPSocket);
        UDPSocket = nullptr;
    }
    if (TCPListenerSocket)
    {
        TCPListenerSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(TCPListenerSocket);
        TCPListenerSocket = nullptr;
    }
    for (auto& Elem : ConnectedPeers)
    {
        if (Elem.Value)
        {
            UE_LOG(LogTemp, Log, TEXT("closing connection with: %s"), *Elem.Key);
            Elem.Value->Close();
            ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Elem.Value);
        }
    }
    ConnectedPeers.Empty();

<<<<<<< Updated upstream
   /* GetGameInstance()->GetWorld()->GetTimerManager().ClearTimer(BroadcastTimerHandle);
    GetGameInstance()->GetWorld()->GetTimerManager().ClearTimer(ListenTimerHandle);
    GetGameInstance()->GetWorld()->GetTimerManager().ClearTimer(AcceptTimerHandle);
    GetGameInstance()->GetWorld()->GetTimerManager().ClearTimer(CheckDataTimerHandle);*/
    FTSTicker::GetCoreTicker().RemoveTicker(BroadcastTickerHandle);
    FTSTicker::GetCoreTicker().RemoveTicker(ListenTickerHandle);
    FTSTicker::GetCoreTicker().RemoveTicker(AcceptTickerHandle);
    FTSTicker::GetCoreTicker().RemoveTicker(CheckDataTickerHandle);
=======
=======
>>>>>>> Stashed changes
	if (found != nullptr && found->source != nullptr) {
		found->source->UpdateStaticData(skeleton);
	}
#endif
>>>>>>> Stashed changes
}

void UCREWNetworkSubsystem::StartNetworking()
{
<<<<<<< Updated upstream
    // Get local IP
    bool valid;
    TSharedPtr<FInternetAddr> LocalAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, valid);
    if (!LocalAddr.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get local IP"));
        return;
    }
    LocalIP = LocalAddr->ToString(false);
=======
	//Need to be called from a scoped lock
	int32 byteSent;
	ECREWNetworkType type = ECREWNetworkType::Command;
<<<<<<< Updated upstream
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes

    // Setup UDP socket
    UDPSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_DGram, TEXT("UDP Broadcast Socket"), true);
    if (!UDPSocket)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create UDP socket"));
        return;
    }
    UDPSocket->SetBroadcast(true);

<<<<<<< Updated upstream
    TSharedRef<FInternetAddr> UDPAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    FIPv4Address IP(0, 0, 0, 0); // Any address
    UDPAddr->SetIp(IP.Value);
    UDPAddr->SetPort(UDPPort);
    if (!UDPSocket->Bind(*UDPAddr))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to bind UDP socket"));
        return;
    }

    // Setup TCP listener
    TCPListenerSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("TCP Listener Socket"), false);
    if (!TCPListenerSocket)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create TCP listener socket"));
        return;
    }
    TSharedRef<FInternetAddr> TCPAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    TCPAddr->SetIp(IP.Value);
    TCPAddr->SetPort(TCPPort);
    if (!TCPListenerSocket->Bind(*TCPAddr) || !TCPListenerSocket->Listen(8))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to setup TCP listener"));
        return;
    }

    BroadcastTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UCREWNetworkSubsystem::BroadcastPresence), 1.0f);
    ListenTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UCREWNetworkSubsystem::ListenForBroadcasts), 0.1f);
    AcceptTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UCREWNetworkSubsystem::AcceptIncomingConnections), 0.1f);
    CheckDataTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UCREWNetworkSubsystem::CheckForIncomingData), 0.01f);

    UE_LOG(LogTemp, Log, TEXT("Peer started: IP=%s, UDP=%d, TCP=%d"), *LocalIP, UDPPort, TCPPort);
}

bool UCREWNetworkSubsystem::BroadcastPresence(float DeltaTime)
{
    FScopeLock Lock(&NetworkCriticalSection);
    if (!UDPSocket)
        return true;

    FMemoryWriter MemoryWriter(NetworkBuffer, true);
    int32 port = TCPPort;
    MemoryWriter << LocalIP;
    MemoryWriter << port;

    TSharedRef<FInternetAddr> BroadcastAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    BroadcastAddr->SetIp(FIPv4Address(255, 255, 255, 255).Value);//SetBroadcastAddress();
    BroadcastAddr->SetPort(UDPPort);

    int32 BytesSent = 0;
    UDPSocket->SendTo(NetworkBuffer.GetData(), NetworkBuffer.Num(), BytesSent, *BroadcastAddr);
    return true;
}

bool UCREWNetworkSubsystem::ListenForBroadcasts(float DeltaTime)
{
    FScopeLock Lock(&NetworkCriticalSection);
    if (!UDPSocket)
        return true;

    TArray<uint8> Data;
    Data.SetNum(1024);
    TSharedRef<FInternetAddr> RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    int32 BytesRead = 0;

    uint32 PendingDataSize = 0;
    while (UDPSocket->HasPendingData(PendingDataSize) && PendingDataSize > 0) {
        NetworkBuffer.SetNumUninitialized(PendingDataSize);
        if (UDPSocket->RecvFrom(NetworkBuffer.GetData(), NetworkBuffer.Num(), BytesRead, *RemoteAddr) && BytesRead > 0)
        {
            NetworkBuffer.SetNum(BytesRead);
            FMemoryReader MemoryReader(NetworkBuffer);
            FString ip;
            int32 port;
            MemoryReader << ip;
            MemoryReader << port;
            AttemptConnection(ip, port);
        }
    }
    return true;
}

void UCREWNetworkSubsystem::AttemptConnection(const FString& PeerIP, int32 PeerPort)
{
    if (ConnectedPeers.Contains(PeerIP))
        return;

    FSocket* ClientTCPSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("TCP Client Socket"), false);

    if (!ClientTCPSocket)
        return;

    TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    bool valid;
    Addr->SetIp(*PeerIP, valid);
    Addr->SetPort(PeerPort);

    if (valid && ClientTCPSocket->Connect(*Addr))
    {
        FScopeLock Lock(&NetworkCriticalSection);
        ConnectedPeers.Add(PeerIP, ClientTCPSocket);
        UE_LOG(LogTemp, Log, TEXT("Connected to peer: %s:%d"), *PeerIP, PeerPort);
    }
    else
    {
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientTCPSocket);
    }
}

bool UCREWNetworkSubsystem::AcceptIncomingConnections(float DeltaTime)
{
    if (!TCPListenerSocket)
        return true;

    bool bHasPending;
    if (TCPListenerSocket->HasPendingConnection(bHasPending) && bHasPending)
    {
        FSocket* IncomingSocket = TCPListenerSocket->Accept(TEXT("Incoming Peer"));
        if (IncomingSocket)
        {
            TSharedPtr<FInternetAddr> PeerAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
            IncomingSocket->GetPeerAddress(*PeerAddr);
            FString PeerIP = PeerAddr->ToString(false);

            if (PeerIP != LocalIP && !ConnectedPeers.Contains(PeerIP))
            {
                FScopeLock Lock(&NetworkCriticalSection);
                ConnectedPeers.Add(PeerIP, IncomingSocket);
                UE_LOG(LogTemp, Log, TEXT("Accepted peer: %s"), *PeerIP);
            }
            else
            {
                IncomingSocket->Close();
                ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(IncomingSocket);
            }
        }
    }
    return true;
}


bool UCREWNetworkSubsystem::CheckForIncomingData(float DeltaTime)
{
    FScopeLock Lock(&NetworkCriticalSection);
    TArray<FString> DisconnectedPeers;
    for (auto& Elem : ConnectedPeers)
    {
        FSocket* InSocket = Elem.Value;
        if (InSocket)
        {
            if (InSocket->GetConnectionState() != SCS_Connected)
            {
                UE_LOG(LogTemp, Log, TEXT("not connected status from: %s"), *Elem.Key);
                DisconnectedPeers.Add(Elem.Key);
                continue; // Skip to the next peer
            }
            uint32 PendingDataSize = 0;
            while (InSocket->HasPendingData(PendingDataSize) && PendingDataSize > 0)
            {
                NetworkBuffer.SetNumUninitialized(PendingDataSize);
                //UE_LOG(LogTemp, Log, TEXT("Bytes pending: %d"), PendingDataSize);
                int32 BytesRead = 0;
                if (InSocket->Recv(NetworkBuffer.GetData(), NetworkBuffer.Num(), BytesRead))
                {
                    //UE_LOG(LogTemp, Log, TEXT("Bytes read: %d"), BytesRead);

                    if (BytesRead > 0)
                    {
                        NetworkBuffer.SetNum(BytesRead);
                        FMemoryReader MemoryReader(NetworkBuffer);
                        FName name;
                        double t;
                        MemoryReader << name;
                        MemoryReader << t;
                        TransformArraySerializer::DeserializeCompressedTransforms(MemoryReader, tempPose);

                        FReplicatedPosePlayHead* found = NamedPoseStreams.Find(name);

                        if (found == nullptr) {
                            found = &NamedPoseStreams.Add(name, FReplicatedPosePlayHead());
                        }
                        found->AddFrame(tempPose, t);
                    }
                }
                else {
                    UE_LOG(LogTemp, Log, TEXT("0 recv from: %s"), *Elem.Key);
                    DisconnectedPeers.Add(Elem.Key);
                }
            }
        }
        else {
            UE_LOG(LogTemp, Log, TEXT("nullptr socket from: %s"), *Elem.Key);
            DisconnectedPeers.Add(Elem.Key);
        }
    }
    for (const FString& PeerIP : DisconnectedPeers)
    {
        FSocket* InSocket = ConnectedPeers[PeerIP];
        if (InSocket)
        {
            InSocket->Close(); // Close the socket
            ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(InSocket); // Free resources
        }
        ConnectedPeers.Remove(PeerIP); // Remove from the map
        UE_LOG(LogTemp, Log, TEXT("Peer disconnected: %s"), *PeerIP);
    }
    return true;
=======
		for (auto p : c.remaining) {
			UdpSocket->SendTo(NetworkBuffer.GetData(), NetworkBuffer.Num(), byteSent, p.ToInternetAddr().Get());
		}
	}
>>>>>>> Stashed changes
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
