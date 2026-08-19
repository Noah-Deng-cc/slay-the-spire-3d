using UnrealBuildTool;
using System.Collections.Generic;

public class SS3DTarget : TargetRules
{
    public SS3DTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.Add("SS3D");
    }
}
