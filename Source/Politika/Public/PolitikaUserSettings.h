#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "PolitikaUserSettings.generated.h"

UCLASS(Config=GameUserSettings)
class POLITIKA_API UPolitikaUserSettings final : public UGameUserSettings
{
    GENERATED_BODY()
public:
    virtual void LoadSettings(bool bForceReload = false) override;
    virtual void SaveSettings() override;
    virtual void ValidateSettings() override;
    void ApplySessionDefaults();
};
