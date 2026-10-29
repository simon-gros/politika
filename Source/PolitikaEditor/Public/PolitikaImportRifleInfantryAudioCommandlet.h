#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PolitikaImportRifleInfantryAudioCommandlet.generated.h"

/** Imports neutral MP3 clips as SoundWaves, creates random-variation cues, and wires the rifleman profile. */
UCLASS()
class UPolitikaImportRifleInfantryAudioCommandlet final : public UCommandlet
{
    GENERATED_BODY()
public:
    UPolitikaImportRifleInfantryAudioCommandlet();
    virtual int32 Main(const FString& Params) override;
};
