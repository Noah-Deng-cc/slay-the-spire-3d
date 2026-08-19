using UnrealBuildTool;
using System.Collections.Generic;

public class SS3DEditorTarget : TargetRules
{
    public SS3DEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("SS3D");
    }
}
