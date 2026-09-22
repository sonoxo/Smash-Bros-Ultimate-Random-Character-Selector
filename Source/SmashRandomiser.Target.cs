using UnrealBuildTool;
using System.Collections.Generic;

public class SmashRandomiserTarget : TargetRules
{
    public SmashRandomiserTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.Add("SmashRandomiser");
    }
}
