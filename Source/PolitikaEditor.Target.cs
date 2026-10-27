using UnrealBuildTool;

public class PolitikaEditorTarget : TargetRules
{
    public PolitikaEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.AddRange(new[] { "PolitikaBootstrap", "Politika", "PolitikaEditor" });
    }
}
