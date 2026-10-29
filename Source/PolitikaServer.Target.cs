using UnrealBuildTool;

// MP-3 headless dedicated server: the same gameplay modules and rules as the game target,
// built without rendering, audio output, local players or the shell UI (TargetType.Server).
public class PolitikaServerTarget : TargetRules
{
    public PolitikaServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.AddRange(new[] { "PolitikaBootstrap", "Politika" });
    }
}
