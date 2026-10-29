#if WITH_DEV_AUTOMATION_TESTS

// Politika.Bootstrap.LaunchOptions: the project-owned switch parser under both policies.
// Strict is the only policy a Shipping build can apply; Development is the non-Shipping one.

#include "PolitikaBootstrap.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaLaunchOptionsAutomationTest,
    "Politika.Bootstrap.LaunchOptions",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
    EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaLaunchOptionsAutomationTest::RunTest(const FString& Parameters)
{
    using EPolicy = EPolitikaLaunchSwitchPolicy;
    const TCHAR* const DevelopmentSwitches[] = {
        TEXT("-PolitikaMPRematch=/Game/Maps/RTSVerticalSlice?listen?PolitikaLayout=1v1"),
        TEXT("-PolitikaMPRematch=127.0.0.1:17777"),
        TEXT("-PolitikaMPRematchDelay=3"),
        TEXT("-PolitikaMPRematchAttempts=2"),
        TEXT("-PolitikaMPProbe"),
        TEXT("-politikampprobe"),
        TEXT("-PolitikaTestFixture=MultiplayerSkirmish"),
        TEXT("-PolitikaMPGroundProbe=300:1300/-700:-950"),
    };
    for (const TCHAR* Switch : DevelopmentSwitches)
    {
        const FString Line = FString::Printf(TEXT("127.0.0.1:17777 -windowed %s -nosound"), Switch);
        TestEqual(FString::Printf(TEXT("Development accepts %s"), Switch),
            FPolitikaLaunchOptions::Parse(*Line, EPolicy::Development).Errors.Num(), 0);
        TestEqual(FString::Printf(TEXT("Strict rejects %s"), Switch),
            FPolitikaLaunchOptions::Parse(*Line, EPolicy::Strict).Errors.Num(), 1);
    }

    // The MP-3 dedicated qualification client command line, plus the hostile probe.
    const TCHAR* const ClientLine = TEXT("127.0.0.1:17777 -windowed -ResX=1280 -ResY=720 "
        "-PolitikaMPRematch=127.0.0.1:17777 -PolitikaMPRematchDelay=3 -PolitikaMPRematchAttempts=1 "
        "-PolitikaMPProbe -UserDir=/tmp/user-client-a/ -unattended -nosplash");
    TestTrue(TEXT("Development accepts the multiplayer client line"),
        FPolitikaLaunchOptions::Parse(ClientLine, EPolicy::Development).Errors.IsEmpty());
    TestEqual(TEXT("Strict reports one deduplicated error for the client line"),
        FPolitikaLaunchOptions::Parse(ClientLine, EPolicy::Strict).Errors.Num(), 1);

    // Unknown, valued or empty-valued switches stay rejected under either policy.
    const TCHAR* const Rejected[] = {
        TEXT("-PolitikaMPProbe=1"),
        TEXT("-PolitikaMPRematch"),
        TEXT("-PolitikaMPRematch="),
        TEXT("-PolitikaMPRematchDelay"),
        TEXT("-PolitikaMPRematchAttempts="),
        TEXT("-PolitikaMPRematchTypo=3"),
        TEXT("-PolitikaMPGroundProbe="),
        TEXT("-PolitikaTestFixture="),
        TEXT("-PolitikaOffline=invalid"),
        TEXT("-PolitikaSafeMode=1"),
        TEXT("-PolitikaUnknown"),
        TEXT("--PolitikaMPProbe"),
    };
    for (const TCHAR* Switch : Rejected)
    {
        for (const EPolicy Policy : { EPolicy::Strict, EPolicy::Development })
        {
            TestEqual(FString::Printf(TEXT("%s rejects %s"),
                Policy == EPolicy::Strict ? TEXT("Strict") : TEXT("Development"), Switch),
                FPolitikaLaunchOptions::Parse(Switch, Policy).Errors.Num(), 1);
        }
    }

    for (const EPolicy Policy : { EPolicy::Strict, EPolicy::Development })
    {
        // Ordinary switches and UE-owned flags are unaffected by either policy.
        const FPolitikaLaunchOptions Safe = FPolitikaLaunchOptions::Parse(
            TEXT("-PolitikaSafeMode -PolitikaOffline -nullrhi -log"), Policy);
        TestTrue(TEXT("safe mode and offline are accepted"), Safe.Errors.IsEmpty() && Safe.bSafeMode);

        // A malformed Development switch still cancels a destructive reset.
        const FPolitikaLaunchOptions Reset = FPolitikaLaunchOptions::Parse(
            TEXT("-PolitikaResetUserConfig -PolitikaMPProbe=1"), Policy);
        TestFalse(TEXT("a rejected switch cancels config reset"), Reset.bResetUserConfig);
        TestEqual(TEXT("one rejection is reported"), Reset.Errors.Num(), 1);
    }
    const FPolitikaLaunchOptions DevelopmentReset = FPolitikaLaunchOptions::Parse(
        TEXT("-PolitikaResetUserConfig -PolitikaMPProbe"), EPolicy::Development);
    TestTrue(TEXT("a recognized Development switch keeps a valid reset"),
        DevelopmentReset.Errors.IsEmpty() && DevelopmentReset.bResetUserConfig);

#if UE_BUILD_SHIPPING
    TestEqual(TEXT("Shipping default policy is Strict"),
        FPolitikaLaunchOptions::Parse(TEXT("-PolitikaMPProbe")).Errors.Num(), 1);
#else
    TestTrue(TEXT("non-Shipping default policy is Development"),
        FPolitikaLaunchOptions::Parse(TEXT("-PolitikaMPProbe -PolitikaMPRematchDelay=3")).Errors.IsEmpty());
#endif
    return true;
}

#endif
