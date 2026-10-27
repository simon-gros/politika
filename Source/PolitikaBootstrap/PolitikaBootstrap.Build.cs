using UnrealBuildTool;
using System.Diagnostics;
using System.IO;

public class PolitikaBootstrap : ModuleRules
{
    public PolitikaBootstrap(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.Add("Core");
        PrivateDependencyModuleNames.Add("Json");

        // Rust owns canonical version parsing/generation, including direct UBT builds.
        string Root = Path.GetFullPath(Path.Combine(ModuleDirectory, "../.."));
        string Generated = Path.Combine(Root, "Intermediate", "PolitikaIdentity");
        Directory.CreateDirectory(Generated);
        var Start = new ProcessStartInfo("cargo") {
            WorkingDirectory = Root, UseShellExecute = false
        };
        foreach (string Arg in new[] { "run", "--locked", "--quiet", "--manifest-path",
            Path.Combine(Root, "tools/version/Cargo.toml"), "--", "generate", "--output",
            Path.Combine(Generated, "PolitikaVersion.generated.h") }) {
            Start.ArgumentList.Add(Arg);
        }
        using (Process Generator = Process.Start(Start)) {
            if (Generator == null) { throw new BuildException("Could not start Rust version generator."); }
            Generator.WaitForExit();
            if (Generator.ExitCode != 0) { throw new BuildException("POLITIKA Rust version generation failed."); }
        }
        PrivateIncludePaths.Add(Generated);
        ExternalDependencies.Add(Path.Combine(Root, "version.json"));
        ExternalDependencies.AddRange(Directory.GetFiles(Path.Combine(Root, "tools/version/src"), "*.rs", SearchOption.AllDirectories));
        // Refresh the generated Git identity when the current checkout changes.
        string GitDir = Path.Combine(Root, ".git");
        if (Directory.Exists(GitDir)) {
            ExternalDependencies.Add(Path.Combine(GitDir, "HEAD"));
            ExternalDependencies.Add(Path.Combine(GitDir, "index"));
            string Ref = File.ReadAllText(Path.Combine(GitDir, "HEAD")).Trim();
            if (Ref.StartsWith("ref: ")) { ExternalDependencies.Add(Path.Combine(GitDir, Ref.Substring(5))); }
        }
    }
}
