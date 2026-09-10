using UnrealBuildTool;
public class AuraPlaygroundTarget : TargetRules {
 public AuraPlaygroundTarget(TargetInfo Target):base(Target) {Type=TargetType.Game;DefaultBuildSettings=BuildSettingsVersion.Latest;ExtraModuleNames.Add("AuraPlayground");}
}
