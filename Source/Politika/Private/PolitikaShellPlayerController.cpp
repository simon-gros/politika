#include "PolitikaShellPlayerController.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/App.h"
#include "PolitikaBootstrap.h"
#include "PolitikaGameInstance.h"
#include "PolitikaShellGameMode.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SSafeZone.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "PolitikaShell"

void APolitikaShellPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController() || GetNetMode() == NM_DedicatedServer || !FApp::CanEverRender())
    {
        FPolitikaBootstrap::Get().LogEvent(TEXT("ui_skipped"), TEXT("Ready"), TEXT("no_local_renderer"));
        return;
    }
    UGameViewportClient* Viewport = GetWorld()->GetGameViewport();
    if (!Viewport || !GetGameInstance<UPolitikaGameInstance>())
    {
        FPolitikaBootstrap::Get().ReportFatal(TEXT("ShellViewportUnavailable"),
            LOCTEXT("ViewportUnavailable", "The shell viewport or game instance is unavailable. Check the project Maps & Modes configuration."), TEXT("Ready"));
        return;
    }
    ShellWidget = BuildShell();
    Viewport->AddViewportWidgetContent(ShellWidget.ToSharedRef(), 10);
    bShowMouseCursor = true;
    FInputModeUIOnly Input;
    Input.SetWidgetToFocus(PrimaryAction);
    Input.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(Input);
    FSlateApplication::Get().SetKeyboardFocus(PrimaryAction, EFocusCause::SetDirectly);
    FPolitikaBootstrap::Get().LogEvent(TEXT("ui_ready"), TEXT("Ready"), TEXT("ok"), TEXT("None"),
        GetWorld()->GetAuthGameMode<APolitikaOfflineGameMode>() ? TEXT("offline_sandbox") : TEXT("main_menu"));
}

void APolitikaShellPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
    if (ShellWidget && GetWorld() && GetWorld()->GetGameViewport())
    {
        GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(ShellWidget.ToSharedRef());
    }
    ShellWidget.Reset();
    PrimaryAction.Reset();
    Super::EndPlay(Reason);
}

TSharedRef<SWidget> APolitikaShellPlayerController::BuildShell()
{
    const TWeakObjectPtr<UPolitikaGameInstance> Game = GetGameInstance<UPolitikaGameInstance>();
    const bool bSandbox = GetWorld()->GetAuthGameMode<APolitikaOfflineGameMode>() != nullptr;
    const FLinearColor Ink(0.84f, 0.87f, 0.79f);
    const FLinearColor Muted(0.52f, 0.61f, 0.55f);

    TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);
    Content->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
    [
        SNew(STextBlock).Text(LOCTEXT("Eyebrow", "STRATEGY / YUGOSLAVIA / 1980s"))
        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 12)).ColorAndOpacity(Muted)
    ];
    Content->AddSlot().AutoHeight()
    [
        SNew(STextBlock).Text(LOCTEXT("Title", "POLITIKA"))
        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 42)).ColorAndOpacity(Ink)
    ];
    Content->AddSlot().AutoHeight().Padding(0, 10, 0, 20)
    [
        SNew(STextBlock).AutoWrapText(true).ColorAndOpacity(Ink)
        .Text(bSandbox ? LOCTEXT("SandboxIntro", "Offline sandbox is open.\nThis shell milestone provides session entry and return; RTS simulation, campaign and skirmish are not implemented yet.")
            : LOCTEXT("MenuIntro", "Local command centre\nNo account or network connection required."))
    ];
    Content->AddSlot().AutoHeight().Padding(0, 0, 0, 16)
    [
        SNew(STextBlock).AutoWrapText(true).ColorAndOpacity(Muted)
        .Text_Lambda([Game]
        {
            const bool bTemporary = !Game.IsValid() || !Game->HasPersistentProfile();
            const bool bSafe = FPolitikaBootstrap::Get().UsesTransientSettings();
            return FText::Format(LOCTEXT("Status", "{0}  ·  {1}"),
                bTemporary ? LOCTEXT("Guest", "Temporary offline profile — progress will not persist") : LOCTEXT("Local", "Local profile ready"),
                bSafe ? LOCTEXT("Safe", "Temporary display settings") : LOCTEXT("Normal", "Normal display settings"));
        })
    ];

    TSharedRef<SButton> Start = SNew(SButton).ContentPadding(FMargin(18, 14))
        .OnClicked_Lambda([this, Game, bSandbox]
        {
            if (Game.IsValid())
            {
                if (bSandbox) { Game->ReturnToMenu(this); }
                else { Game->OpenOfflineSandbox(this); }
            }
            return FReply::Handled();
        })
        [ SNew(STextBlock).Text(bSandbox ? LOCTEXT("Return", "Return to main menu") : LOCTEXT("Start", "Enter offline sandbox")) ];
    PrimaryAction = Start;
    Content->AddSlot().AutoHeight().Padding(0, 0, 0, 12)[Start];

    Content->AddSlot().AutoHeight().Padding(0, 0, 0, 12)
    [
        SNew(SBorder).Padding(16).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor(0.10f, 0.08f, 0.04f))
        .Visibility_Lambda([Game] { return Game.IsValid() && !Game->Issues().IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed; })
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
            [ SNew(STextBlock).Text(LOCTEXT("Recovery", "Startup recovery — offline entry is still available")).AutoWrapText(true) ]
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(STextBlock).AutoWrapText(true).Text_Lambda([Game]
                {
                    TArray<FText> Lines;
                    if (Game.IsValid())
                    {
                        for (const FPolitikaStartupIssue& Issue : Game->Issues())
                        {
                            Lines.Add(FText::Format(LOCTEXT("Issue", "[{0}] {1}"), FText::FromString(Issue.Code), Issue.Message));
                        }
                    }
                    return FText::Join(FText::FromString(TEXT("\n\n")), Lines);
                })
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 0)
            [
                SNew(SButton).ContentPadding(10).OnClicked_Lambda([Game]
                {
                    if (Game.IsValid()) { Game->RetryProfile(); }
                    return FReply::Handled();
                })
                [ SNew(STextBlock).Text(LOCTEXT("Retry", "Retry local profile")) ]
            ]
        ]
    ];

    Content->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
    [
        SNew(SButton).ContentPadding(10).OnClicked_Lambda([Game]
        {
            if (Game.IsValid()) { Game->EnableSessionSafeMode(); }
            return FReply::Handled();
        })
        [ SNew(STextBlock).Text(LOCTEXT("SafeButton", "Use safe display settings for this session")) ]
    ];
    Content->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
    [
        SNew(SButton).ContentPadding(10).OnClicked_Lambda([this, Game]
        {
            if (bResetConfirmation && Game.IsValid())
            {
                Game->ResetSettings();
                bResetConfirmation = false;
            }
            else { bResetConfirmation = true; }
            return FReply::Handled();
        })
        [ SNew(STextBlock).AutoWrapText(true).Text_Lambda([this]
        {
            return bResetConfirmation ? LOCTEXT("ConfirmReset", "Confirm: back up and reset display settings (saves are kept)")
                : LOCTEXT("Reset", "Back up and reset display settings…");
        }) ]
    ];
    Content->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
    [
        SNew(SButton).ContentPadding(10)
        .Visibility_Lambda([this] { return bResetConfirmation ? EVisibility::Visible : EVisibility::Collapsed; })
        .OnClicked_Lambda([this] { bResetConfirmation = false; return FReply::Handled(); })
        [ SNew(STextBlock).Text(LOCTEXT("CancelReset", "Cancel reset")) ]
    ];
    Content->AddSlot().AutoHeight().Padding(0, 0, 0, 16)
    [
        SNew(SButton).ContentPadding(10).OnClicked_Lambda([this]
        {
            FPolitikaBootstrap::Get().LogEvent(TEXT("quit_requested"), TEXT("Shutdown"));
            UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
            return FReply::Handled();
        })
        [ SNew(STextBlock).Text(LOCTEXT("Quit", "Exit")) ]
    ];
    Content->AddSlot().AutoHeight()
    [
        SNew(SExpandableArea).InitiallyCollapsed(true)
        .HeaderContent()[ SNew(STextBlock).Text(LOCTEXT("Details", "Startup diagnostics")) ]
        .BodyContent()
        [ SNew(STextBlock).AutoWrapText(true).ColorAndOpacity(Muted).Text_Lambda([Game]
        { return Game.IsValid() ? Game->Diagnostics() : FText::GetEmpty(); }) ]
    ];

    // SGameLayerManager already applies the project's DPI curve to viewport content.
    // A second SDPIScaler here squares the scale (9x at 4K) and clips the menu.
    return SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor(0.014f, 0.023f, 0.020f)).Padding(0)
        [
            SNew(SSafeZone).HAlign(HAlign_Center).VAlign(VAlign_Center)
            [
                SNew(SBox).MaxDesiredWidth(740)
                [ SNew(SScrollBox) + SScrollBox::Slot().Padding(32)[Content] ]
            ]
        ];
}

#undef LOCTEXT_NAMESPACE
