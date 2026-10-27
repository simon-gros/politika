using UnrealBuildTool;

public class Politika : ModuleRules
{
    public Politika(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "PolitikaBootstrap" });
        PrivateDependencyModuleNames.AddRange(new[] { "InputCore", "Slate", "SlateCore", "ApplicationCore", "UMG" });
    }
}
