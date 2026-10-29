#include "PolitikaInfantryPresentation.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "DrawDebugHelpers.h"
#include "PolitikaBootstrap.h"

bool UPolitikaInfantryPresentationDefinition::Validate(FString& OutError) const
{
    OutError.Reset();
    if (SchemaVersion != 1 || StableId.IsNone() || SourceDescription.IsEmpty())
        OutError = TEXT("Presentation identity, supported schema and provenance are required.");
    else if (!Mesh || !Mesh->GetSkeleton())
        OutError = TEXT("A skeletal mesh with a skeleton is required.");
    else if (!Idle || !Move || !Fire || !Death)
        OutError = TEXT("Idle, move, fire and death animation sequences are required.");
    else if (Idle->GetSkeleton() != Mesh->GetSkeleton() || Move->GetSkeleton() != Mesh->GetSkeleton()
        || Fire->GetSkeleton() != Mesh->GetSkeleton() || Death->GetSkeleton() != Mesh->GetSkeleton())
        OutError = TEXT("Every clip must use the presentation mesh skeleton.");
    else if (MuzzleSocket.IsNone() || (!Mesh->FindSocket(MuzzleSocket)
        && Mesh->GetRefSkeleton().FindBoneIndex(MuzzleSocket) == INDEX_NONE))
        OutError = TEXT("The required muzzle socket/bone does not exist.");
    else if (WeaponMesh && (WeaponSocket.IsNone() || (!Mesh->FindSocket(WeaponSocket)
        && Mesh->GetRefSkeleton().FindBoneIndex(WeaponSocket) == INDEX_NONE)))
        OutError = TEXT("A weapon mesh requires a valid weapon attachment socket/bone.");
    else if (bRequireAudio && (!SelectSound || !MoveSound || !AttackSound || !FireSound || !FootstepSound || !DeathSound))
        OutError = TEXT("The profile requires selection, move, attack, fire, footstep and death audio.");
    else if (MeshScale.ContainsNaN() || MeshScale.GetMin() <= 0 || MeshOffsetCm.ContainsNaN()
        || MeshRotation.ContainsNaN() || !FMath::IsFinite(LocomotionReferenceSpeed)
        || LocomotionReferenceSpeed <= 0 || !FMath::IsFinite(CorpseLifetimeSeconds)
        || CorpseLifetimeSeconds < Death->GetPlayLength() || CorpseLifetimeSeconds > 30)
        OutError = TEXT("Transforms, locomotion speed and bounded corpse lifetime must be valid.");
    if (OutError.IsEmpty())
    {
        const UAnimSequence* RequiredClips[] = {Idle, Move, Fire, Death};
        for (const auto* Clip : RequiredClips)
            if (!FMath::IsFinite(Clip->GetPlayLength()) || Clip->GetPlayLength() <= 0)
                OutError = TEXT("Required animation clips must have positive finite duration.");
        const UAnimSequence* OptionalClips[] = {ReloadClip, GrenadeClip, ReadyClip, SelectClip, DeselectClip, DeathAlternate, DeathBlast};
        for (const auto* Clip : OptionalClips)
            if (bRequireExtendedPlaytestMedia && !Clip)
                OutError = TEXT("The complete playtest profile requires every extended animation clip.");
            else if (Clip && (Clip->GetSkeleton() != Mesh->GetSkeleton() || !FMath::IsFinite(Clip->GetPlayLength()) || Clip->GetPlayLength() <= 0))
                OutError = TEXT("Extended animation clips must be non-empty and use the shared skeleton.");
        const USoundBase* Cues[] = {SelectSound, MoveSound, AttackSound, FireSound, FootstepSound, DeathSound,
            ReloadSound, GrenadeSound, DeathBlastSound};
        for (const auto* Cue : Cues)
            if (bRequireExtendedPlaytestMedia && !Cue)
                OutError = TEXT("The complete playtest profile requires all nine sound roles.");
            else if (Cue && (!FMath::IsFinite(Cue->GetDuration()) || Cue->GetDuration() <= 0))
                OutError = TEXT("Sound roles must have positive finite duration.");
        const UAnimSequence* DeathClips[] = {Death, DeathAlternate, DeathBlast};
        for (const auto* Clip : DeathClips)
            if (Clip && Clip->GetPlayLength() > CorpseSinkDelaySeconds)
                OutError = TEXT("Corpse sinking must wait until the death animation completes.");
        if (!FMath::IsFinite(CorpseSinkDelaySeconds) || CorpseSinkDelaySeconds < 0
            || CorpseSinkDelaySeconds > CorpseLifetimeSeconds || !FMath::IsFinite(CorpseSinkSpeedCmPerSecond)
            || CorpseSinkSpeedCmPerSecond < 0) OutError = TEXT("Corpse sinking policy must be finite and bounded.");
    }
    return OutError.IsEmpty();
}

UPolitikaInfantryPresentationComponent::UPolitikaInfantryPresentationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPolitikaInfantryPresentationComponent::Initialize(UPolitikaInfantryPresentationDefinition* InDefinition)
{
    Definition = InDefinition;
    FString Error;
    if (!Definition || !Definition->Validate(Error)) return;
    AActor* Owner = GetOwner();
    Visual = NewObject<USkeletalMeshComponent>(Owner, TEXT("InfantryVisual"));
    Owner->AddInstanceComponent(Visual);
    Visual->SetupAttachment(Owner->GetRootComponent());
    Visual->SetAbsolute(false, false, true);
    Visual->SetRelativeScale3D(Definition->MeshScale);
    Visual->SetRelativeLocation(Definition->MeshOffsetCm / Owner->GetActorScale3D());
    Visual->SetRelativeRotation(Definition->MeshRotation);
    Visual->SetSkeletalMesh(Definition->Mesh);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Visual->SetCanEverAffectNavigation(false);
    Visual->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    Visual->RegisterComponent();
    if (Definition->WeaponMesh)
    {
        WeaponVisual = NewObject<UStaticMeshComponent>(Owner, TEXT("InfantryWeaponVisual"));
        Owner->AddInstanceComponent(WeaponVisual);
        WeaponVisual->SetupAttachment(Visual, Definition->WeaponSocket);
        WeaponVisual->SetStaticMesh(Definition->WeaponMesh);
        WeaponVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponVisual->SetCanEverAffectNavigation(false);
        WeaponVisual->RegisterComponent();
    }
    PreviousPosition = Owner->GetActorLocation();
    SetComponentTickEnabled(true);
    Play(Definition->Idle, true);
    UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_PRESENTATION_READY actor=%s profile=%s mesh=%s skeleton=%s muzzle=%s"),
        *Owner->GetName(), *Definition->StableId.ToString(), *Definition->Mesh->GetPathName(),
        *Definition->Mesh->GetSkeleton()->GetPathName(), *Definition->MuzzleSocket.ToString());
}

void UPolitikaInfantryPresentationComponent::Play(UAnimSequence* Clip, bool bLoop)
{
    if (!Visual || !Clip || CurrentClip == Clip) return;
    CurrentClip = Clip;
    Visual->PlayAnimation(Clip, bLoop);
    UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_ANIMATION actor=%s clip=%s loop=%d"),
        *GetOwner()->GetName(), *Clip->GetName(), bLoop ? 1 : 0);
}

void UPolitikaInfantryPresentationComponent::Sound(USoundBase* Cue, FName Event)
{
    if (!Cue) return;
    UGameplayStatics::PlaySoundAtLocation(this, Cue, GetOwner()->GetActorLocation(), 0.65f);
    UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_AUDIO actor=%s event=%s sound=%s duration=%.3f"),
        *GetOwner()->GetName(), *Event.ToString(), *Cue->GetPathName(), Cue->GetDuration());
}

namespace
{
// One acknowledgement voice per world at a time: a box selection or group order would
// otherwise play every member's line at once.
bool ClaimAcknowledgementVoice(const UWorld* World)
{
    static TWeakObjectPtr<const UWorld> LastWorld;
    static double LastSeconds = 0.0;
    const double Now = World ? World->GetRealTimeSeconds() : 0.0;
    if (LastWorld.Get() == World && Now - LastSeconds < 0.4) return false;
    LastWorld = World;
    LastSeconds = Now;
    return true;
}
}

void UPolitikaInfantryPresentationComponent::Acknowledge(FName Event)
{
    if (!Definition || bDead || VoiceCooldown > 0 || !ClaimAcknowledgementVoice(GetWorld())) return;
    USoundBase* Cue = Event == TEXT("select") ? Definition->SelectSound.Get()
        : Event == TEXT("move") ? Definition->MoveSound.Get() : Definition->AttackSound.Get();
    Sound(Cue, Event);
    VoiceCooldown = 0.75f;
}

void UPolitikaInfantryPresentationComponent::Shot(const FVector& Target, float Cooldown)
{
    if (!Definition || !Visual || bDead) return;
    const FVector Direction = Target - GetOwner()->GetActorLocation();
    Visual->SetWorldRotation(FRotator(0, Direction.Rotation().Yaw, 0) + Definition->MeshRotation);
    FireRemaining = FMath::Max(0.2f, Cooldown * 0.95f);
    Play(Definition->Fire, true);
    Sound(Definition->FireSound, TEXT("fire"));
#if !UE_BUILD_SHIPPING
    const FVector Muzzle = Visual->GetSocketLocation(Definition->MuzzleSocket);
    DrawDebugLine(GetWorld(), Muzzle, Target, FColor(255, 215, 90), false, 0.08f, 0, 2.0f);
    DrawDebugSphere(GetWorld(), Muzzle, 7, 6, FColor::Yellow, false, 0.06f);
    UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_SHOT actor=%s muzzle=%s target=%s"),
        *GetOwner()->GetName(), *Muzzle.ToString(), *Target.ToString());
#endif
}

void UPolitikaInfantryPresentationComponent::SetSelected(bool bInSelected)
{
    bSelected = bInSelected;
    if (!Definition || bDead || ActionRemaining > 0) return;
    UAnimSequence* Transition = bSelected ? Definition->SelectClip.Get() : Definition->DeselectClip.Get();
    if (Transition) { Play(Transition, false); ActionRemaining = Transition->GetPlayLength(); }
}

void UPolitikaInfantryPresentationComponent::Reload(float Seconds)
{
    if (!Definition || !Visual || bDead) return;
    ActionRemaining = Seconds;
    if (Definition->ReloadClip)
    {
        CurrentClip = nullptr;
        Play(Definition->ReloadClip, false);
        Visual->SetPlayRate(Definition->ReloadClip->GetPlayLength() / FMath::Max(Seconds, 0.01f));
    }
    Sound(Definition->ReloadSound, TEXT("reload"));
}

void UPolitikaInfantryPresentationComponent::Grenade(float Seconds)
{
    if (!Definition || !Visual || bDead) return;
    ActionRemaining = Seconds;
    if (Seconds <= 0.2f) { Sound(Definition->GrenadeSound, TEXT("grenade_launch")); return; }
    if (Definition->GrenadeClip)
    {
        CurrentClip = nullptr; Play(Definition->GrenadeClip, false);
        Visual->SetPlayRate(Definition->GrenadeClip->GetPlayLength() / Seconds);
    }
}

void UPolitikaInfantryPresentationComponent::CancelGrenade()
{
    if (Definition && CurrentClip == Definition->GrenadeClip)
    {
        ActionRemaining = 0;
        CurrentClip = nullptr;
    }
}

void UPolitikaInfantryPresentationComponent::Die(bool bBlast)
{
    if (!Definition || bDead) return;
    bDead = true;
    UAnimSequence* Clip = bBlast && Definition->DeathBlast ? Definition->DeathBlast.Get()
        : (GetOwner()->GetUniqueID() % 2 && Definition->DeathAlternate ? Definition->DeathAlternate.Get() : Definition->Death.Get());
    Play(Clip, false);
    if (Visual) Visual->SetPlayRate(1);
    Sound(bBlast && Definition->DeathBlastSound ? Definition->DeathBlastSound.Get() : Definition->DeathSound.Get(), TEXT("death"));
    GetOwner()->SetLifeSpan(Definition->CorpseLifetimeSeconds);
}

void UPolitikaInfantryPresentationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* Tick)
{
    Super::TickComponent(DeltaTime, TickType, Tick);
    if (!Definition || !Visual) return;
    if (bDead)
    {
        const float Before = FMath::Max(0.0f, DeathElapsed - Definition->CorpseSinkDelaySeconds);
        DeathElapsed += DeltaTime;
        const float After = FMath::Max(0.0f, DeathElapsed - Definition->CorpseSinkDelaySeconds);
        Visual->AddWorldOffset(FVector(0, 0, -(After - Before) * Definition->CorpseSinkSpeedCmPerSecond));
        return;
    }
    VoiceCooldown = FMath::Max(0.0f, VoiceCooldown - DeltaTime);
    FireRemaining = FMath::Max(0.0f, FireRemaining - DeltaTime);
    const FVector Position = GetOwner()->GetActorLocation();
    const FVector Travel = Position - PreviousPosition;
    MovementDistance += Travel.Size2D();
    MovementWindow += DeltaTime;
    const float Speed = DeltaTime > SMALL_NUMBER ? Travel.Size2D() / DeltaTime : 0;
    if (MovementWindow >= 0.1f)
    {
        LastMoveSpeed = MovementDistance / MovementWindow;
        MovementDistance = MovementWindow = 0;
    }
    PreviousPosition = Position;
    MovingRemaining = FMath::Max(0.0f, MovingRemaining - DeltaTime);
    if (Speed > 1)
    {
        MovingRemaining = 0.12f;
        Visual->SetWorldRotation(FRotator(0, Travel.Rotation().Yaw, 0) + Definition->MeshRotation);
    }
    ActionRemaining = FMath::Max(0.0f, ActionRemaining - DeltaTime);
    if (ActionRemaining > 0 && MovingRemaining <= 0) return;
    if (MovingRemaining > 0)
    {
        Play(Definition->Move, true);
        Visual->SetPlayRate(FMath::Clamp(LastMoveSpeed / Definition->LocomotionReferenceSpeed, 0.5f, 2.0f));
        FootstepRemaining -= DeltaTime;
        if (FootstepRemaining <= 0)
        {
            Sound(Definition->FootstepSound, TEXT("footstep"));
            FootstepRemaining = 0.4f;
        }
    }
    else
    {
        Visual->SetPlayRate(1.0f);
        Play(FireRemaining > 0 ? Definition->Fire.Get() : bSelected && Definition->ReadyClip
            ? Definition->ReadyClip.Get() : Definition->Idle.Get(), true);
        FootstepRemaining = 0;
    }
}
