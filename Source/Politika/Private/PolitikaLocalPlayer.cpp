#include "PolitikaLocalPlayer.h"

#include "PolitikaBootstrap.h"

FString UPolitikaLocalPlayer::GetGameLoginOptions() const
{
    // network_protocol gates admission; the game version is diagnostic only.
    return FString::Printf(TEXT("PolitikaProtocol=%lld?PolitikaGame=%s"),
        FPolitikaBootstrap::Get().NetworkProtocol(), *FPolitikaBootstrap::Get().GameVersion());
}
