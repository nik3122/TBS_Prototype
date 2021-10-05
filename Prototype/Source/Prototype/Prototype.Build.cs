// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using System;
using UnrealBuildTool;

public class Prototype : ModuleRules
{
    public Prototype(ReadOnlyTargetRules Target) : base (Target)
	{
		Console.WriteLine("-= DoD =- BUILD MODULE \"Prototype\"");

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        //PrivatePCHHeaderFile = "Prototype.h";

        bUseUnity = false;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "UMG",
            "Slate",
            "SlateCore",
            "Niagara",
            "GeometricObjects",
            "DeveloperSettings",
        });

        PublicIncludePaths.AddRange(new string[] {
            Path.Combine(ModuleDirectory, "Niagara")
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Sockets",
        });

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

        //Load_TinyXML2(Target);
    }

    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }

    public bool Load_TinyXML2(ReadOnlyTargetRules Target)
    {
        bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32)) {
            isLibrarySupported = true;

            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x86";
            string LibrariesPath = Path.Combine(ThirdPartyPath, "TinyXML2", "Libraries");

            Console.WriteLine("-= DoD =- TinyXML2 lib path = " + LibrariesPath);

            string Sufix = Target.Configuration == UnrealTargetConfiguration.Debug ? "d" : "";
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "TinyXML2" + Sufix + ".lib"));

            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "TinyXML2", "Includes"));

            PublicDefinitions.Add(string.Format("WITH_TINYXML2D_LIB_BINDING={0}", 1));

            Console.WriteLine("-= DoD =- PUBLIC ADDITIONAL LIBRARY = TinyXML2");
        }

        return isLibrarySupported;
    }

}
