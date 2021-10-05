
using System;
using System.IO;
using UnrealBuildTool;
using System.Collections.Generic;

public class CustomEditors : ModuleRules
{
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

	private string ThirdPartyPath
	{
		get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
	}

	public CustomEditors(ReadOnlyTargetRules Target) : base(Target)
	{
		Console.WriteLine("-= DoD =- BUILD MODULE \"CustomEditors\"");

        PrivatePCHHeaderFile = "CustomEditors.h";

		bOutputPubliclyDistributable = false;

		PublicIncludePaths.AddRange(new string[] {
            "CustomEditors"
        });
		
		PrivateIncludePaths.AddRange(new string[] {
            "CustomEditors",
        });

		PrivateDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "UnrealEd",
            "Engine",
            "InputCore",
            "AssetTools",
            "XmlParser",
            "BlueprintGraph",
            "AnimGraph",
        });
		
		//	Add Prototype (game module):
		PublicDependencyModuleNames.Add("Prototype");

		Load_TinyXML2(Target);
	}

	public bool Load_TinyXML2(ReadOnlyTargetRules Target)
	{
		bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
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