using UnrealBuildTool;
using System.Collections.Generic;

public class SS3DEditorTarget : TargetRules
{
    public SS3DEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.Add("SS3D");
    }
}
