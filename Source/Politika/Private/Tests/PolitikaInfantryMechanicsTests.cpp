#if WITH_DEV_AUTOMATION_TESTS
#include "PolitikaPrototypeGameplay.h"
#include "PolitikaInfantryBehavior.h"
#include "PolitikaInfantryPresentation.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaInfantryMechanicsTest,
    "Politika.Infantry.MagazineGrenadeSprintAndAuthority",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPolitikaInfantryMechanicsTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    const UWorld::InitializationValues Init = UWorld::InitializationValues().AllowAudioPlayback(false)
        .RequiresHitProxies(false).CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false)
        .ShouldSimulatePhysics(false).SetTransactional(false).CreateFXSystem(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("InfantryMechanics"),
        GetTransientPackage(), false, ERHIFeatureLevel::Num, &Init);
    if (!World) return false;
    auto* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    auto Spawn = [&](uint64 Id, uint32 Owner, FVector Position)
    {
        auto* Unit = World->SpawnActor<APolitikaPrototypeUnit>(Position, FRotator::ZeroRotator);
        const FName Faction = Owner == 1 ? FName(TEXT("faction.player"))
            : Owner == 2 ? FName(TEXT("faction.opposition")) : FName(TEXT("faction.unaligned"));
        Unit->Configure(Id, Owner, FText::FromString(TEXT("Test infantry")), 1000, 210, 650, 8, 0.05f,
            true, Mesh, FVector(0.45f, 0.45f, 0.84f), FLinearColor::White, Faction);
        return Unit;
    };
    auto* Shooter = Spawn(1, 1, FVector(0,0,42));
    auto* Target = Spawn(2, 2, FVector(200,0,42));
    auto* Ally = Spawn(3, 1, FVector(210,30,42));
    auto* SystemInfantry = Spawn(4, 0, FVector(1800,1800,42));
    auto* Behavior = NewObject<UPolitikaInfantryBehaviorDefinition>();
    TestTrue(TEXT("Default behavior validates"), Behavior->Validate());
    Behavior->SprintMultiplier = NAN;
    TestFalse(TEXT("Nonfinite sprint rejected"), Behavior->Validate());
    Behavior->SprintMultiplier = 1.5f;
    Shooter->ConfigureInfantryBehavior(Behavior);
    Shooter->WeaponCooldown = 0.2f;
    Shooter->SetAttackOrder(Target);
    for (int32 I=0; I<20; ++I) { Shooter->ResolveCombat(0.05f); Target->ApplyPendingDamage(); }
    TestEqual(TEXT("Authored 0.2 second cadence yields five shots in twenty fixed steps"), Shooter->GetAmmo(), 25);
    Shooter->CooldownRemaining = 0;
    Shooter->WeaponCooldown = 0.05f;
    Behavior->MagazineCapacity = 2;
    Behavior->ReloadSeconds = 0.2f;
    Shooter->ConfigureInfantryBehavior(Behavior);
    Shooter->SetAttackOrder(Target);
    Shooter->ResolveCombat(0.05f); Target->ApplyPendingDamage();
    Shooter->ResolveCombat(0.05f); Target->ApplyPendingDamage();
    TestEqual(TEXT("Magazine exhausted by actual shots"), Shooter->GetAmmo(), 0);
    const int32 BeforeReload = Target->GetHealth();
    Shooter->ResolveCombat(0.05f);
    TestTrue(TEXT("Empty magazine starts reload"), Shooter->IsReloading());
    for (int32 I=0; I<4; ++I) Shooter->ResolveCombat(0.05f);
    TestEqual(TEXT("Reload does not grant damage"), Target->ApplyPendingDamage(), 0);
    TestEqual(TEXT("Magazine refilled once"), Shooter->GetAmmo(), 2);
    Shooter->ResolveCombat(0.05f); Target->ApplyPendingDamage();
    TestEqual(TEXT("Fire resumes after reload"), Target->GetHealth(), BeforeReload-8);
    Shooter->BeginReload();
    TestTrue(TEXT("Manual partial reload starts"), Shooter->IsReloading());
    TestFalse(TEXT("Double reload rejected"), Shooter->CanReload());
    for (int32 I=0; I<4; ++I) Shooter->ResolveCombat(0.05f);

    auto* Wall = World->SpawnActor<AActor>(FVector(100,0,80), FRotator::ZeroRotator);
    auto* Box = NewObject<UBoxComponent>(Wall);
    Wall->SetRootComponent(Box); Box->SetBoxExtent(FVector(10,60,100));
    Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly); Box->SetCollisionObjectType(ECC_WorldStatic);
    Box->SetCollisionResponseToAllChannels(ECR_Block); Box->RegisterComponent();
    Box->SetWorldLocation(FVector(100,0,80));
    TestFalse(TEXT("Solid wall blocks rifle line of fire"), Shooter->HasLineOfFire(Target));
    const int32 BlockedAmmo = Shooter->GetAmmo();
    Shooter->ResolveCombat(0.05f);
    TestEqual(TEXT("Blocked rifle consumes no ammo"), Shooter->GetAmmo(), BlockedAmmo);
    Wall->Destroy();
    TestTrue(TEXT("Grenade supports hostile target"), Shooter->CanUseGrenade(Target));
    TestFalse(TEXT("Friendly grenade denied"), Shooter->CanUseGrenade(Ally));

    auto* Match = World->SpawnActor<APolitikaPrototypeMatch>();
    TestTrue(TEXT("Validate test setup"), Match->Lifecycle.TransitionTo(MatchPhase::SetupValidated));
    TestTrue(TEXT("Load test setup"), Match->Lifecycle.TransitionTo(MatchPhase::Loading));
    TestTrue(TEXT("Start test setup"), Match->Lifecycle.TransitionTo(MatchPhase::Starting));
    TestTrue(TEXT("Activate test setup"), Match->Lifecycle.TransitionTo(MatchPhase::Active));
    Match->bContentReady = true;
    Match->PlayableMin = FVector2D(-1000,-1000); Match->PlayableMax = FVector2D(1000,1000);
    Match->LocalRuntimePlayerId = 1;
    Match->RuntimeParticipants = {
        {1, 1, FName(TEXT("faction.player")), EPolitikaPrototypeControllerKind::LocalHuman},
        {2, 2, FName(TEXT("faction.opposition")), EPolitikaPrototypeControllerKind::AssignedOpponent}
    };
    Match->UnalignedSystemAuthority = {FName(TEXT("faction.unaligned")), true};
    Match->Units = {Shooter, Target, Ally, SystemInfantry};
    SystemInfantry->ConfigureInfantryBehavior(Behavior);
    Shooter->SetAttackOrder(Target);
    const auto AssertRejectedAbilityIsSideEffectFree = [&](uint32 PlayerId, APolitikaPrototypeUnit* Issuer,
        const Order& Intent, const TCHAR* Label)
    {
        const EPolitikaPrototypeOrder IssuerOrder = Issuer->GetCurrentOrder();
        const int32 IssuerAmmo = Issuer->GetAmmo();
        const float IssuerGrenadeCooldown = Issuer->GrenadeCooldown;
        const float IssuerSprintCooldown = Issuer->SprintCooldown;
        const int32 TargetHealth = Target->GetHealth();
        const int32 EventStart = Match->PublishedEvents.Num();
        Match->SubmitCommand(PlayerId, Issuer, Target, Intent);
        TestEqual(FString::Printf(TEXT("%s is rejected immediately"), Label), Match->PublishedEvents.Num(), EventStart + 1);
        if (Match->PublishedEvents.Num() > EventStart)
        {
            const auto* Outcome = std::get_if<CommandOutcomeEvent>(&Match->PublishedEvents.Last());
            TestNotNull(FString::Printf(TEXT("%s publishes typed outcome"), Label), Outcome);
            if (Outcome)
            {
                TestFalse(FString::Printf(TEXT("%s is not accepted"), Label), Outcome->accepted);
                TestEqual(FString::Printf(TEXT("%s reports ownership rejection"), Label),
                    Outcome->rejection, OrderRejection::NotOwned);
            }
        }
        TestEqual(FString::Printf(TEXT("%s preserves issuer order"), Label), Issuer->GetCurrentOrder(), IssuerOrder);
        TestEqual(FString::Printf(TEXT("%s preserves magazine ammunition"), Label), Issuer->GetAmmo(), IssuerAmmo);
        TestEqual(FString::Printf(TEXT("%s preserves grenade cooldown"), Label), Issuer->GrenadeCooldown, IssuerGrenadeCooldown);
        TestEqual(FString::Printf(TEXT("%s preserves sprint cooldown"), Label), Issuer->SprintCooldown, IssuerSprintCooldown);
        TestEqual(FString::Printf(TEXT("%s preserves target health"), Label), Target->GetHealth(), TargetHealth);
        TestTrue(FString::Printf(TEXT("%s leaves no pending command"), Label), Match->PendingOrders.IsEmpty());
    };
    AssertRejectedAbilityIsSideEffectFree(2, Shooter, GrenadeOrder{Target->GetStableId()}, TEXT("Foreign grenade ability"));
    AssertRejectedAbilityIsSideEffectFree(2, Shooter, ReloadOrder{}, TEXT("Foreign reload ability"));
    AssertRejectedAbilityIsSideEffectFree(2, Shooter, SprintOrder{}, TEXT("Foreign sprint ability"));
    AssertRejectedAbilityIsSideEffectFree(1, SystemInfantry, GrenadeOrder{Target->GetStableId()},
        TEXT("System-owned infantry grenade ability"));
    Match->SubmitCommand(2, Shooter, Target, GrenadeOrder{Target->StableId});
    Match->AdmitPendingOrders();
    TestEqual(TEXT("Wrong owner cannot begin grenade"), Shooter->GrenadeWindup, 0.0f);
    Match->SubmitCommand(1, Shooter, Target, GrenadeOrder{Target->StableId});
    Match->AdmitPendingOrders();
    TestTrue(TEXT("Admitted grenade begins windup"), Shooter->GrenadeWindup > 0);
    Shooter->StopOrder();
    TestEqual(TEXT("Stop cancels unlaunched grenade"), Shooter->GrenadeWindup, 0.0f);
    Match->SubmitCommand(1, Shooter, Target, GrenadeOrder{Target->StableId});
    Match->AdmitPendingOrders();
    for (int32 I=0; I<40; ++I) Shooter->ResolveCombat(0.05f);
    TestEqual(TEXT("Admitted grenade launches after full windup"), Match->InfantryProjectiles.Num(), 1);
    TestFalse(TEXT("Grenade cooldown prevents immediate second launch"), Shooter->CanUseGrenade(Target));
    const int32 PreBlast = Target->GetHealth();
    TestEqual(TEXT("Projectile is not instant damage"), Target->ApplyPendingDamage(), 0);
    for (int32 I=0; I<30; ++I) Match->AdvanceGrenades(0.05f);
    TestEqual(TEXT("One hostile splash impact"), Target->ApplyPendingDamage(), 50);
    TestEqual(TEXT("Friendly splash excluded"), Ally->ApplyPendingDamage(), 0);
    TestEqual(TEXT("Projectile retires once"), Match->InfantryProjectiles.Num(), 0);
    TestEqual(TEXT("Measured blast health"), Target->GetHealth(), PreBlast-50);
    Match->LaunchGrenade(Shooter, Target->GetActorLocation());
    Match->TransitionLifecycle(MatchPhase::Returning);
    TestTrue(TEXT("Leaving active discards projectile state"), Match->InfantryProjectiles.IsEmpty());
    Match->SubmitCommand(1, Shooter, nullptr, SprintOrder{});
    TestEqual(TEXT("Inactive command cannot mutate sprint"), Shooter->SprintRemaining, 0.0f);
    Shooter->SprintRemaining=1;
    Shooter->SprintCooldown=12;
    TestFalse(TEXT("Sprint cooldown prevents retrigger"), Shooter->CanSprint());
    Shooter->SetMoveOrder(EPolitikaPrototypeOrder::Move, FVector(0,200,42), FVector(0,200,42),
        TArray<FVector>{FVector(0,0,42),FVector(0,200,42)});
    Shooter->NavigationMatchGeneration = Match->MatchGeneration;
    Shooter->SimulateMovement(0.05f);
    TestTrue(TEXT("Sprint applies bounded speed multiplier"),
        FMath::IsNearlyEqual(FVector::Dist2D(FVector(0,0,42),Shooter->GetActorLocation()),15.75,0.01));
    Shooter->SprintRemaining = 0;
    Shooter->SetActorLocation(FVector(0,0,42));
    Shooter->SetMoveOrder(EPolitikaPrototypeOrder::AttackMove, FVector(500,0,42), FVector(500,0,42),
        TArray<FVector>{FVector(0,0,42),FVector(500,0,42)});
    Shooter->NavigationMatchGeneration = Match->MatchGeneration;
    Shooter->OrderTarget = Target;
    Shooter->SimulateMovement(0.05f);
    TestTrue(TEXT("Rifle attack-move pauses at a shootable enemy"), Shooter->GetActorLocation().Equals(FVector(0,0,42)));
    Target->PendingDamage = 2000;
    Target->ApplyPendingDamage();
    Shooter->CleanupDeadTarget();
    Shooter->SimulateMovement(0.05f);
    TestTrue(TEXT("Attack-move resumes its retained route after target death"), Shooter->GetActorLocation().X > 0);
    World->DestroyWorld(false);
    return true;
}
#endif
