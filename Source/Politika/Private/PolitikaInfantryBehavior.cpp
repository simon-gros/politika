#include "PolitikaInfantryBehavior.h"
bool UPolitikaInfantryBehaviorDefinition::Validate() const
{
    const float Positive[] = {ReloadSeconds, AcquisitionRangeCm, TurnDegreesPerSecond, AcceptableAimDegrees,
        GrenadeRangeCm, GrenadeDamage, GrenadeRadiusCm, GrenadeSpeedCmPerSecond,
        GrenadeWindupSeconds, GrenadeReloadSeconds, SprintMultiplier, SprintSeconds, SprintCooldownSeconds};
    for (float Value : Positive) if (!FMath::IsFinite(Value) || Value <= 0) return false;
    return MagazineCapacity > 0 && MagazineCapacity <= 1000 && AcceptableAimDegrees <= 180
        && FMath::IsFinite(GrenadeArcHeightCm) && GrenadeArcHeightCm >= 0
        && SprintMultiplier <= 4 && SprintCooldownSeconds >= SprintSeconds;
}
