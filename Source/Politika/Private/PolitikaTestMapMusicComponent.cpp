#include "PolitikaTestMapMusicComponent.h"

#include "Engine/World.h"
#include "PolitikaBootstrap.h"
#include "Sound/SoundWave.h"

UPolitikaTestMapMusicComponent::UPolitikaTestMapMusicComponent()
{
    bAutoActivate = false;
    bAutoDestroy = false;
    bStopWhenOwnerDestroyed = true;
    bAllowSpatialization = false;
    bIsUISound = true;
    VolumeMultiplier = 0.2f;
}

void UPolitikaTestMapMusicComponent::BeginPlay()
{
    Super::BeginPlay();
    if (!GetWorld() || GetWorld()->GetNetMode() == NM_DedicatedServer) return;
    USoundWave* Music = LoadObject<USoundWave>(nullptr, TEXT("/Game/Politika/Audio/Music/SW_Dusk.SW_Dusk"));
    if (!Music)
    {
        UE_LOG(LogPolitikaStartup, Warning, TEXT("TEST_MAP_MUSIC_MISSING track=Dusk"));
        return;
    }
    SetSound(Music);
    Play();
    UE_LOG(LogPolitikaStartup, Display, TEXT("TEST_MAP_MUSIC_START track=Dusk volume=%.2f looping=%d playing=%d"),
        VolumeMultiplier, Music->bLooping, IsPlaying());
}

void UPolitikaTestMapMusicComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Stop();
    UE_LOG(LogPolitikaStartup, Display, TEXT("TEST_MAP_MUSIC_STOP track=Dusk playing=%d"), IsPlaying());
    Super::EndPlay(EndPlayReason);
}
