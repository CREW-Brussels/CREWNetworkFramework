// Copyright Epic Games, Inc. All Rights Reserved.

#include "CREWNetwork.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "CREWNetworkSettings.h"

#define LOCTEXT_NAMESPACE "FCREWNetworkModule"

void FCREWNetworkModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FCREWNetworkModule::RegisterSettings);
}

void FCREWNetworkModule::RegisterSettings()
{
	if (ISettingsModule* settings = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
		settings->RegisterSettings(
			"Project",
			"Plugins",
			"CREWNetwork",
			LOCTEXT("CREWNetworkName", "CREW Network"),
			LOCTEXT("CREWNetworkDesc", "CREW Network framework, auto discorver compatible instance of the project on the same network, auto connect them, and allow easy pose replication and simple messaging betwwen them."),
			GetMutableDefault<UCREWNetworkSettings>()
		);
	}
}

void FCREWNetworkModule::ShutdownModule()
{
	if (ISettingsModule* settings = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
		settings->UnregisterSettings("Project", "Plugins", "CREWNetwork");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCREWNetworkModule, CREWNetwork)