#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PolitikaCreateShellCommandlet.generated.h"

/** Generate real binary map assets without Python or manual Level Blueprint startup wiring. */
UCLASS()
class UPolitikaCreateShellCommandlet final : public UCommandlet
{
    GENERATED_BODY()
public:
    UPolitikaCreateShellCommandlet();
    virtual int32 Main(const FString& Params) override;
};
