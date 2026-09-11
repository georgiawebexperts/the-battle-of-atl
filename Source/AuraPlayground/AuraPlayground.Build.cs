using UnrealBuildTool;
public class AuraPlayground : ModuleRules {
 public AuraPlayground(ReadOnlyTargetRules Target):base(Target) {
  if(Target.bBuildEditor)PrivateDependencyModuleNames.AddRange(new string[]{"UnrealEd","LandscapeEditor","Water"});
  PrivateDependencyModuleNames.Add("ApplicationCore");
  PCHUsage=PCHUsageMode.UseExplicitOrSharedPCHs;
  PublicDependencyModuleNames.AddRange(new string[]{"Core","CoreUObject","Engine","InputCore","Landscape","NavigationSystem","AIModule","Slate","SlateCore"});
 }
}
