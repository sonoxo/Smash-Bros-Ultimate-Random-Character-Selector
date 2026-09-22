using UnrealBuildTool;
using System.Collections.Generic;

public class SmashRandomiserEditorTarget : TargetRules
{
    public SmashRandomiserEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.Add("SmashRandomiser");
    }
}
