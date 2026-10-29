#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PolitikaPrototypeContent.generated.h"

class UStaticMesh;
class UWorld;
class UPolitikaInfantryPresentationDefinition;
class UPolitikaInfantryBehaviorDefinition;

UENUM(BlueprintType)
enum class EPolitikaPrototypeMapProfile : uint8
{
    Test,
    Skirmish,
    Campaign
};

UENUM(BlueprintType)
enum class EPolitikaPrototypeObjectiveType : uint8
{
    ControlPoint
};

UENUM(BlueprintType)
enum class EPolitikaContentDiagnosticSeverity : uint8
{
    Info,
    Warning,
    Error
};

USTRUCT(BlueprintType)
struct POLITIKA_API FPolitikaContentDiagnostic
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName Code;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EPolitikaContentDiagnosticSeverity Severity = EPolitikaContentDiagnosticSeverity::Error;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName AssetId;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString PropertyPath;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString Message;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString Expected;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString Actual;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString SuggestedAction;
};

UCLASS(BlueprintType)
class POLITIKA_API UPolitikaWeaponDefinition final : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    static constexpr int32 CurrentSchemaVersion = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") FName StableId;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") int32 SchemaVersion = CurrentSchemaVersion;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Display") FText DisplayName;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat", meta=(Units="cm")) float RangeCm = 0.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat") float Damage = 0.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat", meta=(Units="s")) float CooldownSeconds = 0.0f;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

UCLASS(BlueprintType)
class POLITIKA_API UPolitikaUnitArchetypeDefinition final : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    static constexpr int32 CurrentSchemaVersion = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") FName StableId;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") int32 SchemaVersion = CurrentSchemaVersion;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") FName FactionId;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Display") FText DisplayName;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat", meta=(ClampMin="1")) int32 MaxHealth = 1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement", meta=(Units="cm/s", ClampMin="0")) float MovementSpeedCmPerSecond = 0.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat") FName WeaponId;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation") TObjectPtr<UStaticMesh> VisualMesh;
    // Optional, additive presentation profile. Existing static-mesh archetypes remain valid.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation") TObjectPtr<UPolitikaInfantryPresentationDefinition> InfantryPresentation;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat") TObjectPtr<UPolitikaInfantryBehaviorDefinition> InfantryBehavior;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation") FVector VisualScale = FVector::OneVector;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation") FLinearColor FactionTint = FLinearColor::White;
    // Opt-in for imported textured presentation meshes; existing primitive art remains tinted.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation") bool bUseAuthoredMeshMaterials = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat") bool bAttackable = true;
    // Horizontal fog-of-war vision radius shared with the owning team in networked sessions.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Vision", meta=(Units="cm", ClampMin="0")) float SightRadiusCm = 1200.0f;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

USTRUCT(BlueprintType)
struct POLITIKA_API FPolitikaPrototypePlayerSlot
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) int32 SlotId = 0;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) int32 RuntimePlayerId = 0;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FName FactionId;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) bool bHumanControlled = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FVector StartLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct POLITIKA_API FPolitikaPrototypeUnitPlacement
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FName UnitArchetypeId;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) int32 SlotId = 0;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FVector Location = FVector::ZeroVector;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FRotator Rotation = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct POLITIKA_API FPolitikaPrototypeObjectiveDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FName StableId;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) EPolitikaPrototypeObjectiveType Type = EPolitikaPrototypeObjectiveType::ControlPoint;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FVector Location = FVector::ZeroVector;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Units="cm", ClampMin="0")) float CaptureRadiusCm = 0.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Units="s", ClampMin="0")) float CaptureDurationSeconds = 0.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UStaticMesh> MarkerMesh;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FVector MarkerScale = FVector::OneVector;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FLinearColor MarkerTint = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct POLITIKA_API FPolitikaPrototypeLightingDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FVector SunLocation = FVector::ZeroVector;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FRotator SunRotation = FRotator::ZeroRotator;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0")) float SunIntensity = 0.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FLinearColor SunColor = FLinearColor::White;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FLinearColor GroundTint = FLinearColor::White;
};

UCLASS(BlueprintType)
class POLITIKA_API UPolitikaPrototypeMapDefinition final : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    static constexpr int32 CurrentSchemaVersion = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") FName StableId;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") int32 SchemaVersion = CurrentSchemaVersion;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") EPolitikaPrototypeMapProfile Profile = EPolitikaPrototypeMapProfile::Test;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") FSoftObjectPath LevelAsset;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Space") FName CoordinateSpace = TEXT("world_xy_centimeters");
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Space") FVector2D PlayableMin = FVector2D::ZeroVector;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Space") FVector2D PlayableMax = FVector2D::ZeroVector;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Space") FName MovementProfileId;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Space") FName GroundActorTag = TEXT("PolitikaPrototypeGround");
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera") FVector CameraOffset = FVector::ZeroVector;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(ClampMin="1", ClampMax="179")) float CameraFieldOfView = 65.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera") FVector2D CameraPanMin = FVector2D::ZeroVector;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera") FVector2D CameraPanMax = FVector2D::ZeroVector;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scenario") TArray<FPolitikaPrototypePlayerSlot> PlayerSlots;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scenario") TArray<FPolitikaPrototypeUnitPlacement> UnitPlacements;
    // Units of the Unaligned NPC side, present in every layout and never part of a player
    // slot. SlotId must be 0 and the archetype must belong to faction.unaligned.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scenario") TArray<FPolitikaPrototypeUnitPlacement> UnalignedPlacements;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scenario") FPolitikaPrototypeObjectiveDefinition Objective;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation") FPolitikaPrototypeLightingDefinition Lighting;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

UCLASS(BlueprintType)
class POLITIKA_API UPolitikaPrototypeContentCatalog final : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    static constexpr int32 CurrentSchemaVersion = 1;
    static const FName CatalogAssetId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") FName StableId;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") int32 SchemaVersion = CurrentSchemaVersion;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Content") TArray<TObjectPtr<UPolitikaWeaponDefinition>> Weapons;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Content") TArray<TObjectPtr<UPolitikaUnitArchetypeDefinition>> Units;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Content") TArray<TObjectPtr<UPolitikaPrototypeMapDefinition>> Maps;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
    const UPolitikaWeaponDefinition* FindWeapon(FName Id) const;
    const UPolitikaUnitArchetypeDefinition* FindUnit(FName Id) const;
    const UPolitikaPrototypeMapDefinition* FindMap(FName Id) const;
};

USTRUCT()
struct POLITIKA_API FPolitikaPrototypeContentSnapshot
{
    GENERATED_BODY()

    UPROPERTY() TObjectPtr<UPolitikaPrototypeContentCatalog> Catalog;
    UPROPERTY() TObjectPtr<UPolitikaPrototypeMapDefinition> MapDefinition;
};

UCLASS()
class POLITIKA_API UPolitikaPrototypeContentValidator final : public UObject
{
    GENERATED_BODY()
public:
    static constexpr int32 SupportedSchemaVersion = 1;
    static const FName PrototypeMapAssetId;
    static const FName PrototypeMovementProfileId;

    static void ValidateCatalog(const UPolitikaPrototypeContentCatalog* Catalog, TArray<FPolitikaContentDiagnostic>& OutDiagnostics);
    static void ValidateMapStructure(const UPolitikaPrototypeMapDefinition* Map, const UPolitikaPrototypeContentCatalog* Catalog, TArray<FPolitikaContentDiagnostic>& OutDiagnostics);
    static bool ValidateMapSpatial(const UWorld* World, const UPolitikaPrototypeMapDefinition* Map, TArray<FPolitikaContentDiagnostic>& OutDiagnostics);
    static bool HasErrors(const TArray<FPolitikaContentDiagnostic>& Diagnostics);
    static FString FormatDiagnostic(const FPolitikaContentDiagnostic& Diagnostic);
    static FString DiagnosticsToJson(const TArray<FPolitikaContentDiagnostic>& Diagnostics);
};

class POLITIKA_API FPolitikaPrototypeContentLoader final
{
public:
    static bool LoadAndValidate(UWorld* World, FName MapId, FPolitikaPrototypeContentSnapshot& OutSnapshot, TArray<FPolitikaContentDiagnostic>& OutDiagnostics);
#if WITH_DEV_AUTOMATION_TESTS
    // One-shot authored candidate override at the normal map-resolution boundary.
    // The catalog and shared validators still run exactly as in production.
    static void SetAutomationMapCandidate(UPolitikaPrototypeMapDefinition* Candidate);
    static void ClearAutomationMapCandidate();
#endif
};
