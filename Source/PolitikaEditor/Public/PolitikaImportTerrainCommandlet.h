#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PolitikaImportTerrainCommandlet.generated.h"

/** Explicit, additive CC0 terrain import into an existing prototype map. */
UCLASS()
class UPolitikaImportTerrainCommandlet final : public UCommandlet
{
    GENERATED_BODY()
public:
    UPolitikaImportTerrainCommandlet();
    virtual int32 Main(const FString& Params) override;
};
