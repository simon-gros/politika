#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/ActorComponent.h"
#include "PolitikaInfantryPresentation.generated.h"

class USkeletalMesh;
class USkeletalMeshComponent;
class UStaticMesh;
class UAnimSequence;
class USoundBase;

/** Shared cosmetic defaults. No animation, audio or VFX callback owns gameplay damage. */
UCLASS(BlueprintType)
class POLITIKA_API UPolitikaInfantryPresentationDefinition final : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) int32 SchemaVersion = 1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FName StableId;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FString SourceDescription;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) bool bPlaytestPlaceholder = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) bool bRequireAudio = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) bool bRequireExtendedPlaytestMedia = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<USkeletalMesh> Mesh;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UStaticMesh> WeaponMesh;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FName WeaponSocket = NAME_None;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UAnimSequence> Idle;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UAnimSequence> Move;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UAnimSequence> Fire;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UAnimSequence> Death;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UAnimSequence> ReloadClip;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UAnimSequence> GrenadeClip;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UAnimSequence> ReadyClip;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UAnimSequence> SelectClip;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UAnimSequence> DeselectClip;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UAnimSequence> DeathAlternate;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UAnimSequence> DeathBlast;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FName MuzzleSocket;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FVector MeshScale = FVector::OneVector;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FRotator MeshRotation = FRotator::ZeroRotator;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FVector MeshOffsetCm = FVector(0, 0, -42);
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float LocomotionReferenceSpeed = 210.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float CorpseLifetimeSeconds = 4.5f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float CorpseSinkDelaySeconds = 2.5f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float CorpseSinkSpeedCmPerSecond = 30;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<USoundBase> SelectSound;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<USoundBase> MoveSound;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<USoundBase> AttackSound;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<USoundBase> FireSound;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<USoundBase> FootstepSound;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<USoundBase> DeathSound;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<USoundBase> ReloadSound;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<USoundBase> GrenadeSound;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<USoundBase> DeathBlastSound;

    bool Validate(FString& OutError) const;
};

/** Presentation-only state machine; movement and shots are supplied by the existing simulation. */
UCLASS()
class POLITIKA_API UPolitikaInfantryPresentationComponent final : public UActorComponent
{
    GENERATED_BODY()
public:
    UPolitikaInfantryPresentationComponent();
    void Initialize(UPolitikaInfantryPresentationDefinition* InDefinition);
    void Acknowledge(FName Event);
    void Shot(const FVector& Target, float Cooldown);
    void Die(bool bBlast = false);
    void SetSelected(bool bSelected);
    void CancelGrenade();
    void Reload(float Seconds);
    void Grenade(float Seconds);
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Tick) override;
    USkeletalMeshComponent* GetVisual() const { return Visual; }
private:
    void Play(UAnimSequence* Clip, bool bLoop);
    void Sound(USoundBase* Cue, FName Event);
    UPROPERTY(Transient) TObjectPtr<UPolitikaInfantryPresentationDefinition> Definition;
    UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> Visual;
    UPROPERTY(Transient) TObjectPtr<class UStaticMeshComponent> WeaponVisual;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> CurrentClip;
    FVector PreviousPosition = FVector::ZeroVector;
    float FireRemaining = 0;
    float VoiceCooldown = 0;
    float FootstepRemaining = 0;
    float MovingRemaining = 0;
    float LastMoveSpeed = 0;
    float ActionRemaining = 0;
    float DeathElapsed = 0;
    float MovementWindow = 0;
    float MovementDistance = 0;
    bool bSelected = false;
    bool bDead = false;
};
