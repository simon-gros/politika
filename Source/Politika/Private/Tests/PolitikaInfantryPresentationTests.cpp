#if WITH_DEV_AUTOMATION_TESTS
#include "PolitikaInfantryPresentation.h"
#include "PolitikaPrototypeContent.h"
#include "PolitikaPrototypeGameplay.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/AnimSequence.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaInfantryProfileValidationTest,
    "Politika.Infantry.IncompletePresentationRejected",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPolitikaInfantryProfileValidationTest::RunTest(const FString& Parameters)
{
    auto* Profile = NewObject<UPolitikaInfantryPresentationDefinition>();
    FString Error;
    TestFalse(TEXT("Empty presentation rejected"), Profile->Validate(Error));
    TestTrue(TEXT("Identity diagnostic supplied"), Error.Contains(TEXT("identity")));
    Profile->StableId = TEXT("presentation.test.infantry");
    Profile->SourceDescription = TEXT("Automation fixture");
    TestFalse(TEXT("Missing skeletal mesh rejected"), Profile->Validate(Error));
    TestTrue(TEXT("Missing mesh diagnostic supplied"), Error.Contains(TEXT("skeletal mesh")));
    Profile->SchemaVersion = 2;
    TestFalse(TEXT("Future schema rejected"), Profile->Validate(Error));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaInfantryAuthorityAndDeathTest,
    "Politika.Infantry.CosmeticsDoNotOwnDamageAndDeathCleanup",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPolitikaInfantryAuthorityAndDeathTest::RunTest(const FString& Parameters)
{
    auto* Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
        TEXT("/Game/Politika/Prototypes/Infantry/RifleInfantry/DA_InfantryCatalog.DA_InfantryCatalog"));
    if (!Catalog) return false;
    const auto* Definition = Catalog->FindUnit(TEXT("unit.playtest.rifle_infantry"));
    if (!Definition || !Definition->InfantryPresentation) return false;
    const UWorld::InitializationValues Init = UWorld::InitializationValues().AllowAudioPlayback(false)
        .RequiresHitProxies(false).CreatePhysicsScene(false).CreateNavigation(false).CreateAISystem(false)
        .ShouldSimulatePhysics(false).SetTransactional(false).CreateFXSystem(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("InfantryPresentationAutomation"),
        GetTransientPackage(), false, ERHIFeatureLevel::Num, &Init);
    if (!World) return false;
    auto* Soldier = World->SpawnActor<APolitikaPrototypeUnit>(FVector(0, 0, 42), FRotator::ZeroRotator);
    auto* Target = World->SpawnActor<APolitikaPrototypeUnit>(FVector(100, 0, 42), FRotator::ZeroRotator);
    Soldier->Configure(1, 1, FText::FromString(TEXT("Imported soldier")), 8, 210, 650, 8, 0.2f,
        true, Definition->VisualMesh, Definition->VisualScale, FLinearColor::White);
    Soldier->ConfigureInfantryPresentation(Definition->InfantryPresentation);
    Target->Configure(2, 2, FText::FromString(TEXT("Target")), 100, 1, 650, 8, 0.2f,
        true, Definition->VisualMesh, Definition->VisualScale, FLinearColor::White);
    auto* Cosmetic = Soldier->FindComponentByClass<UPolitikaInfantryPresentationComponent>();
    TestNotNull(TEXT("Presentation component initialized"), Cosmetic->GetVisual());
    Cosmetic->SetComponentTickEnabled(false);
    Cosmetic->GetVisual()->SetVisibility(false);
    Soldier->SetAttackOrder(Target);
    Soldier->ResolveCombat(0.05f);
    TestEqual(TEXT("Damage resolves with animation ticks disabled and mesh hidden"), Target->ApplyPendingDamage(), 8);
    Soldier->ResolveCombat(0.05f);
    TestEqual(TEXT("Cosmetic operations cannot bypass weapon cooldown"), Target->ApplyPendingDamage(), 0);
    Target->SetAttackOrder(Soldier);
    Target->ResolveCombat(0.05f);
    Soldier->ApplyPendingDamage();
    TestFalse(TEXT("Fatal damage is authoritative"), Soldier->IsAlive());
    Soldier->BeginDeathPresentation();
    TestFalse(TEXT("Corpse no longer blocks picking or movement"), Soldier->GetActorEnableCollision());
    TestTrue(TEXT("Corpse has bounded cleanup lifetime"), Soldier->GetLifeSpan() > 0 && Soldier->GetLifeSpan() <= 30);
    TestTrue(TEXT("Death clip is played separately from combat state"),
        Cosmetic->GetVisual()->GetSingleNodeInstance()->GetCurrentAsset() == Definition->InfantryPresentation->Death
        || Cosmetic->GetVisual()->GetSingleNodeInstance()->GetCurrentAsset() == Definition->InfantryPresentation->DeathAlternate);
    const FVector ActorAtDeath = Soldier->GetActorLocation();
    const float VisualAtDeath = Cosmetic->GetVisual()->GetComponentLocation().Z;
    Cosmetic->TickComponent(Definition->InfantryPresentation->CorpseSinkDelaySeconds + 0.5f, LEVELTICK_All, nullptr);
    TestTrue(TEXT("Corpse visual sinks after delay"), Cosmetic->GetVisual()->GetComponentLocation().Z < VisualAtDeath);
    TestTrue(TEXT("Corpse sinking cannot move simulation actor"), Soldier->GetActorLocation().Equals(ActorAtDeath));
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaImportedInfantryValidationTest,
    "Politika.Infantry.ImportedProfileAndCatalog",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPolitikaImportedInfantryValidationTest::RunTest(const FString& Parameters)
{
    auto* Profile = LoadObject<UPolitikaInfantryPresentationDefinition>(nullptr,
        TEXT("/Game/Politika/Prototypes/Infantry/RifleInfantry/DA_RifleInfantryPresentation.DA_RifleInfantryPresentation"));
    TestNotNull(TEXT("Imported presentation exists"), Profile);
    if (!Profile) return false;
    FString Error;
    TestTrue(TEXT("Real skeleton, clips, socket and audio validate"), Profile->Validate(Error));
    auto* Invalid = DuplicateObject<UPolitikaInfantryPresentationDefinition>(Profile, GetTransientPackage());
    Invalid->MuzzleSocket = TEXT("MissingRequiredMuzzle");
    TestFalse(TEXT("Missing required socket rejected"), Invalid->Validate(Error));
    Invalid->MuzzleSocket = Profile->MuzzleSocket;
    Invalid->Fire = nullptr;
    TestFalse(TEXT("Missing required animation rejected"), Invalid->Validate(Error));
    Invalid->Fire = Profile->Fire;
    TestNotNull(TEXT("Imported selection variants wired"), Profile->SelectSound.Get());
    TestNotNull(TEXT("Imported movement variants wired"), Profile->MoveSound.Get());
    TestNotNull(TEXT("Imported rifle variants wired"), Profile->FireSound.Get());
    TestNotNull(TEXT("Movement Foley supports footsteps"), Profile->FootstepSound.Get());
    for (USoundBase* Sound : {Profile->SelectSound.Get(), Profile->MoveSound.Get(), Profile->FireSound.Get()})
        if (Sound) TestTrue(TEXT("Imported audio has finite positive duration"), FMath::IsFinite(Sound->GetDuration()) && Sound->GetDuration() > 0);
    TestNotNull(TEXT("English death voice variants wired"), Profile->DeathSound.Get());
    TestNotNull(TEXT("English attack voice variants wired"), Profile->AttackSound.Get());
    TestNull(TEXT("Unavailable reload animation stays absent"), Profile->ReloadClip.Get());
    TestNull(TEXT("Unavailable grenade animation stays absent"), Profile->GrenadeClip.Get());
    Invalid->FireSound = nullptr;
    TestFalse(TEXT("A missing required audio role rejects the profile"), Invalid->Validate(Error));
    Invalid->FireSound = Profile->FireSound;
    Invalid->CorpseSinkDelaySeconds = 0;
    TestFalse(TEXT("Sinking before death clip ends is rejected"), Invalid->Validate(Error));
    auto* Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
        TEXT("/Game/Politika/Prototypes/Infantry/RifleInfantry/DA_InfantryCatalog.DA_InfantryCatalog"));
    TestNotNull(TEXT("Isolated catalog exists"), Catalog);
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    UPolitikaPrototypeContentValidator::ValidateCatalog(Catalog, Diagnostics);
    TestFalse(TEXT("Imported catalog satisfies ordinary content admission"),
        UPolitikaPrototypeContentValidator::HasErrors(Diagnostics));
    if (Catalog)
    {
        const auto* Map = Catalog->FindMap(TEXT("map.test.rifle_infantry"));
        TestNotNull(TEXT("Isolated map definition resolves"), Map);
        if (Map) TestEqual(TEXT("One soldier and one non-player target"), Map->UnitPlacements.Num(), 2);
    }
    return true;
}
#endif
