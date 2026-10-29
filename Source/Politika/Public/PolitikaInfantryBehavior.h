#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PolitikaInfantryBehavior.generated.h"

/** Optional shared gameplay contract, independent of meshes, animation and audio. */
UCLASS(BlueprintType)
class POLITIKA_API UPolitikaInfantryBehaviorDefinition final : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) int32 MagazineCapacity = 30;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float ReloadSeconds = 3;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) bool bRequiresLineOfFire = true;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) bool bAutoAcquireWhenIdle = true;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float AcquisitionRangeCm = 800;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float TurnDegreesPerSecond = 360;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float AcceptableAimDegrees = 11.5f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float GrenadeRangeCm = 650;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float GrenadeDamage = 50;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float GrenadeRadiusCm = 150;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float GrenadeSpeedCmPerSecond = 900;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float GrenadeArcHeightCm = 180;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float GrenadeWindupSeconds = 2;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float GrenadeReloadSeconds = 3;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float SprintMultiplier = 1.5f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float SprintSeconds = 4;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float SprintCooldownSeconds = 12;
    bool Validate() const;
};
