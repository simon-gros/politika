#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "PolitikaLocalPlayer.generated.h"

/** Adds POLITIKA's on-wire compatibility key to the join URL checked by PreLogin. */
UCLASS()
class POLITIKA_API UPolitikaLocalPlayer final : public ULocalPlayer
{
    GENERATED_BODY()
public:
    virtual FString GetGameLoginOptions() const override;
};
