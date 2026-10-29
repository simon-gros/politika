using UnrealBuildTool;

public class PolitikaEditor : ModuleRules
{
    public PolitikaEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "NavigationSystem", "Landscape", "BSPUtils", "UnrealEd", "AudioEditor", "Json", "AssetTools", "AssetRegistry", "Politika", "PolitikaBootstrap" });
    }
}
