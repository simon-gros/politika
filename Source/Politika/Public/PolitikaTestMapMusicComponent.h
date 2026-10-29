#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "PolitikaTestMapMusicComponent.generated.h"

/** World-owned, non-spatial background music for the prototype test maps only. */
UCLASS()
class POLITIKA_API UPolitikaTestMapMusicComponent final : public UAudioComponent
{
    GENERATED_BODY()
public:
    UPolitikaTestMapMusicComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
