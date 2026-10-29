#pragma once
#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PolitikaImportInfantryCommandlet.generated.h"

/** Local placeholder importer; -PromoteDefault explicitly updates the prototype's authored playable placements. */
UCLASS()
class UPolitikaImportInfantryCommandlet final : public UCommandlet
{
    GENERATED_BODY()
public:
    UPolitikaImportInfantryCommandlet();
    virtual int32 Main(const FString& Params) override;
};
