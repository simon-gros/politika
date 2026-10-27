using UnrealBuildTool;

public class PolitikaEditor : ModuleRules
{
    public PolitikaEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UnrealEd", "Politika", "PolitikaBootstrap" });
    }
}
