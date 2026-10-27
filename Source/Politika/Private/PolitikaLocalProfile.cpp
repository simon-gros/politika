#include "PolitikaLocalProfile.h"

#include "Kismet/GameplayStatics.h"
#include "Misc/SecureHash.h"

namespace
{
    constexpr uint8 Signature[] = {'P', 'K', 'P', 'R', 'O', 'F', '0', '1'};
    constexpr int32 DigestSize = 20;
    constexpr int32 HeaderSize = sizeof(Signature) + DigestSize;
    constexpr int32 MaximumProfileBytes = 1024 * 1024;
}

UPolitikaLocalProfile* UPolitikaLocalProfile::LoadVerified()
{
    TArray<uint8> Stored;
    if (!UGameplayStatics::LoadDataFromSlot(Stored, Slot, 0) ||
        Stored.Num() <= HeaderSize || Stored.Num() > MaximumProfileBytes ||
        FMemory::Memcmp(Stored.GetData(), Signature, sizeof(Signature)) != 0)
    {
        return nullptr;
    }
    // Corruption detection, not an authentication/security boundary. UE's raw SaveGame
    // reader can assert on arbitrary damaged headers, so never pass unchecked bytes to it.
    uint8 Digest[DigestSize];
    FSHA1::HashBuffer(Stored.GetData() + HeaderSize, Stored.Num() - HeaderSize, Digest);
    if (FMemory::Memcmp(Digest, Stored.GetData() + sizeof(Signature), DigestSize) != 0)
    {
        return nullptr;
    }
    TArray<uint8> Payload;
    Payload.Append(Stored.GetData() + HeaderSize, Stored.Num() - HeaderSize);
    return Cast<UPolitikaLocalProfile>(UGameplayStatics::LoadGameFromMemory(Payload));
}

bool UPolitikaLocalProfile::SaveVerified()
{
    TArray<uint8> Payload;
    if (!IsValidProfile() || !UGameplayStatics::SaveGameToMemory(this, Payload) ||
        Payload.IsEmpty() || Payload.Num() > MaximumProfileBytes - HeaderSize)
    {
        return false;
    }
    uint8 Digest[DigestSize];
    FSHA1::HashBuffer(Payload.GetData(), Payload.Num(), Digest);
    TArray<uint8> Stored;
    Stored.Append(Signature, sizeof(Signature));
    Stored.Append(Digest, DigestSize);
    Stored.Append(Payload);
    return UGameplayStatics::SaveDataToSlot(Stored, Slot, 0);
}
