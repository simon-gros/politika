#pragma once

#include "CoreMinimal.h"

namespace PolitikaNavigationFailureFixture
{
inline const FVector Start(-700.0, -600.0, 42.0);
inline const FVector Goal(800.0, -600.0, 42.0);
inline const FVector BarrierLocation(0.0, 0.0, 800.0);
inline const FVector BarrierScale(2.0, 100.0, 20.0);
inline constexpr const TCHAR* BarrierTag = TEXT("PolitikaNavigationDisconnectedBarrier");
inline constexpr const TCHAR* Level = TEXT("/Game/Politika/TestFixtures/Navigation/W_DisconnectedNavigation");
inline constexpr const TCHAR* Definition = TEXT("/Game/Politika/TestFixtures/Navigation/DA_DisconnectedNavigation");
inline constexpr const TCHAR* MapId = TEXT("map.test.disconnected_navigation");
}

namespace PolitikaUnprojectableDestinationFixture
{
inline const FVector Start(-700.0, -600.0, 42.0);
inline const FVector Goal(1300.0, 1300.0, -14.0);
inline const FVector ExclusionLocation(1300.0, 1300.0, 42.0);
inline const FVector ExclusionExtent(300.0, 300.0, 100.0);
inline constexpr const TCHAR* ExclusionTag = TEXT("PolitikaNavigationUnprojectableExclusion");
inline constexpr const TCHAR* Level = TEXT("/Game/Politika/TestFixtures/Navigation/W_UnprojectableDestination");
inline constexpr const TCHAR* Definition = TEXT("/Game/Politika/TestFixtures/Navigation/DA_UnprojectableDestination");
inline constexpr const TCHAR* MapId = TEXT("map.test.unprojectable_destination");
}

namespace PolitikaProjectionDisplacementFixture
{
inline const FVector Start(-700.0, -600.0, 42.0);
inline const FVector Goal(1300.0, 1300.0, 42.0);
inline const FVector PlatformLocation(1300.0, 1300.0, 190.0);
inline const FVector PlatformScale(4.0, 4.0, 0.2);
inline const FVector RampLocation(1300.0, 150.0, 93.0);
inline const FVector RampScale(2.0, 19.0, 0.2);
inline constexpr float RampAngleDegrees = -6.45f;
inline const FVector ExclusionLocation(1300.0, 1300.0, 42.0);
inline const FVector ExclusionExtent(300.0, 300.0, 100.0);
inline constexpr const TCHAR* PlatformTag = TEXT("PolitikaProjectionDisplacementGround");
inline constexpr const TCHAR* RampTag = TEXT("PolitikaProjectionDisplacementRamp");
inline constexpr const TCHAR* ExclusionTag = TEXT("PolitikaProjectionDisplacementLowGroundExclusion");
inline constexpr const TCHAR* Level = TEXT("/Game/Politika/TestFixtures/Navigation/W_ProjectionDisplacement");
inline constexpr const TCHAR* Definition = TEXT("/Game/Politika/TestFixtures/Navigation/DA_ProjectionDisplacement");
inline constexpr const TCHAR* MapId = TEXT("map.test.projection_displacement");
}
