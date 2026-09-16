using UnrealBuildTool;
public class AuraPlayground : ModuleRules {
 public AuraPlayground(ReadOnlyTargetRules Target):base(Target) {
  if(Target.bBuildEditor)PrivateDependencyModuleNames.AddRange(new string[]{"UnrealEd","LandscapeEditor","Water","MeshDescription","PhysicsUtilities"});
  PrivateDependencyModuleNames.AddRange(new string[]{"ApplicationCore","PCG","Json","PhysicsCore"});
  PCHUsage=PCHUsageMode.UseExplicitOrSharedPCHs;
  PublicDependencyModuleNames.AddRange(new string[]{"Core","CoreUObject","Engine","AudioExtensions","InputCore","Landscape","NavigationSystem","AIModule","Slate","SlateCore"});
 }
}
