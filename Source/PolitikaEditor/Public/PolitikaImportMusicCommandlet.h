#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PolitikaImportMusicCommandlet.generated.h"

/** Import Simon's original Dusk track without rewriting any authored maps. */
UCLASS()
class UPolitikaImportMusicCommandlet final : public UCommandlet
{
    GENERATED_BODY()
public:
    UPolitikaImportMusicCommandlet();
    virtual int32 Main(const FString& Params) override;
};
