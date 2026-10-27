using UnrealBuildTool;
using System.IO;

public class Politika : ModuleRules
{
    public Politika(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "../PolitikaCore"));
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "NavigationSystem", "Json", "PolitikaBootstrap" });
        PrivateDependencyModuleNames.AddRange(new[] { "InputCore", "Slate", "SlateCore", "ApplicationCore", "UMG" });
    }
}
