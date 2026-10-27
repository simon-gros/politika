#include "PolitikaShellGameMode.h"
#include "PolitikaShellPlayerController.h"

APolitikaShellGameMode::APolitikaShellGameMode()
{
    PlayerControllerClass = APolitikaShellPlayerController::StaticClass();
    DefaultPawnClass = nullptr;
    HUDClass = nullptr;
}
