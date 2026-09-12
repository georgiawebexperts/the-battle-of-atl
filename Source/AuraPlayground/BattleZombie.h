#pragma once
#include "CoreMinimal.h"
#include "PiedmontExplorer.h"
#include "BattleZombie.generated.h"
class UMaterialInstanceDynamic;class AAIController;class ABattleWeaponCrate;class ABattleGunman;
UCLASS()
class AURAPLAYGROUND_API ABattleZombie : public APiedmontExplorer {
 GENERATED_BODY()
public:
 ABattleZombie();
 UPROPERTY(EditAnywhere) int32 VisualStyle=-1;
 UPROPERTY() TObjectPtr<USkeletalMesh> CoatMesh;
 UPROPERTY() TObjectPtr<USkeletalMesh> JacketMesh;
 UPROPERTY() TArray<TObjectPtr<UAnimSequence>> StyleIdle;
 UPROPERTY() TArray<TObjectPtr<UAnimSequence>> StyleWalk;
 UPROPERTY() TArray<TObjectPtr<UAnimSequence>> StyleRun;
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 UPROPERTY(BlueprintReadOnly) float Health=100;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float WeaponDropChance=.25f;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<ABattleWeaponCrate> DroppedWeapon;
 void DropWeapon();
 UPROPERTY(BlueprintReadOnly) bool bSprinter=false;
 UPROPERTY(BlueprintReadOnly) bool bTelegraphing=false;
 UPROPERTY(BlueprintReadOnly) int32 Attacks=0;
 UPROPERTY(BlueprintReadOnly) int32 PathRequests=0;
 UPROPERTY(BlueprintReadOnly) int32 Headshots=0;
 UPROPERTY(BlueprintReadOnly) FString Subtitle;
 UPROPERTY(BlueprintReadOnly) float SubtitleRemaining=0;
 float MoveSpeed=140,AttackDamage=20,WarningSeconds=.9f;
 float Emergence=1;
 void Speak(bool Charge=false);
 FString CharacterName() const{return VisualStyle==0?TEXT("Farmers Market Vendor"):TEXT("Punk");}
protected:
 virtual bool CanUseWeapon() const override{return false;}
private:
 UPROPERTY() TObjectPtr<UStaticMeshComponent> LeftEye;
 UPROPERTY() TObjectPtr<UStaticMeshComponent> RightEye;
 UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> Skin;
 UFUNCTION() void EnforceParkBoundary(float Dt,FVector OldLocation,FVector OldVelocity);
 FVector LastParkPosition;
 float AttackDelay=1,WarningRemaining=0,PathDelay=0,DeathTime=0,Flinch=0,SpeechDelay=12;
};
UCLASS()
class AURAPLAYGROUND_API ABattleEnemyDirector : public AActor {
 GENERATED_BODY()
public:
 ABattleEnemyDirector();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 UPROPERTY(BlueprintReadOnly) int32 DesiredZombies=0;
 UPROPERTY(BlueprintReadOnly) int32 LiveZombies=0;
 UPROPERTY(BlueprintReadOnly) int32 Spawned=0;
 UPROPERTY(BlueprintReadOnly) int32 Waves=0;
 UPROPERTY(BlueprintReadOnly) int32 WaveSpawned=0;
 UPROPERTY(BlueprintReadOnly) int32 RemainingWave=0;
 UPROPERTY(BlueprintReadWrite) bool bFreezeSpawns=false;
private:
 float SpawnDelay=5,WaveDelay=0;
 bool SpawnZombie(bool Wave);
public:
 void TickGunmen(float Dt);
 bool SpawnGunman();
 UPROPERTY(BlueprintReadOnly) int32 GunmenSpawned=0;
 UPROPERTY(BlueprintReadOnly) float GunmanDelay=60;
 UPROPERTY() TWeakObjectPtr<ABattleGunman> ActiveGunman;
};
