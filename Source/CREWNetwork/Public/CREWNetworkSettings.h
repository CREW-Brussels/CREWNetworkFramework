// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CREWNetworkSettings.generated.h"


UCLASS(config=CREWNetwork, defaultconfig, Meta=(DisplayName="CREW Network"))
class CREWNETWORK_API UCREWNetworkSettings : public UObject//UGameInstanceSubsystem//UGameInstanceSubsystem//UEngineSubsystem
{
	GENERATED_BODY()

public:
	UCREWNetworkSettings(const FObjectInitializer &ObjectInitializer);

	UPROPERTY(EditAnywhere, config, Category="UDP",	Meta=(ConfigRestartRequired = true, DisplayName="Broadcasting Port",
			Tooltip = "The port CREW Network is going to use for broadcasting presence in the network through broadcast.\nNOTE: You need to restart the engine to apply new changes."))
	int BroadcastPort = 16502;

	UPROPERTY(EditAnywhere, config, Category="UDP", Meta=(ConfigRestartRequired = true, DisplayName="Communication Port",
		Tooltip = "The port CREW Network is going to use for communicating between instances in the same network through unicast.\nNOTE: You need to restart the engine to apply new changes."))
	int CommunicationPort = 16503;

	UPROPERTY(EditAnywhere, config, Category="AutoConnect", Meta=(ConfigRestartRequired = true, DisplayName="Application Name",
		Tooltip = "The application name is used to prevent uncompatible projects from discorvering and communicating each others.\nNOTE: You need to restart the engine to apply new changes."))
	FName AppName = "Default";

	UPROPERTY(EditAnywhere, config, Category="AutoConnect", Meta=(ConfigRestartRequired = true, DisplayName="Auto Connect",
		Tooltip = "Auto Connect will attempt to connect clients to a running server if one is found on the network, disabling auto connect wont prevent discorvered instances from communicating using CREW Network.\nNOTE: You need to restart the engine to apply new changes."))
	bool bEnableAutoConnect = true;
};

