#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PolitikaShellGameMode.generated.h"

UCLASS()
class POLITIKA_API APolitikaShellGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    APolitikaShellGameMode();
};

/** A travel destination for the shell milestone, not an RTS match implementation. */
UCLASS()
class POLITIKA_API APolitikaOfflineGameMode final : public APolitikaShellGameMode
{
    GENERATED_BODY()
};
