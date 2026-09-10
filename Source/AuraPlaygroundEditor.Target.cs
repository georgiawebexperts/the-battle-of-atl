using UnrealBuildTool;
public class AuraPlaygroundEditorTarget : TargetRules {
 public AuraPlaygroundEditorTarget(TargetInfo Target):base(Target) {Type=TargetType.Editor;DefaultBuildSettings=BuildSettingsVersion.Latest;ExtraModuleNames.Add("AuraPlayground");}
}
