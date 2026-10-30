#include "PolitikaPrototypeGameplay.h"
#include "PolitikaInfantryBehavior.h"
#include "PolitikaInfantryPresentation.h"
#include "PolitikaBootstrap.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"

namespace
{
// Blast damage spares the thrower and its allies, exactly as direct fire does.
template <typename ProjectileT>
bool AreAllied(const APolitikaPrototypeUnit* Candidate, const ProjectileT& Projectile)
{
    return politika::rules::AreAllied(Candidate->GetOwnerPlayer(), Candidate->GetTeamId(),
        Projectile.Owner, Projectile.Team);
}
}

void APolitikaPrototypeUnit::ConfigureInfantryBehavior(UPolitikaInfantryBehaviorDefinition* Definition)
{
    InfantryBehavior = Definition;
    Ammo = Definition ? Definition->MagazineCapacity : 0;
    ReloadRemaining = GrenadeCooldown = GrenadeWindup = SprintRemaining = SprintCooldown = 0;
    StepStartPosition = GetActorLocation();
}

bool APolitikaPrototypeUnit::CanReload() const
{
    return IsAlive() && InfantryBehavior && Ammo < InfantryBehavior->MagazineCapacity
        && ReloadRemaining <= 0 && GrenadeWindup <= 0;
}

bool APolitikaPrototypeUnit::CanSprint() const
{
    return IsAlive() && InfantryBehavior && SprintCooldown <= 0 && GrenadeWindup <= 0;
}

bool APolitikaPrototypeUnit::CanUseGrenade(const APolitikaPrototypeUnit* Target) const
{
    return IsAlive() && InfantryBehavior && Target && Target != this && Target->IsAlive()
        && Target->IsAttackable() && IsHostileTo(Target)
        && GrenadeCooldown <= 0 && GrenadeWindup <= 0 && SprintRemaining <= 0
        && ReloadRemaining <= 0
        && FVector::Dist2D(GetActorLocation(), Target->GetActorLocation()) <= InfantryBehavior->GrenadeRangeCm;
}

void APolitikaPrototypeUnit::BeginReload()
{
    if (!HasAuthority() || !CanReload()) return;
    // Discipline shortens or lengthens the reload (DISC-D3).
    const int32 Permille = GetModifierPermille(politika::modifiers::Attribute::ReloadTime);
    ReloadRemaining = InfantryBehavior->ReloadSeconds * static_cast<float>(Permille) / static_cast<float>(politika::modifiers::kNeutral);
#if !UE_BUILD_SHIPPING
    if (Permille != politika::modifiers::kNeutral)
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_DISCIPLINE_EFFECT effect=reload_time unit=%llu participant=%u value=%d permille=%d base_seconds=%.3f seconds=%.3f step=%llu"),
            StableId, OwnerPlayer, GetResolveMilli(), Permille, InfantryBehavior->ReloadSeconds, ReloadRemaining, AuthorityStep());
#endif
    InfantryPresentation->Reload(ReloadRemaining);
    MulticastAbilityPresentation(0, ReloadRemaining);
    UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_RELOAD_BEGIN unit=%llu ammo=%d seconds=%.2f step=%llu"),
        StableId, Ammo, ReloadRemaining, AuthorityStep());
}

void APolitikaPrototypeUnit::BeginGrenade(APolitikaPrototypeUnit* Target)
{
    if (!HasAuthority()) return;
    StopOrder();
    GrenadeTarget = Target;
    GrenadeWindup = InfantryBehavior->GrenadeWindupSeconds;
    InfantryPresentation->Grenade(GrenadeWindup);
    MulticastAbilityPresentation(1, GrenadeWindup);
    UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_GRENADE_WINDUP unit=%llu target=%llu"), StableId, Target->StableId);
}

bool APolitikaPrototypeUnit::HasLineOfFire(const APolitikaPrototypeUnit* Target) const
{
    if (!InfantryBehavior || !InfantryBehavior->bRequiresLineOfFire) return true;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(InfantryLineOfFire), false, this);
    Params.AddIgnoredActor(Target);
    FHitResult Hit;
    return !GetWorld()->LineTraceSingleByObjectType(Hit, GetActorLocation() + FVector(0, 0, 45),
        Target->GetActorLocation() + FVector(0, 0, 30), FCollisionObjectQueryParams(ECC_WorldStatic), Params);
}

void APolitikaPrototypeMatch::LaunchGrenade(APolitikaPrototypeUnit* Unit, const FVector& Destination)
{
    const auto* Data = Unit->InfantryBehavior.Get();
    FInfantryProjectile P;
    P.Start = Unit->GetActorLocation() + FVector(0, 0, 45);
    P.End = Destination;
    P.Previous = P.Start;
    P.Duration = FMath::Max(0.05f, static_cast<float>(FVector::Dist2D(P.Start, P.End)) / Data->GrenadeSpeedCmPerSecond);
    P.Height = Data->GrenadeArcHeightCm;
    P.Owner = Unit->OwnerPlayer;
    P.Team = Unit->TeamId;
    P.Damage = Data->GrenadeDamage;
    P.Radius = Data->GrenadeRadiusCm;
    P.Generation = MatchGeneration;
    InfantryProjectiles.Add(P);
    Unit->InfantryPresentation->Grenade(0.2f);
    Unit->MulticastAbilityPresentation(1, 0.2f);
    UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_GRENADE_LAUNCHED unit=%llu duration=%.3f radius=%.1f"),
        Unit->StableId, P.Duration, P.Radius);
}

void APolitikaPrototypeMatch::AdvanceGrenades(float DeltaSeconds)
{
    if (!HasAuthority()) return;
    for (int32 Index = InfantryProjectiles.Num() - 1; Index >= 0; --Index)
    {
        auto& P = InfantryProjectiles[Index];
        if (P.Generation != MatchGeneration) { InfantryProjectiles.RemoveAt(Index); continue; }
        P.Elapsed += DeltaSeconds;
        const float Alpha = FMath::Clamp(P.Elapsed / P.Duration, 0.0f, 1.0f);
        FVector Position = FMath::Lerp(P.Start, P.End, Alpha) + FVector(0, 0, 4 * P.Height * Alpha * (1 - Alpha));
        FHitResult Hit;
        const bool bHit = GetWorld()->SweepSingleByObjectType(Hit, P.Previous, Position, FQuat::Identity,
            FCollisionObjectQueryParams(ECC_WorldStatic), FCollisionShape::MakeSphere(5));
        if (bHit) Position = Hit.Location;
#if !UE_BUILD_SHIPPING
        DrawDebugSphere(GetWorld(), Position, 8, 8, FColor::Orange, false, 0.06f);
        DrawDebugLine(GetWorld(), P.Previous, Position, FColor::Orange, false, 0.1f, 0, 2);
#endif
        P.Previous = Position;
        if (Alpha < 1 && !bHit) continue;
        int32 Victims = 0;
        for (APolitikaPrototypeUnit* Candidate : Units)
        {
            if (!Candidate || !Candidate->IsAlive() || !Candidate->IsAttackable()
                || AreAllied(Candidate, P) || FVector::Dist(Candidate->GetActorLocation(), Position) > P.Radius) continue;
            Candidate->ReceiveWeaponHit(P.Damage, nullptr, true);
            ++Victims;
        }
#if !UE_BUILD_SHIPPING
        DrawDebugSphere(GetWorld(), Position, P.Radius, 16, FColor::Red, false, 0.4f);
#endif
        UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_GRENADE_IMPACT victims=%d blocked=%d"), Victims, bHit ? 1 : 0);
        InfantryProjectiles.RemoveAt(Index);
    }
}

void APolitikaPrototypePlayerController::GrenadeSelected()
{
    // One press throws one grenade: only the first unit of the active Tab subgroup, never the
    // whole group. The group bar captures clicks, so it is never a grenade aim point.
    auto* ActiveMatch = ResolveMatch();
    const TArray<APolitikaPrototypeUnit*> Subgroup = GetActiveSubgroupUnits(ActiveMatch);
    APolitikaPrototypeUnit* Unit = Subgroup.IsEmpty() ? nullptr : Subgroup[0];
    if (!Unit || GroupBarSlotUnderCursor() != INDEX_NONE) return;
    FHitResult Hit;
    GetHitResultUnderCursor(ECC_Visibility, false, Hit);
    auto* Target = Cast<APolitikaPrototypeUnit>(Hit.GetActor());
    SubmitIntent(ActiveMatch, Unit, Target, politika::rules::GrenadeOrder{Target ? Target->GetStableId() : 0});
}

void APolitikaPrototypePlayerController::ReloadSelected()
{
    auto* ActiveMatch = ResolveMatch();
    SubmitGroupIntent(ActiveMatch, GetActiveSubgroupUnits(ActiveMatch), nullptr, politika::rules::ReloadOrder{});
}

void APolitikaPrototypePlayerController::SprintSelected()
{
    auto* ActiveMatch = ResolveMatch();
    SubmitGroupIntent(ActiveMatch, GetActiveSubgroupUnits(ActiveMatch), nullptr, politika::rules::SprintOrder{});
}
