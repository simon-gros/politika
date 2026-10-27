#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PolitikaLocalProfile.generated.h"

/** Local identity only. Preferences, online credentials and campaign snapshots do not belong here. */
UCLASS()
class POLITIKA_API UPolitikaLocalProfile final : public USaveGame
{
    GENERATED_BODY()
public:
    static constexpr int32 CurrentSchema = 1;
    static constexpr const TCHAR* Slot = TEXT("PolitikaLocalProfile");

    /** UE slot storage with a versioned integrity envelope before UObject deserialization. */
    static UPolitikaLocalProfile* LoadVerified();
    bool SaveVerified();

    UPROPERTY(SaveGame)
    int32 SchemaVersion = CurrentSchema;

    UPROPERTY(SaveGame)
    FGuid LocalId;

    UPROPERTY(SaveGame)
    FDateTime CreatedUtc;

    bool IsValidProfile() const { return SchemaVersion == CurrentSchema && LocalId.IsValid(); }
    FString SaveSlotPrefix() const { return TEXT("Politika_") + LocalId.ToString(EGuidFormats::Digits) + TEXT("_"); }
};
