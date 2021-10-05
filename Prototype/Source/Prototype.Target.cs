// Fill out your copyright notice in the Description page of Project Settings.

using System;
using UnrealBuildTool;
using System.Collections.Generic;

public class PrototypeTarget : TargetRules
{
    public PrototypeTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;

        DefaultBuildSettings = BuildSettingsVersion.V2;

        ExtraModuleNames.AddRange(new string[] { "Prototype" });
                    
        foreach (string moduleName in ExtraModuleNames)
        {
            Console.WriteLine("---- Unknown ---- {0}::ExtraModuleNames \"{1}\"", new object[] { this.ToString(), moduleName });
        }
    }
}
