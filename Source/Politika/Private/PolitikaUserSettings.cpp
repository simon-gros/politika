#include "PolitikaUserSettings.h"

#include "PolitikaBootstrap.h"

void UPolitikaUserSettings::LoadSettings(bool bForceReload)
{
    if (FPolitikaBootstrap::Get().UsesTransientSettings())
    {
        SetToDefaults();
        SetScreenResolution(FIntPoint(1280, 720));
        SetFullscreenMode(EWindowMode::Windowed);
        SetOverallScalabilityLevel(0);
        SetVSyncEnabled(true);
        SetDynamicResolutionEnabled(false);
        SetFrameRateLimit(60);
        bUseHDRDisplayOutput = false;
        // Prevent UE's obsolete-version path from deleting even the temporary config branch.
        UpdateVersion();
        ConfirmVideoMode();
        return;
    }
    Super::LoadSettings(bForceReload);
}

void UPolitikaUserSettings::SaveSettings()
{
    if (!FPolitikaBootstrap::Get().UsesTransientSettings())
    {
        Super::SaveSettings();
    }
}

void UPolitikaUserSettings::ValidateSettings()
{
    // Bootstrap has already detected unsupported persisted versions. Do not use UE's
    // destructive old-version reset here; loading future schemas must not discard user data.
    if (!IsVersionValid())
    {
        UpdateVersion();
    }
    Super::ValidateSettings();
}

void UPolitikaUserSettings::ApplySessionDefaults()
{
    if (FPolitikaBootstrap::Get().UseTransientSettings())
    {
        LoadSettings(true);
        // Separate apply calls deliberately avoid ApplySettings(), which implicitly saves.
        ApplyResolutionSettings(false);
        ApplyNonResolutionSettings();
        UE_LOG(LogPolitikaSettings, Display, TEXT("Conservative session settings applied without saving preferences."));
    }
}
