#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PolitikaShellPlayerController.generated.h"

class SWidget;

UCLASS()
class POLITIKA_API APolitikaShellPlayerController final : public APlayerController
{
    GENERATED_BODY()
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    TSharedPtr<SWidget> ShellWidget;
    TSharedPtr<SWidget> PrimaryAction;
    bool bResetConfirmation = false;
    TSharedRef<SWidget> BuildShell();
};
