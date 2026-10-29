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
        string GitEntry = Path.Combine(Root, ".git");
        if (Directory.Exists(GitEntry) || File.Exists(GitEntry)) {
            // Linked worktrees have a .git file, private HEAD/index, and shared branch refs.
            // Let Git resolve each path rather than assuming one directory owns them all.
            string HeadPath = ResolveGitPath(Root, "HEAD");
            ExternalDependencies.Add(HeadPath);
            ExternalDependencies.Add(ResolveGitPath(Root, "index"));
            if (File.Exists(GitEntry)) { ExternalDependencies.Add(GitEntry); }
            string Ref = File.ReadAllText(HeadPath).Trim();
            if (Ref.StartsWith("ref: ")) {
                ExternalDependencies.Add(ResolveGitPath(Root, Ref.Substring(5)));
            }
            string PackedRefs = ResolveGitPath(Root, "packed-refs");
            if (File.Exists(PackedRefs)) { ExternalDependencies.Add(PackedRefs); }
        }
    }

    private static string ResolveGitPath(string Root, string Name)
    {
        var Start = new ProcessStartInfo("git") {
            WorkingDirectory = Root, UseShellExecute = false, RedirectStandardOutput = true
        };
        foreach (string Arg in new[] { "rev-parse", "--git-path", Name }) {
            Start.ArgumentList.Add(Arg);
        }
        using (Process Git = Process.Start(Start)) {
            if (Git == null) { throw new BuildException("Could not resolve Git identity dependencies."); }
            string Result = Git.StandardOutput.ReadToEnd().Trim();
            Git.WaitForExit();
            if (Git.ExitCode != 0 || string.IsNullOrEmpty(Result)) {
                throw new BuildException("Could not resolve Git identity dependency: " + Name);
            }
            return Path.GetFullPath(Path.IsPathRooted(Result) ? Result : Path.Combine(Root, Result));
        }
    }
}
