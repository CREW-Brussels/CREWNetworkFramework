#include "CREWNetworkSubsystem.h"
#include "SocketSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UCREWNetworkSubsystem::UCREWNetworkSubsystem() {
	Socket = nullptr;
	IsServer = false;
	UDPReceiver = nullptr;
	bufferPose.SetNumUninitialized(60);
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
}



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
            Elem.Value->Close();
            ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Elem.Value);
        }
    }
    ConnectedPeers.Empty();

   /* GetGameInstance()->GetWorld()->GetTimerManager().ClearTimer(BroadcastTimerHandle);
    GetGameInstance()->GetWorld()->GetTimerManager().ClearTimer(ListenTimerHandle);
    GetGameInstance()->GetWorld()->GetTimerManager().ClearTimer(AcceptTimerHandle);
    GetGameInstance()->GetWorld()->GetTimerManager().ClearTimer(CheckDataTimerHandle);*/
    FTSTicker::GetCoreTicker().RemoveTicker(BroadcastTickerHandle);
    FTSTicker::GetCoreTicker().RemoveTicker(ListenTickerHandle);
    FTSTicker::GetCoreTicker().RemoveTicker(AcceptTickerHandle);
    FTSTicker::GetCoreTicker().RemoveTicker(CheckDataTickerHandle);
}

void UCREWNetworkSubsystem::StartNetworking()
{
    // Get local IP
    bool valid;
    TSharedPtr<FInternetAddr> LocalAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, valid);
    if (!LocalAddr.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get local IP"));
        return;
    }
    LocalIP = LocalAddr->ToString(false);

    // Setup UDP socket
    UDPSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_DGram, TEXT("UDP Broadcast Socket"), true);
    if (!UDPSocket)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create UDP socket"));
        return;
    }
    UDPSocket->SetBroadcast(true);

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
    TArray<FString> DisconnectedPeers;
    for (auto& Elem : ConnectedPeers)
    {
        FSocket* InSocket = Elem.Value;
        if (InSocket)
        {
            if (InSocket->GetConnectionState() != SCS_Connected)
            {
                DisconnectedPeers.Add(Elem.Key);
                continue; // Skip to the next peer
            }
            uint32 PendingDataSize = 0;
            while (InSocket->HasPendingData(PendingDataSize) && PendingDataSize > 0)
            {
                FScopeLock Lock(&NetworkCriticalSection);
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
            }
        }
        else {
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
}
