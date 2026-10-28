#pragma once

#include "CoreMinimal.h"
#if WITH_EDITOR
#include "GameFramework/Actor.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Null.h"
#endif

// Shared by the Editor regression and isolated saved fixture authoring. No runtime
// geometry is spawned by the packaged acceptance scenario.
namespace PolitikaNavigationObstacleFixture
{
inline const FVector Start(-700.0, -600.0, 42.0);
inline const FVector Goal(800.0, -600.0, 42.0);
inline const FVector WallLocation(0.0, -600.0, 800.0);
inline const FVector WallScale(2.0, 5.0, 20.0);
inline constexpr const TCHAR* WallTag = TEXT("PolitikaNavigationStaticObstacle");
inline constexpr const TCHAR* Level = TEXT("/Game/Politika/TestFixtures/Navigation/W_StaticObstacleDetour");
inline constexpr const TCHAR* MapId = TEXT("map.test.static_obstacle_detour");
#if !UE_BUILD_SHIPPING
// Inputs only: the coordinator remains the sole accumulator/fixed-step owner.
inline float ScheduleDelta(int32 Schedule, uint64 Update)
{
    switch (Schedule)
    {
        case 1: return 1.0f / 120.0f;
        case 2: return 1.0f / 60.0f;
        case 3: return 1.0f / 30.0f;
        case 4: return Update % 2 == 0 ? 0.0125f : 0.0375f;
        case 5: { constexpr float Values[] = {0.01f, 0.20f, 0.04f}; return Values[Update % 3]; }
        default: return 0.0f;
    }
}
#endif
#if WITH_EDITOR
inline void ExcludeSolidInterior(AActor* Wall)
{
    // Tall triangle surfaces can enclose a disconnected walkable island. The
    // static modifier makes the entire solid blocking footprint non-navigable.
    auto* Modifier = NewObject<UNavModifierComponent>(Wall, TEXT("SolidObstacleNavigation"));
    Wall->AddInstanceComponent(Modifier);
    Modifier->SetAreaClass(UNavArea_Null::StaticClass());
    Modifier->RegisterComponent();
}
#endif
}
