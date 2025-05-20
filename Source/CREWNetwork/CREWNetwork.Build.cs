// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CREWNetwork : ModuleRules
{
	public CREWNetwork(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
				"CoreUObject",
				"Engine",
				"AnimGraphRuntime",
                "Sockets",
                "Networking"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "AnimationCore",
				"Settings"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

       /* if (Target.bBuildEditor)
        {
<<<<<<< Updated upstream
            PrivateDependencyModuleNames.Add("UnrealEd"); // Editor-only modules
        }*/
=======
            if (rawObject.TryGetObjectArrayField("Plugins", out var pluginObjects))
            {
                foreach (JsonObject pluginObject in pluginObjects)
                {
                    pluginObject.TryGetStringField("Name", out var pluginName);

                    pluginObject.TryGetBoolField("Enabled", out var pluginEnabled);

                    if (pluginName == "LiveLink" && pluginEnabled)
                    {
						using_live_link = true;
                    }
                }
            }
        }
		if (using_live_link)
		{
            PublicDefinitions.Add("WITH_LIVE_LINK=1");
			PrivateDefinitions.Add("WITH_LIVE_LINK=1");
            PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "LiveLink",
                "LiveLinkInterface",
                "LiveLinkMessageBusFramework"
			}
            );
            System.Console.WriteLine("Live link plugin is enabled");
        }
		else
		{
            PublicDefinitions.Add("WITH_LIVE_LINK=0");
            PrivateDefinitions.Add("WITH_LIVE_LINK=0");
            System.Console.WriteLine("Live link plugin is disabled");
        }
        /* if (Target.bBuildEditor)
         {
             PrivateDependencyModuleNames.Add("UnrealEd"); // Editor-only modules
         }*/
>>>>>>> Stashed changes
    }
}
