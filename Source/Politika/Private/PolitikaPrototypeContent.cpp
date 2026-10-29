#include "PolitikaPrototypeContent.h"

#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Components/StaticMeshComponent.h"

const FName UPolitikaPrototypeContentCatalog::CatalogAssetId(TEXT("catalog.prototype_content"));
const FName UPolitikaPrototypeContentValidator::PrototypeMapAssetId(TEXT("map.prototype.rts_vertical_slice"));
const FName UPolitikaPrototypeContentValidator::PrototypeMovementProfileId(TEXT("movement.prototype_ground"));

namespace
{
constexpr int32 MaxSupportedSchemaVersion = 1;
#if WITH_DEV_AUTOMATION_TESTS
UPolitikaPrototypeMapDefinition* GAutomationMapCandidate = nullptr;
#endif

bool IsFiniteVector(const FVector& Value)
{
    return FMath::IsFinite(Value.X) && FMath::IsFinite(Value.Y) && FMath::IsFinite(Value.Z);
}

bool IsFiniteColor(const FLinearColor& Value)
{
    return FMath::IsFinite(Value.R) && FMath::IsFinite(Value.G)
        && FMath::IsFinite(Value.B) && FMath::IsFinite(Value.A);
}

bool IsPositiveScale(const FVector& Value)
{
    return IsFiniteVector(Value) && Value.X > 0.0 && Value.Y > 0.0 && Value.Z > 0.0;
}

bool IsStableId(FName Id)
{
    if (Id.IsNone()) return false;
    const FString Text = Id.ToString();
    if (Text.IsEmpty() || Text != Text.ToLower() || Text[0] == TEXT('.') || Text[Text.Len() - 1] == TEXT('.')) return false;
    bool bPreviousSeparator = false;
    for (const TCHAR Character : Text)
    {
        if (Character == TEXT('.'))
        {
            if (bPreviousSeparator) return false;
            bPreviousSeparator = true;
        }
        else
        {
            if (!(FChar::IsLower(Character) || FChar::IsDigit(Character) || Character == TEXT('_'))) return false;
            bPreviousSeparator = false;
        }
    }
    return true;
}

FString SeverityText(EPolitikaContentDiagnosticSeverity Severity)
{
    switch (Severity)
    {
        case EPolitikaContentDiagnosticSeverity::Info: return TEXT("info");
        case EPolitikaContentDiagnosticSeverity::Warning: return TEXT("warning");
        case EPolitikaContentDiagnosticSeverity::Error: return TEXT("error");
        default: return TEXT("error");
    }
}

void AddDiagnostic(TArray<FPolitikaContentDiagnostic>& Out, FName Code, FName AssetId,
    FString PropertyPath, FString Message, FString Expected = {}, FString Actual = {}, FString SuggestedAction = {},
    EPolitikaContentDiagnosticSeverity Severity = EPolitikaContentDiagnosticSeverity::Error)
{
    FPolitikaContentDiagnostic& Diagnostic = Out.AddDefaulted_GetRef();
    Diagnostic.Code = Code;
    Diagnostic.Severity = Severity;
    Diagnostic.AssetId = AssetId;
    Diagnostic.PropertyPath = MoveTemp(PropertyPath);
    Diagnostic.Message = MoveTemp(Message);
    Diagnostic.Expected = MoveTemp(Expected);
    Diagnostic.Actual = MoveTemp(Actual);
    Diagnostic.SuggestedAction = MoveTemp(SuggestedAction);
}

bool InBounds(const UPolitikaPrototypeMapDefinition& Map, const FVector& Location)
{
    return IsFiniteVector(Location)
        && Location.X >= Map.PlayableMin.X && Location.X <= Map.PlayableMax.X
        && Location.Y >= Map.PlayableMin.Y && Location.Y <= Map.PlayableMax.Y;
}

void ValidateSchemaVersion(int32 Actual, FName AssetId, FString Property, TArray<FPolitikaContentDiagnostic>& Out)
{
    if (Actual != MaxSupportedSchemaVersion)
    {
        AddDiagnostic(Out, TEXT("CONTENT.SCHEMA.UNSUPPORTED"), AssetId, MoveTemp(Property),
            TEXT("The content schema version is not supported."), FString::FromInt(MaxSupportedSchemaVersion),
            FString::FromInt(Actual), TEXT("Migrate the asset to the supported schema version before using it."));
    }
}

void ValidateStableId(FName Id, FName AssetId, FString Property, TArray<FPolitikaContentDiagnostic>& Out)
{
    if (!IsStableId(Id))
    {
        AddDiagnostic(Out, TEXT("CONTENT.ID.INVALID"), AssetId, MoveTemp(Property),
            TEXT("Stable IDs must contain lowercase ASCII letters, digits, or underscores, with optional single-dot namespace separators."),
            TEXT("[a-z0-9_]+(\\.[a-z0-9_]+)*"), Id.ToString(), TEXT("Assign a stable ID in the owning type namespace."));
    }
}

template <typename T>
void CheckDuplicateIds(const TArray<T*>& Values, const TCHAR* TypeName, TArray<FPolitikaContentDiagnostic>& Out)
{
    TMap<FName, int32> Counts;
    for (const T* Value : Values) if (Value) ++Counts.FindOrAdd(Value->StableId);
    for (const TPair<FName, int32>& Pair : Counts)
    {
        if (Pair.Key.IsNone() || Pair.Value < 2) continue;
        AddDiagnostic(Out, TEXT("CONTENT.ID.DUPLICATE"), Pair.Key, FString::Printf(TEXT("%s.StableId"), TypeName),
            TEXT("Two or more definitions share the same stable ID in this type namespace."),
            TEXT("unique ID within type"), FString::Printf(TEXT("%s (%d definitions)"), *Pair.Key.ToString(), Pair.Value),
            TEXT("Give each definition a distinct stable ID."));
    }
}

bool IsKnownMapProfile(EPolitikaPrototypeMapProfile Profile)
{
    return Profile == EPolitikaPrototypeMapProfile::Test
        || Profile == EPolitikaPrototypeMapProfile::Skirmish
        || Profile == EPolitikaPrototypeMapProfile::Campaign;
}

bool ProjectToNav(const UNavigationSystemV1* NavSystem, const ANavigationData* NavData,
    const FVector& Input, FNavLocation& Output)
{
    return NavSystem && NavData
        && NavSystem->ProjectPointToNavigation(Input, Output, FVector(80.0, 80.0, 180.0), NavData);
}
}

#if WITH_DEV_AUTOMATION_TESTS
void FPolitikaPrototypeContentLoader::SetAutomationMapCandidate(UPolitikaPrototypeMapDefinition* Candidate)
{
    GAutomationMapCandidate = Candidate;
}

void FPolitikaPrototypeContentLoader::ClearAutomationMapCandidate()
{
    GAutomationMapCandidate = nullptr;
}
#endif

FPrimaryAssetId UPolitikaWeaponDefinition::GetPrimaryAssetId() const
{
    return FPrimaryAssetId(FPrimaryAssetType(TEXT("PolitikaWeapon")), StableId);
}

FPrimaryAssetId UPolitikaUnitArchetypeDefinition::GetPrimaryAssetId() const
{
    return FPrimaryAssetId(FPrimaryAssetType(TEXT("PolitikaUnit")), StableId);
}

FPrimaryAssetId UPolitikaPrototypeMapDefinition::GetPrimaryAssetId() const
{
    return FPrimaryAssetId(FPrimaryAssetType(TEXT("PolitikaMap")), StableId);
}

FPrimaryAssetId UPolitikaPrototypeContentCatalog::GetPrimaryAssetId() const
{
    return FPrimaryAssetId(FPrimaryAssetType(TEXT("PolitikaPrototypeContentCatalog")), StableId);
}

const UPolitikaWeaponDefinition* UPolitikaPrototypeContentCatalog::FindWeapon(FName Id) const
{
    for (const UPolitikaWeaponDefinition* Weapon : Weapons)
        if (Weapon && Weapon->StableId == Id) return Weapon;
    return nullptr;
}

const UPolitikaUnitArchetypeDefinition* UPolitikaPrototypeContentCatalog::FindUnit(FName Id) const
{
    for (const UPolitikaUnitArchetypeDefinition* Unit : Units)
        if (Unit && Unit->StableId == Id) return Unit;
    return nullptr;
}

const UPolitikaPrototypeMapDefinition* UPolitikaPrototypeContentCatalog::FindMap(FName Id) const
{
    for (const UPolitikaPrototypeMapDefinition* Map : Maps)
        if (Map && Map->StableId == Id) return Map;
    return nullptr;
}

bool UPolitikaPrototypeContentValidator::HasErrors(const TArray<FPolitikaContentDiagnostic>& Diagnostics)
{
    return Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& Diagnostic)
    {
        return Diagnostic.Severity == EPolitikaContentDiagnosticSeverity::Error;
    });
}

void UPolitikaPrototypeContentValidator::ValidateCatalog(
    const UPolitikaPrototypeContentCatalog* Catalog,
    TArray<FPolitikaContentDiagnostic>& OutDiagnostics)
{
    if (!Catalog)
    {
        AddDiagnostic(OutDiagnostics, TEXT("CONTENT.CATALOG.MISSING"), UPolitikaPrototypeContentCatalog::CatalogAssetId, TEXT("Catalog"),
            TEXT("The required prototype content catalog could not be loaded."), UPolitikaPrototypeContentCatalog::CatalogAssetId.ToString(), TEXT("missing"),
            TEXT("Create the catalog asset and include the prototype definition assets."));
        return;
    }

    ValidateStableId(Catalog->StableId, Catalog->StableId, TEXT("StableId"), OutDiagnostics);
    if (Catalog->StableId != UPolitikaPrototypeContentCatalog::CatalogAssetId)
        AddDiagnostic(OutDiagnostics, TEXT("CONTENT.CATALOG.ID_MISMATCH"), Catalog->StableId, TEXT("StableId"),
            TEXT("The catalog stable ID does not match the configured primary asset ID used for deterministic discovery."),
            UPolitikaPrototypeContentCatalog::CatalogAssetId.ToString(), Catalog->StableId.ToString(),
            TEXT("Set the catalog stable ID to the configured catalog identifier."));
    ValidateSchemaVersion(Catalog->SchemaVersion, Catalog->StableId, TEXT("SchemaVersion"), OutDiagnostics);

    TArray<UPolitikaWeaponDefinition*> Weapons;
    TArray<UPolitikaUnitArchetypeDefinition*> Units;
    TArray<UPolitikaPrototypeMapDefinition*> Maps;
    for (const TObjectPtr<UPolitikaWeaponDefinition>& Weapon : Catalog->Weapons)
    {
        if (!Weapon)
        {
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.REFERENCE.NULL"), Catalog->StableId, TEXT("Weapons"),
                TEXT("The catalog contains an empty weapon reference."), TEXT("weapon asset"), TEXT("null"), TEXT("Assign a valid weapon definition."));
            continue;
        }
        Weapons.Add(Weapon.Get());
        ValidateStableId(Weapon->StableId, Weapon->StableId, TEXT("StableId"), OutDiagnostics);
        ValidateSchemaVersion(Weapon->SchemaVersion, Weapon->StableId, TEXT("SchemaVersion"), OutDiagnostics);
        if (Weapon->DisplayName.IsEmpty())
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.VALUE.REQUIRED"), Weapon->StableId, TEXT("DisplayName"), TEXT("A weapon display name is required."), TEXT("non-empty localized name"), TEXT("empty"), TEXT("Set a localized display name."));
        if (!FMath::IsFinite(Weapon->RangeCm) || Weapon->RangeCm <= 0.0f)
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.VALUE.RANGE"), Weapon->StableId, TEXT("RangeCm"), TEXT("Weapon range must be finite and greater than zero."), TEXT("> 0 cm"), LexToString(Weapon->RangeCm), TEXT("Set a positive finite weapon range."));
        if (!FMath::IsFinite(Weapon->Damage) || Weapon->Damage <= 0.0f)
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.VALUE.RANGE"), Weapon->StableId, TEXT("Damage"), TEXT("Damage must be finite and greater than zero for an assigned prototype weapon."), TEXT("> 0 damage"), LexToString(Weapon->Damage), TEXT("Set positive finite damage."));
        if (!FMath::IsFinite(Weapon->CooldownSeconds) || Weapon->CooldownSeconds <= 0.0f)
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.VALUE.RANGE"), Weapon->StableId, TEXT("CooldownSeconds"), TEXT("Weapon cooldown must be finite and greater than zero."), TEXT("> 0 seconds"), LexToString(Weapon->CooldownSeconds), TEXT("Set a positive finite cooldown."));
    }
    for (const TObjectPtr<UPolitikaUnitArchetypeDefinition>& Unit : Catalog->Units)
    {
        if (!Unit)
        {
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.REFERENCE.NULL"), Catalog->StableId, TEXT("Units"),
                TEXT("The catalog contains an empty unit reference."), TEXT("unit archetype asset"), TEXT("null"), TEXT("Assign a valid unit archetype definition."));
            continue;
        }
        Units.Add(Unit.Get());
        ValidateStableId(Unit->StableId, Unit->StableId, TEXT("StableId"), OutDiagnostics);
        ValidateSchemaVersion(Unit->SchemaVersion, Unit->StableId, TEXT("SchemaVersion"), OutDiagnostics);
        if (Unit->FactionId.IsNone())
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.VALUE.REQUIRED"), Unit->StableId, TEXT("FactionId"), TEXT("A unit must declare its owning faction."), TEXT("stable faction ID"), TEXT("empty"), TEXT("Assign the owning faction ID."));
        if (Unit->DisplayName.IsEmpty())
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.VALUE.REQUIRED"), Unit->StableId, TEXT("DisplayName"), TEXT("A unit display name is required."), TEXT("non-empty localized name"), TEXT("empty"), TEXT("Set a localized display name."));
        if (Unit->MaxHealth <= 0)
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.VALUE.RANGE"), Unit->StableId, TEXT("MaxHealth"), TEXT("Maximum health must be positive."), TEXT("> 0 health"), LexToString(Unit->MaxHealth), TEXT("Set positive maximum health."));
        if (!FMath::IsFinite(Unit->MovementSpeedCmPerSecond) || Unit->MovementSpeedCmPerSecond <= 0.0f)
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.VALUE.RANGE"), Unit->StableId, TEXT("MovementSpeedCmPerSecond"), TEXT("This prototype movement profile requires a positive finite movement speed."), TEXT("> 0 cm/s"), LexToString(Unit->MovementSpeedCmPerSecond), TEXT("Set a positive finite movement speed."));
        if (Unit->WeaponId.IsNone() || !Catalog->FindWeapon(Unit->WeaponId))
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.REFERENCE.UNRESOLVED"), Unit->StableId, TEXT("WeaponId"), TEXT("The unit weapon reference does not resolve in the catalog."), TEXT("ID of a catalog weapon"), Unit->WeaponId.ToString(), TEXT("Add the weapon to the catalog or correct the stable ID."));
        if (!Unit->VisualMesh)
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.REFERENCE.MISSING_MESH"), Unit->StableId, TEXT("VisualMesh"), TEXT("A prototype unit requires a cooked presentation mesh."), TEXT("loaded UStaticMesh"), TEXT("null"), TEXT("Assign a mesh; runtime does not substitute a hidden default."));
        if (!IsPositiveScale(Unit->VisualScale))
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.VALUE.RANGE"), Unit->StableId, TEXT("VisualScale"), TEXT("Visual scale components must be finite and positive."), TEXT("finite XYZ > 0"), Unit->VisualScale.ToString(), TEXT("Correct the scale."));
        if (!IsFiniteColor(Unit->FactionTint))
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.VALUE.NONFINITE"), Unit->StableId, TEXT("FactionTint"), TEXT("Faction tint contains a non-finite component."), TEXT("finite RGBA"), Unit->FactionTint.ToString(), TEXT("Replace non-finite color components."));
    }
    for (const TObjectPtr<UPolitikaPrototypeMapDefinition>& Map : Catalog->Maps)
    {
        if (!Map)
        {
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.REFERENCE.NULL"), Catalog->StableId, TEXT("Maps"),
                TEXT("The catalog contains an empty map reference."), TEXT("map definition asset"), TEXT("null"), TEXT("Assign a valid map definition."));
            continue;
        }
        Maps.Add(Map.Get());
        ValidateMapStructure(Map.Get(), Catalog, OutDiagnostics);
    }
    CheckDuplicateIds(Weapons, TEXT("Weapons"), OutDiagnostics);
    CheckDuplicateIds(Units, TEXT("Units"), OutDiagnostics);
    CheckDuplicateIds(Maps, TEXT("Maps"), OutDiagnostics);
}

void UPolitikaPrototypeContentValidator::ValidateMapStructure(
    const UPolitikaPrototypeMapDefinition* Map,
    const UPolitikaPrototypeContentCatalog* Catalog,
    TArray<FPolitikaContentDiagnostic>& OutDiagnostics)
{
    if (!Map)
    {
        AddDiagnostic(OutDiagnostics, TEXT("MAP.DEFINITION.MISSING"), NAME_None, TEXT("Map"),
            TEXT("A gameplay map definition is required for structural validation."), TEXT("UPolitikaPrototypeMapDefinition"), TEXT("missing"),
            TEXT("Load the map definition from the content catalog before validation."));
        return;
    }
    const FName Id = Map->StableId;
    ValidateStableId(Map->StableId, Id, TEXT("StableId"), OutDiagnostics);
    ValidateSchemaVersion(Map->SchemaVersion, Id, TEXT("SchemaVersion"), OutDiagnostics);
    if (!IsKnownMapProfile(Map->Profile))
        AddDiagnostic(OutDiagnostics, TEXT("MAP.PROFILE.INVALID"), Id, TEXT("Profile"), TEXT("The map profile enum is invalid."), TEXT("Test, Skirmish, or Campaign"), LexToString(static_cast<uint8>(Map->Profile)), TEXT("Select a supported map profile."));
    if (Map->LevelAsset.IsNull())
        AddDiagnostic(OutDiagnostics, TEXT("MAP.LEVEL.MISSING"), Id, TEXT("LevelAsset"), TEXT("The map definition must reference its Unreal level."), TEXT("valid level soft object path"), TEXT("null"), TEXT("Set the matching Unreal level asset."));
    if (Map->CoordinateSpace != TEXT("world_xy_centimeters"))
        AddDiagnostic(OutDiagnostics, TEXT("MAP.COORDINATE_SPACE.UNSUPPORTED"), Id, TEXT("CoordinateSpace"), TEXT("The prototype supports Unreal world XY coordinates in centimeters."), TEXT("world_xy_centimeters"), Map->CoordinateSpace.ToString(), TEXT("Use the supported coordinate convention or implement a converter."));
    if (!FMath::IsFinite(Map->PlayableMin.X) || !FMath::IsFinite(Map->PlayableMin.Y)
        || !FMath::IsFinite(Map->PlayableMax.X) || !FMath::IsFinite(Map->PlayableMax.Y)
        || Map->PlayableMin.X >= Map->PlayableMax.X || Map->PlayableMin.Y >= Map->PlayableMax.Y)
        AddDiagnostic(OutDiagnostics, TEXT("MAP.BOUNDS.INVALID"), Id, TEXT("PlayableMin/PlayableMax"), TEXT("Playable bounds must be finite and have positive area."), TEXT("finite min < max on both axes"), FString::Printf(TEXT("min=%s max=%s"), *Map->PlayableMin.ToString(), *Map->PlayableMax.ToString()), TEXT("Correct the playable bounds."));
    if (Map->MovementProfileId != PrototypeMovementProfileId)
        AddDiagnostic(OutDiagnostics, TEXT("MAP.MOVEMENT.UNSUPPORTED"), Id, TEXT("MovementProfileId"), TEXT("The fixture validator and movement implementation support only its authored prototype ground profile."), PrototypeMovementProfileId.ToString(), Map->MovementProfileId.ToString(), TEXT("Use the prototype ground profile or implement and register another traversal profile."));
    if (!FMath::IsFinite(Map->CameraFieldOfView) || Map->CameraFieldOfView <= 1.0f || Map->CameraFieldOfView >= 179.0f
        || !IsFiniteVector(Map->CameraOffset))
        AddDiagnostic(OutDiagnostics, TEXT("MAP.CAMERA.INVALID"), Id, TEXT("CameraOffset/CameraFieldOfView"), TEXT("Camera offset and field of view must be finite and the FOV must be between 1 and 179 degrees."), TEXT("finite offset; 1 < FOV < 179"), FString::Printf(TEXT("offset=%s fov=%s"), *Map->CameraOffset.ToString(), *LexToString(Map->CameraFieldOfView)), TEXT("Correct the map camera values."));
    if (Map->CameraPanMin.X > Map->CameraPanMax.X || Map->CameraPanMin.Y > Map->CameraPanMax.Y
        || Map->CameraPanMin.X < Map->PlayableMin.X || Map->CameraPanMin.Y < Map->PlayableMin.Y
        || Map->CameraPanMax.X > Map->PlayableMax.X || Map->CameraPanMax.Y > Map->PlayableMax.Y)
        AddDiagnostic(OutDiagnostics, TEXT("MAP.CAMERA.BOUNDS_INVALID"), Id, TEXT("CameraPanMin/CameraPanMax"), TEXT("Camera pan bounds must be ordered and contained by playable bounds."), TEXT("playable bounds containment"), FString::Printf(TEXT("min=%s max=%s"), *Map->CameraPanMin.ToString(), *Map->CameraPanMax.ToString()), TEXT("Correct the camera pan bounds."));

    if (Map->PlayerSlots.Num() < 2)
        AddDiagnostic(OutDiagnostics, TEXT("MAP.SLOTS.INSUFFICIENT"), Id, TEXT("PlayerSlots"), TEXT("The prototype fixture requires at least two occupied test slots."), TEXT(">= 2 slots"), LexToString(Map->PlayerSlots.Num()), TEXT("Author a slot for each test side."));
    TMap<int32, int32> SlotCounts;
    TMap<uint32, int32> PlayerCounts;
    TMap<FName, int32> FactionCounts;
    int32 HumanSlotCount = 0;
    for (int32 Index = 0; Index < Map->PlayerSlots.Num(); ++Index)
    {
        const FPolitikaPrototypePlayerSlot& Slot = Map->PlayerSlots[Index];
        ++SlotCounts.FindOrAdd(Slot.SlotId);
        ++PlayerCounts.FindOrAdd(Slot.RuntimePlayerId);
        ++FactionCounts.FindOrAdd(Slot.FactionId);
        HumanSlotCount += Slot.bHumanControlled ? 1 : 0;
        const FString Prefix = FString::Printf(TEXT("PlayerSlots[%d]."), Index);
        if (Slot.SlotId <= 0 || Slot.RuntimePlayerId == 0 || Slot.FactionId.IsNone())
            AddDiagnostic(OutDiagnostics, TEXT("MAP.SLOT.INVALID"), Id, Prefix + TEXT("SlotId/RuntimePlayerId/FactionId"), TEXT("A slot needs positive slot/player IDs and a faction ID."), TEXT("slot > 0; player > 0; faction ID"), FString::Printf(TEXT("slot=%d player=%d faction=%s"), Slot.SlotId, Slot.RuntimePlayerId, *Slot.FactionId.ToString()), TEXT("Assign valid, unique slot metadata."));
        else if (!IsStableId(Slot.FactionId))
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.ID.INVALID"), Id, Prefix + TEXT("FactionId"), TEXT("Faction identifiers must use the stable-ID format."), TEXT("lowercase namespaced ID"), Slot.FactionId.ToString(), TEXT("Correct the faction identifier."));
        if (Slot.FactionId == FName(TEXT("faction.unaligned")))
            AddDiagnostic(OutDiagnostics, TEXT("MAP.FACTION.UNALIGNED_PLAYABLE"), Id, Prefix + TEXT("FactionId"),
                TEXT("The Unaligned system faction cannot occupy an ordinary playable slot."), TEXT("playable faction ID"),
                Slot.FactionId.ToString(), TEXT("Assign a playable test faction; initialize Unaligned separately as system authority."));
        if (!InBounds(*Map, Slot.StartLocation))
            AddDiagnostic(OutDiagnostics, TEXT("MAP.START.OUT_OF_BOUNDS"), Id, Prefix + TEXT("StartLocation"), TEXT("The player start is outside playable bounds or has non-finite coordinates."), FString::Printf(TEXT("XY inside %s..%s"), *Map->PlayableMin.ToString(), *Map->PlayableMax.ToString()), Slot.StartLocation.ToString(), TEXT("Move the start into legal playable bounds."));
    }
    for (const TPair<int32, int32>& Pair : SlotCounts)
        if (Pair.Value > 1) AddDiagnostic(OutDiagnostics, TEXT("MAP.SLOT.DUPLICATE"), Id, TEXT("PlayerSlots.SlotId"), TEXT("Player slot IDs must be unique."), TEXT("unique slot ID"), FString::Printf(TEXT("%d occurs %d times"), Pair.Key, Pair.Value), TEXT("Assign unique slot IDs."));
    for (const TPair<uint32, int32>& Pair : PlayerCounts)
        if (Pair.Key == 0 || Pair.Value > 1) AddDiagnostic(OutDiagnostics, TEXT("MAP.PLAYER.DUPLICATE"), Id, TEXT("PlayerSlots.RuntimePlayerId"), TEXT("Runtime player IDs must be non-zero and unique."), TEXT("unique player ID > 0"), FString::Printf(TEXT("%d occurs %d times"), Pair.Key, Pair.Value), TEXT("Assign unique non-zero player IDs."));
    for (const TPair<FName, int32>& Pair : FactionCounts)
        if (Pair.Key.IsNone() || Pair.Value > 1) AddDiagnostic(OutDiagnostics, TEXT("MAP.FACTION.DUPLICATE"), Id, TEXT("PlayerSlots.FactionId"), TEXT("Each prototype slot must use a distinct, non-empty faction ID."), TEXT("unique faction ID"), FString::Printf(TEXT("%s occurs %d times"), *Pair.Key.ToString(), Pair.Value), TEXT("Assign distinct faction IDs to the opposing slots."));
    if (HumanSlotCount != 1)
        AddDiagnostic(OutDiagnostics, TEXT("MAP.SLOTS.HUMAN_COUNT"), Id, TEXT("PlayerSlots.bHumanControlled"), TEXT("The prototype requires exactly one locally controlled slot."), TEXT("exactly 1"), LexToString(HumanSlotCount), TEXT("Mark one slot as locally controlled."));

    TMap<int32, int32> StartCounts;
    for (const FPolitikaPrototypePlayerSlot& Slot : Map->PlayerSlots) ++StartCounts.FindOrAdd(Slot.SlotId);
    for (const TPair<int32, int32>& Pair : StartCounts)
        if (Pair.Value != 1) AddDiagnostic(OutDiagnostics, TEXT("MAP.START.COUNT"), Id, TEXT("PlayerSlots.StartLocation"), TEXT("Each player slot must have exactly one required start."), TEXT("1 start per slot"), FString::Printf(TEXT("slot %d has %d"), Pair.Key, Pair.Value), TEXT("Add or remove duplicate starts."));
    for (int32 Left = 0; Left < Map->PlayerSlots.Num(); ++Left)
        for (int32 Right = Left + 1; Right < Map->PlayerSlots.Num(); ++Right)
            if (Map->PlayerSlots[Left].StartLocation.Equals(Map->PlayerSlots[Right].StartLocation, 1.0f))
                AddDiagnostic(OutDiagnostics, TEXT("MAP.START.DUPLICATE"), Id, TEXT("PlayerSlots.StartLocation"),
                    TEXT("Two player slots occupy the same start location."), TEXT("distinct start positions"),
                    FString::Printf(TEXT("slots %d and %d at %s"), Map->PlayerSlots[Left].SlotId, Map->PlayerSlots[Right].SlotId, *Map->PlayerSlots[Left].StartLocation.ToString()),
                    TEXT("Move the starts apart so each side has a valid deployment point."));

    if (Map->UnitPlacements.IsEmpty())
        AddDiagnostic(OutDiagnostics, TEXT("MAP.UNITS.EMPTY"), Id, TEXT("UnitPlacements"), TEXT("The prototype map requires unit placements."), TEXT("at least one placement"), TEXT("empty"), TEXT("Add authored fixture unit placements."));
    for (int32 Index = 0; Index < Map->UnitPlacements.Num(); ++Index)
    {
        const FPolitikaPrototypeUnitPlacement& Placement = Map->UnitPlacements[Index];
        const FString Prefix = FString::Printf(TEXT("UnitPlacements[%d]."), Index);
        const UPolitikaUnitArchetypeDefinition* Unit = Catalog ? Catalog->FindUnit(Placement.UnitArchetypeId) : nullptr;
        if (Placement.UnitArchetypeId.IsNone() || !Unit)
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.REFERENCE.UNRESOLVED"), Id, Prefix + TEXT("UnitArchetypeId"), TEXT("The unit placement does not resolve to a unit in the catalog."), TEXT("catalog unit ID"), Placement.UnitArchetypeId.ToString(), TEXT("Correct the ID or include the unit definition in the catalog."));
        const FPolitikaPrototypePlayerSlot* Slot = Map->PlayerSlots.FindByPredicate([&Placement](const FPolitikaPrototypePlayerSlot& Candidate) { return Candidate.SlotId == Placement.SlotId; });
        if (!Slot)
            AddDiagnostic(OutDiagnostics, TEXT("MAP.SLOT.REFERENCE_UNRESOLVED"), Id, Prefix + TEXT("SlotId"), TEXT("The unit placement references an undeclared player slot."), TEXT("declared slot ID"), LexToString(Placement.SlotId), TEXT("Add the slot or correct the placement."));
        else if (Unit && Unit->FactionId != Slot->FactionId)
            AddDiagnostic(OutDiagnostics, TEXT("CONTENT.FACTION.MISMATCH"), Id, Prefix + TEXT("UnitArchetypeId"), TEXT("The placed unit faction does not match the faction assigned to its slot."), Slot->FactionId.ToString(), Unit->FactionId.ToString(), TEXT("Use an archetype owned by the slot faction."));
        if (!InBounds(*Map, Placement.Location))
            AddDiagnostic(OutDiagnostics, TEXT("MAP.UNIT.OUT_OF_BOUNDS"), Id, Prefix + TEXT("Location"), TEXT("The unit placement is outside playable bounds or has non-finite coordinates."), FString::Printf(TEXT("XY inside %s..%s"), *Map->PlayableMin.ToString(), *Map->PlayableMax.ToString()), Placement.Location.ToString(), TEXT("Move the placement into legal playable bounds."));
        if (!Placement.Rotation.IsNearlyZero() && (!FMath::IsFinite(Placement.Rotation.Pitch) || !FMath::IsFinite(Placement.Rotation.Yaw) || !FMath::IsFinite(Placement.Rotation.Roll)))
            AddDiagnostic(OutDiagnostics, TEXT("MAP.UNIT.ROTATION_NONFINITE"), Id, Prefix + TEXT("Rotation"), TEXT("Unit rotation contains a non-finite value."), TEXT("finite rotation"), Placement.Rotation.ToString(), TEXT("Correct the authored rotation."));
    }

    ValidateStableId(Map->Objective.StableId, Id, TEXT("Objective.StableId"), OutDiagnostics);
    if (Map->Objective.Type != EPolitikaPrototypeObjectiveType::ControlPoint)
        AddDiagnostic(OutDiagnostics, TEXT("MAP.OBJECTIVE.TYPE_UNSUPPORTED"), Id, TEXT("Objective.Type"), TEXT("The prototype currently supports control-point objectives only."), TEXT("ControlPoint"), LexToString(static_cast<uint8>(Map->Objective.Type)), TEXT("Select ControlPoint or implement its rule type."));
    if (!InBounds(*Map, Map->Objective.Location))
        AddDiagnostic(OutDiagnostics, TEXT("MAP.OBJECTIVE.OUT_OF_BOUNDS"), Id, TEXT("Objective.Location"), TEXT("The objective marker is outside playable bounds or has non-finite coordinates."), FString::Printf(TEXT("XY inside %s..%s"), *Map->PlayableMin.ToString(), *Map->PlayableMax.ToString()), Map->Objective.Location.ToString(), TEXT("Move the marker into legal playable bounds."));
    if (!FMath::IsFinite(Map->Objective.CaptureRadiusCm) || Map->Objective.CaptureRadiusCm <= 0.0f)
        AddDiagnostic(OutDiagnostics, TEXT("MAP.OBJECTIVE.RADIUS_INVALID"), Id, TEXT("Objective.CaptureRadiusCm"), TEXT("Capture radius must be finite and positive."), TEXT("> 0 cm"), LexToString(Map->Objective.CaptureRadiusCm), TEXT("Set a positive finite capture radius."));
    if (!FMath::IsFinite(Map->Objective.CaptureDurationSeconds) || Map->Objective.CaptureDurationSeconds <= 0.0f)
        AddDiagnostic(OutDiagnostics, TEXT("MAP.OBJECTIVE.DURATION_INVALID"), Id, TEXT("Objective.CaptureDurationSeconds"), TEXT("Capture duration must be finite and positive."), TEXT("> 0 seconds"), LexToString(Map->Objective.CaptureDurationSeconds), TEXT("Set a positive finite capture duration."));
    if (!Map->Objective.MarkerMesh)
        AddDiagnostic(OutDiagnostics, TEXT("CONTENT.REFERENCE.MISSING_MESH"), Id, TEXT("Objective.MarkerMesh"), TEXT("The objective requires an authored marker mesh."), TEXT("loaded UStaticMesh"), TEXT("null"), TEXT("Assign a marker mesh; runtime does not substitute one."));
    if (!IsPositiveScale(Map->Objective.MarkerScale))
        AddDiagnostic(OutDiagnostics, TEXT("CONTENT.VALUE.RANGE"), Id, TEXT("Objective.MarkerScale"), TEXT("Marker scale components must be finite and positive."), TEXT("finite XYZ > 0"), Map->Objective.MarkerScale.ToString(), TEXT("Correct the marker scale."));
    if (!IsFiniteColor(Map->Objective.MarkerTint))
        AddDiagnostic(OutDiagnostics, TEXT("CONTENT.VALUE.NONFINITE"), Id, TEXT("Objective.MarkerTint"), TEXT("Marker tint contains a non-finite component."), TEXT("finite RGBA"), Map->Objective.MarkerTint.ToString(), TEXT("Correct the marker tint."));

    if (!IsFiniteVector(Map->Lighting.SunLocation)
        || !FMath::IsFinite(Map->Lighting.SunRotation.Pitch) || !FMath::IsFinite(Map->Lighting.SunRotation.Yaw)
        || !FMath::IsFinite(Map->Lighting.SunRotation.Roll)
        || !FMath::IsFinite(Map->Lighting.SunIntensity) || Map->Lighting.SunIntensity < 0.0f
        || !IsFiniteColor(Map->Lighting.SunColor) || !IsFiniteColor(Map->Lighting.GroundTint))
        AddDiagnostic(OutDiagnostics, TEXT("MAP.LIGHTING.INVALID"), Id, TEXT("Lighting"), TEXT("Map lighting values must be finite and intensity must not be negative."), TEXT("finite vectors/colors; intensity >= 0"), TEXT("non-finite or out of range"), TEXT("Correct the map presentation values."));
}

bool UPolitikaPrototypeContentValidator::ValidateMapSpatial(
    const UWorld* World,
    const UPolitikaPrototypeMapDefinition* Map,
    TArray<FPolitikaContentDiagnostic>& OutDiagnostics)
{
    if (!World)
    {
        AddDiagnostic(OutDiagnostics, TEXT("MAP.LEVEL.WORLD_MISSING"), Map ? Map->StableId : NAME_None, TEXT("World"),
            TEXT("The referenced Unreal level could not be loaded for spatial validation."), TEXT("loaded UWorld for the map level"), TEXT("missing"),
            TEXT("Load the referenced level and retry spatial validation."));
        return false;
    }
    if (!Map)
    {
        AddDiagnostic(OutDiagnostics, TEXT("MAP.DEFINITION.MISSING"), NAME_None, TEXT("Map"),
            TEXT("A gameplay map definition is required for spatial validation."), TEXT("UPolitikaPrototypeMapDefinition"), TEXT("missing"),
            TEXT("Load the map definition from the content catalog before spatial validation."));
        return false;
    }
    const FName Id = Map->StableId;
    if (Map->LevelAsset.GetLongPackageName() != World->GetPackage()->GetName())
        AddDiagnostic(OutDiagnostics, TEXT("MAP.LEVEL.MISMATCH"), Id, TEXT("LevelAsset"), TEXT("The loaded Unreal level does not match the level referenced by this map definition."), World->GetPackage()->GetName(), Map->LevelAsset.GetLongPackageName(), TEXT("Set the map definition to the level that owns this playable fixture."));

    AActor* GroundActor = nullptr;
    FBox NavBoundsBox(ForceInit);
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (It->ActorHasTag(Map->GroundActorTag)) GroundActor = *It;
        if (const ANavMeshBoundsVolume* Volume = Cast<ANavMeshBoundsVolume>(*It); Volume && It->ActorHasTag(TEXT("PolitikaPrototypeNavBounds")))
            NavBoundsBox = Volume->GetComponentsBoundingBox(true);
    }
    const AStaticMeshActor* GroundMeshActor = Cast<AStaticMeshActor>(GroundActor);
    const UStaticMeshComponent* GroundComponent = GroundMeshActor ? GroundMeshActor->GetStaticMeshComponent() : nullptr;
    if (!GroundComponent || !GroundComponent->GetStaticMesh())
        AddDiagnostic(OutDiagnostics, TEXT("MAP.GROUND.MISSING"), Id, TEXT("GroundActorTag"), TEXT("The referenced level has no tagged static ground mesh."), Map->GroundActorTag.ToString(), TEXT("missing or no mesh"), TEXT("Author a static ground actor with this tag in the referenced level."));
    else
    {
        if (GroundComponent->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
            AddDiagnostic(OutDiagnostics, TEXT("MAP.GROUND.NO_COLLISION"), Id, TEXT("GroundActor.CollisionEnabled"), TEXT("The ground actor has no collision and cannot support this movement fixture."), TEXT("Query or QueryAndPhysics"), TEXT("NoCollision"), TEXT("Enable ground collision and rebuild navigation."));
        const FBox GroundBounds = GroundComponent->Bounds.GetBox();
        if (GroundBounds.Min.X > Map->PlayableMin.X || GroundBounds.Min.Y > Map->PlayableMin.Y
            || GroundBounds.Max.X < Map->PlayableMax.X || GroundBounds.Max.Y < Map->PlayableMax.Y)
            AddDiagnostic(OutDiagnostics, TEXT("MAP.GROUND.BOUNDS_MISMATCH"), Id, TEXT("GroundActor.Bounds"), TEXT("The ground mesh does not cover the full declared playable bounds."), FString::Printf(TEXT("cover %s..%s"), *Map->PlayableMin.ToString(), *Map->PlayableMax.ToString()), GroundBounds.ToString(), TEXT("Expand the ground or narrow the playable bounds."));
    }
    if (!NavBoundsBox.IsValid)
        AddDiagnostic(OutDiagnostics, TEXT("MAP.NAV.BOUNDS_MISSING"), Id, TEXT("NavigationBounds"), TEXT("The level has no authored NavMesh bounds for the prototype movement profile."), TEXT("ANavMeshBoundsVolume tagged PolitikaPrototypeNavBounds"), TEXT("missing"), TEXT("Add NavMesh bounds and build AI navigation for the level."));
    else if (NavBoundsBox.Min.X > Map->PlayableMin.X || NavBoundsBox.Min.Y > Map->PlayableMin.Y
        || NavBoundsBox.Max.X < Map->PlayableMax.X || NavBoundsBox.Max.Y < Map->PlayableMax.Y)
        AddDiagnostic(OutDiagnostics, TEXT("MAP.NAV.BOUNDS_INVALID"), Id, TEXT("NavigationBounds"),
            TEXT("Navigation bounds do not cover the full playable area."), FString::Printf(TEXT("cover %s..%s"), *Map->PlayableMin.ToString(), *Map->PlayableMax.ToString()),
            NavBoundsBox.ToString(), TEXT("Expand the navigation volume to include all declared gameplay markers."));

    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(const_cast<UWorld*>(World));
    const ANavigationData* NavData = NavSystem ? NavSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
    if (!NavSystem || !NavData)
    {
        AddDiagnostic(OutDiagnostics, TEXT("MAP.NAV.DATA_MISSING"), Id, TEXT("MovementProfileId"), TEXT("No runtime navigation data is available for spatial validation."), TEXT("built default NavMesh for movement profile"), TEXT("missing"), TEXT("Build navigation for the level and include it in the package."));
        return false;
    }
    if (NavSystem->IsNavigationBuildInProgress())
    {
        AddDiagnostic(OutDiagnostics, TEXT("MAP.NAV.NOT_READY"), Id, TEXT("NavigationData"), TEXT("Navigation is still building; a reachability result cannot yet be claimed."), TEXT("completed navigation build"), TEXT("build in progress"), TEXT("Wait for navigation completion before validating or starting the match."));
        return false;
    }

    TMap<int32, FVector> ProjectedStarts;
    for (int32 Index = 0; Index < Map->PlayerSlots.Num(); ++Index)
    {
        const FPolitikaPrototypePlayerSlot& Slot = Map->PlayerSlots[Index];
        FNavLocation Projected;
        if (!ProjectToNav(NavSystem, NavData, Slot.StartLocation, Projected))
        {
            AddDiagnostic(OutDiagnostics, TEXT("MAP.START.NOT_TRAVERSABLE"), Id,
                FString::Printf(TEXT("PlayerSlots[%d].StartLocation"), Index),
                TEXT("The start does not project onto the built NavMesh for the prototype movement profile."),
                Map->MovementProfileId.ToString(), Slot.StartLocation.ToString(), TEXT("Place the start on traversable ground and rebuild navigation."));
        }
        else ProjectedStarts.Add(Slot.SlotId, Projected.Location);
    }

    FNavLocation ProjectedObjective;
    const bool bObjectiveTraversable = ProjectToNav(NavSystem, NavData, Map->Objective.Location, ProjectedObjective);
    if (!bObjectiveTraversable)
        AddDiagnostic(OutDiagnostics, TEXT("MAP.OBJECTIVE.NOT_TRAVERSABLE"), Id, TEXT("Objective.Location"),
            TEXT("The required objective marker does not project onto the built NavMesh for the prototype movement profile."),
            Map->MovementProfileId.ToString(), Map->Objective.Location.ToString(), TEXT("Move the marker onto traversable ground and rebuild navigation."));

    if (bObjectiveTraversable)
    {
        for (const FPolitikaPrototypePlayerSlot& Slot : Map->PlayerSlots)
        {
            const FVector* Start = ProjectedStarts.Find(Slot.SlotId);
            if (!Start) continue;
            double PathLength = 0.0;
            const ENavigationQueryResult::Type Result = UNavigationSystemV1::GetPathLength(
                const_cast<UWorld*>(World), *Start, ProjectedObjective.Location, PathLength, const_cast<ANavigationData*>(NavData));
            if (Result != ENavigationQueryResult::Success || !FMath::IsFinite(PathLength))
                AddDiagnostic(OutDiagnostics, TEXT("MAP.PATH.UNREACHABLE"), Id,
                    FString::Printf(TEXT("PlayerSlots[%d].StartLocation->Objective[%s]"), Slot.SlotId, *Map->Objective.StableId.ToString()),
                    TEXT("The required objective is not reachable from this player start on the built NavMesh."),
                    Map->MovementProfileId.ToString(), FString::Printf(TEXT("start %s, objective %s, result %d"), *Start->ToString(), *ProjectedObjective.Location.ToString(), static_cast<int32>(Result)),
                    TEXT("Remove the blocking topology or author a valid route for this movement profile."));
        }
    }

    for (int32 Index = 0; Index < Map->UnitPlacements.Num(); ++Index)
    {
        FNavLocation Projected;
        const FPolitikaPrototypeUnitPlacement& Placement = Map->UnitPlacements[Index];
        if (!ProjectToNav(NavSystem, NavData, Placement.Location, Projected))
            AddDiagnostic(OutDiagnostics, TEXT("MAP.UNIT.NOT_TRAVERSABLE"), Id,
                FString::Printf(TEXT("UnitPlacements[%d].Location"), Index),
                TEXT("The unit placement does not project onto the built NavMesh for its map movement profile."),
                Map->MovementProfileId.ToString(), Placement.Location.ToString(), TEXT("Place the unit on traversable ground and rebuild navigation."));
    }
    return !HasErrors(OutDiagnostics);
}

FString UPolitikaPrototypeContentValidator::FormatDiagnostic(const FPolitikaContentDiagnostic& Diagnostic)
{
    return FString::Printf(TEXT("%s|%s|%s|%s|%s|expected=%s|actual=%s|action=%s"),
        *Diagnostic.Code.ToString(), *SeverityText(Diagnostic.Severity), *Diagnostic.AssetId.ToString(),
        *Diagnostic.PropertyPath, *Diagnostic.Message, *Diagnostic.Expected, *Diagnostic.Actual, *Diagnostic.SuggestedAction);
}

FString UPolitikaPrototypeContentValidator::DiagnosticsToJson(const TArray<FPolitikaContentDiagnostic>& Diagnostics)
{
    TArray<FPolitikaContentDiagnostic> Sorted = Diagnostics;
    Sorted.Sort([](const FPolitikaContentDiagnostic& A, const FPolitikaContentDiagnostic& B)
    {
        const FString AKey = FString::Printf(TEXT("%s|%s|%s|%s"), *A.AssetId.ToString(), *A.Code.ToString(), *A.PropertyPath, *A.Message);
        const FString BKey = FString::Printf(TEXT("%s|%s|%s|%s"), *B.AssetId.ToString(), *B.Code.ToString(), *B.PropertyPath, *B.Message);
        return AKey < BKey;
    });
    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetNumberField(TEXT("schema_version"), 1);
    Root->SetBoolField(TEXT("passed"), !HasErrors(Sorted));
    TArray<TSharedPtr<FJsonValue>> Values;
    for (const FPolitikaContentDiagnostic& Diagnostic : Sorted)
    {
        TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
        Object->SetStringField(TEXT("code"), Diagnostic.Code.ToString());
        Object->SetStringField(TEXT("severity"), SeverityText(Diagnostic.Severity));
        Object->SetStringField(TEXT("asset_id"), Diagnostic.AssetId.ToString());
        Object->SetStringField(TEXT("property_path"), Diagnostic.PropertyPath);
        Object->SetStringField(TEXT("message"), Diagnostic.Message);
        Object->SetStringField(TEXT("expected"), Diagnostic.Expected);
        Object->SetStringField(TEXT("actual"), Diagnostic.Actual);
        Object->SetStringField(TEXT("suggested_action"), Diagnostic.SuggestedAction);
        Values.Add(MakeShared<FJsonValueObject>(Object));
    }
    Root->SetArrayField(TEXT("diagnostics"), Values);
    FString Output;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    FJsonSerializer::Serialize(Root, Writer);
    return Output;
}

bool FPolitikaPrototypeContentLoader::LoadAndValidate(
    UWorld* World,
    FName MapId,
    FPolitikaPrototypeContentSnapshot& OutSnapshot,
    TArray<FPolitikaContentDiagnostic>& OutDiagnostics)
{
    OutSnapshot = {};
    UAssetManager& AssetManager = UAssetManager::Get();
    const FPrimaryAssetId CatalogId(FPrimaryAssetType(TEXT("PolitikaPrototypeContentCatalog")), UPolitikaPrototypeContentCatalog::CatalogAssetId);
    AssetManager.ScanPathsForPrimaryAssets(CatalogId.PrimaryAssetType, {TEXT("/Game/Politika/Definitions")},
        UPolitikaPrototypeContentCatalog::StaticClass(), false, false, true);
    if (!AssetManager.GetPrimaryAssetObject(CatalogId) && !AssetManager.GetPrimaryAssetPath(CatalogId).IsNull())
    {
        const TSharedPtr<FStreamableHandle> Handle = AssetManager.LoadPrimaryAsset(CatalogId);
        if (Handle.IsValid()) Handle->WaitUntilComplete();
    }
    UPolitikaPrototypeContentCatalog* Catalog = Cast<UPolitikaPrototypeContentCatalog>(AssetManager.GetPrimaryAssetObject(CatalogId));
    if (!Catalog)
    {
        // A fixed bootstrap path is the only path-based lookup. All gameplay cross-references
        // remain stable IDs resolved through this catalog after its primary ID is registered.
        Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
            TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
        if (Catalog && Catalog->GetPrimaryAssetId() == CatalogId)
            AssetManager.AddDynamicAsset(CatalogId, FSoftObjectPath(Catalog), FAssetBundleData());
    }
    UPolitikaPrototypeContentValidator::ValidateCatalog(Catalog, OutDiagnostics);
    if (!Catalog) return false;

    UPolitikaPrototypeMapDefinition* Map = nullptr;
#if WITH_DEV_AUTOMATION_TESTS
    if (MapId == UPolitikaPrototypeContentValidator::PrototypeMapAssetId && GAutomationMapCandidate)
    {
        Map = GAutomationMapCandidate;
        GAutomationMapCandidate = nullptr; // A candidate can affect one startup attempt only.
    }
#endif
#if !UE_BUILD_SHIPPING
    const TCHAR* FixturePath = nullptr;
    if (MapId == TEXT("map.test.invalid_schema"))
        FixturePath = TEXT("/Game/Politika/TestFixtures/DA_InvalidMapSchema.DA_InvalidMapSchema");
    else if (MapId == TEXT("map.test.invalid_participant_faction"))
        FixturePath = TEXT("/Game/Politika/TestFixtures/DA_InvalidParticipantFaction.DA_InvalidParticipantFaction");
    else if (MapId == TEXT("map.test.static_obstacle_detour"))
        FixturePath = TEXT("/Game/Politika/TestFixtures/Navigation/DA_StaticObstacleDetour.DA_StaticObstacleDetour");
    else if (MapId == TEXT("map.test.disconnected_navigation"))
        FixturePath = TEXT("/Game/Politika/TestFixtures/Navigation/DA_DisconnectedNavigation.DA_DisconnectedNavigation");
    else if (MapId == TEXT("map.test.unprojectable_destination"))
        FixturePath = TEXT("/Game/Politika/TestFixtures/Navigation/DA_UnprojectableDestination.DA_UnprojectableDestination");
    if (!Map && FixturePath)
    {
        Map = LoadObject<UPolitikaPrototypeMapDefinition>(nullptr, FixturePath);
        if (!Map)
        {
            AddDiagnostic(OutDiagnostics, TEXT("TEST_FIXTURE.ASSET.MISSING"), MapId, TEXT("FixtureAsset"),
                TEXT("The explicitly requested cooked negative-test map asset could not be resolved."),
                FixturePath, TEXT("missing"),
                TEXT("Build the isolated Development negative-test package with the fixture cook directory."));
            return false;
        }
    }
#endif
    if (!Map) Map = const_cast<UPolitikaPrototypeMapDefinition*>(Catalog->FindMap(MapId));
    if (!Map)
    {
        AddDiagnostic(OutDiagnostics, TEXT("MAP.REFERENCE.UNRESOLVED"), MapId, TEXT("Catalog.Maps"),
            TEXT("The requested gameplay map definition is not present in the catalog."), TEXT("catalog map ID"), MapId.ToString(), TEXT("Add the map definition to the catalog."));
        return false;
    }
    UPolitikaPrototypeContentValidator::ValidateMapStructure(Map, Catalog, OutDiagnostics);
    if (UPolitikaPrototypeContentValidator::HasErrors(OutDiagnostics)) return false;

    OutSnapshot.Catalog = Catalog;
    OutSnapshot.MapDefinition = Map;
    return true;
}
