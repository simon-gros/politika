// Unit resolve: Discipline earned in formation (DISC-D1) and its stress and faction rules
// (DISC-D2, RTS_UNIT_DISCIPLINE.md).
//
// Authority boundary: only the server's fixed-step simulation changes a unit's resolve: once
// every politika::discipline::kEvaluationIntervalSteps, and at a formation member's death for
// casualty shock. Support is measured from the step's actual positions among living members of
// the unit's own formation, so selection, control groups, a neighbouring formation or a dead
// member never count. Isolation is measured among living friendly soldiers (same owner, or same
// alliance team). Clients receive their own units' values for the HUD and nothing else. The
// value drives no other attribute yet.

#include "PolitikaPrototypeGameplay.h"

#include "PolitikaBootstrap.h"
#include "Discipline.h"
#include "Formation.h"

#include <limits>
#include <vector>

namespace pd = politika::discipline;
namespace pf = politika::formation;

namespace
{
// Content faction IDs of the playable factions and the Unaligned (UNIT_SPECIFICATION_FRAMEWORK.md).
// Prototype test factions have no faction rules and use the Standard doctrine.
pd::Doctrine DoctrineOfFactionId(FName FactionId)
{
    using politika::rules::Faction;
    static const TMap<FName, Faction> Known = {
        {FName(TEXT("faction.atlantic_concord")), Faction::AtlanticConcord},
        {FName(TEXT("faction.red_directorate")), Faction::RedDirectorate},
        {FName(TEXT("faction.iron_ascendancy")), Faction::IronAscendancy},
        {FName(TEXT("faction.meridian_pact")), Faction::MeridianPact},
        {FName(TEXT("faction.unaligned")), Faction::Unaligned},
    };
    const Faction* Found = Known.Find(FactionId);
    return Found ? pd::DoctrineOf(*Found) : pd::Doctrine::Standard;
}

bool AreComrades(const APolitikaPrototypeUnit& A, const APolitikaPrototypeUnit& B)
{
    return A.GetOwnerPlayer() == B.GetOwnerPlayer()
        || (A.GetTeamId() != politika::rules::kNoTeam && A.GetTeamId() == B.GetTeamId());
}

#if !UE_BUILD_SHIPPING
const TCHAR* DoctrineName(pd::Doctrine Doctrine)
{
    switch (Doctrine)
    {
        case pd::Doctrine::Standard: return TEXT("standard");
        case pd::Doctrine::MassFormation: return TEXT("mass");
        case pd::Doctrine::SoloTolerant: return TEXT("solo");
    }
    return TEXT("invalid");
}

// Samples for the development log every 5 s.
constexpr uint64 ResolveSampleIntervalSteps = 100;

const TCHAR* ServiceName(pd::Service Service)
{
    switch (Service)
    {
        case pd::Service::Outside: return TEXT("outside");
        case pd::Service::MemberHolding: return TEXT("holding");
        case pd::Service::MemberEarning: return TEXT("earning");
        case pd::Service::Isolated: return TEXT("isolated");
    }
    return TEXT("invalid");
}

struct FResolveTally
{
    int32 Count = 0;
    int32 Earning = 0;
    int32 Holding = 0;
    int32 Min = std::numeric_limits<int32>::max();
    int32 Max = 0;
    int64 Sum = 0;
    void Add(int32 Value, pd::Service Service)
    {
        ++Count;
        Earning += Service == pd::Service::MemberEarning ? 1 : 0;
        Holding += Service == pd::Service::MemberHolding ? 1 : 0;
        Min = FMath::Min(Min, Value);
        Max = FMath::Max(Max, Value);
        Sum += Value;
    }
    int32 Average() const { return Count > 0 ? static_cast<int32>(Sum / Count) : 0; }
};
#endif
} // namespace

pd::Doctrine APolitikaPrototypeMatch::DisciplineDoctrineOf(const APolitikaPrototypeUnit& Unit)
{
#if !UE_BUILD_SHIPPING
    if (!bDisciplineDoctrineOverridesParsed)
    {
        bDisciplineDoctrineOverridesParsed = true;
        FString Spec;
        if (FParse::Value(FCommandLine::Get(), TEXT("PolitikaMPDisciplineDoctrine="), Spec))
        {
            TArray<FString> Entries;
            Spec.ParseIntoArray(Entries, TEXT(","));
            for (const FString& Entry : Entries)
            {
                FString Participant, Name;
                if (!Entry.Split(TEXT(":"), &Participant, &Name)) continue;
                const uint32 Id = static_cast<uint32>(FCString::Atoi(*Participant));
                TOptional<pd::Doctrine> Parsed;
                if (Name == TEXT("standard")) Parsed = pd::Doctrine::Standard;
                else if (Name == TEXT("mass")) Parsed = pd::Doctrine::MassFormation;
                else if (Name == TEXT("solo")) Parsed = pd::Doctrine::SoloTolerant;
                // Automation entries set before the first lookup win.
                if (!Parsed || DisciplineDoctrineOverrides.Contains(Id)) continue;
                DisciplineDoctrineOverrides.Add(Id, *Parsed);
                UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_DISCIPLINE_DOCTRINE_OVERRIDE participant=%u doctrine=%s"),
                    Id, DoctrineName(*Parsed));
            }
        }
    }
#endif
    if (const pd::Doctrine* Override = DisciplineDoctrineOverrides.Find(Unit.GetOwnerPlayer())) return *Override;
    return DoctrineOfFactionId(Unit.GetFactionId());
}

void APolitikaPrototypeMatch::ApplyCasualtyShock(const APolitikaPrototypeUnit& Fallen)
{
    const pf::FormationId Id = Formations.FormationOf(Fallen.GetStableId());
    const pf::Formation* Formation = Formations.Find(Id);
    if (!Formation) return;
    const FVector Where = Fallen.GetActorLocation();
    for (const pf::MemberSlot& Member : Formation->members)
    {
        if (Member.id == Fallen.GetStableId()) continue;
        APolitikaPrototypeUnit* Unit = FindUnitByStableId(Member.id);
        // Members falling in the same step are not shocked by each other.
        if (!Unit || !Unit->IsAlive()) continue;
        if (FVector::DistSquared2D(Unit->GetActorLocation(), Where) > FMath::Square(pd::kCasualtyShockRadiusCm))
        {
#if !UE_BUILD_SHIPPING
            // Out of range: listed so a packaged run can check the radius from both sides.
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_DISCIPLINE_SHOCK casualty=%llu unit=%llu participant=%u formation=%u in_range=0 distance=%.1f before=%d after=%d applied=0 step=%llu"),
                Fallen.GetStableId(), Unit->GetStableId(), Unit->GetOwnerPlayer(), Id.value,
                FVector::Dist2D(Unit->GetActorLocation(), Where), Unit->GetResolveMilli(), Unit->GetResolveMilli(), SimulationStepsExecuted);
#endif
            continue;
        }
        const int32 Before = Unit->GetResolveMilli();
        const pd::ShockResult Shock = pd::ApplyCasualtyShock(Before, Unit->ResolveShock, SimulationStepsExecuted);
        Unit->ResolveShock = Shock.window;
        if (Shock.applied > 0) Unit->ApplyResolve({Shock.value, pd::Trend::Fading});
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_DISCIPLINE_SHOCK casualty=%llu unit=%llu participant=%u formation=%u in_range=1 distance=%.1f before=%d after=%d applied=%d window_start=%llu window_taken=%d step=%llu"),
            Fallen.GetStableId(), Unit->GetStableId(), Unit->GetOwnerPlayer(), Id.value,
            FVector::Dist2D(Unit->GetActorLocation(), Where), Before, Shock.value, Shock.applied, Shock.window.start,
            Shock.window.taken, SimulationStepsExecuted);
        if (const std::optional<std::int32_t> Band = pd::CrossedBand(Before, Shock.value))
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_DISCIPLINE_BAND unit=%llu participant=%u band=%d direction=fading value=%d service=shock formation=%u step=%llu"),
                Unit->GetStableId(), Unit->GetOwnerPlayer(), *Band, Shock.value, Id.value, SimulationStepsExecuted);
#endif
    }
}

void APolitikaPrototypeMatch::UpdateUnitResolve()
{
    if (SimulationStepsExecuted == 0 || SimulationStepsExecuted % pd::kEvaluationIntervalSteps != 0) return;

    // Service and ceiling of every formation member this evaluation; units absent here are outside.
    struct FMemberEvaluation
    {
        pd::Service Service = pd::Service::MemberHolding;
        int32 Ceiling = pd::kNormalCeiling;
    };
    TMap<uint64, FMemberEvaluation> MemberService;
    std::vector<pf::Point> Positions;
    std::vector<uint64> Earning;
    for (const pf::Formation& Formation : Formations.All())
    {
        Positions.clear();
        Positions.reserve(Formation.members.size());
        for (const pf::MemberSlot& Member : Formation.members)
        {
            const APolitikaPrototypeUnit* Unit = FindUnitByStableId(Member.id);
            if (Unit && Unit->IsAlive())
                Positions.push_back({Unit->GetActorLocation().X, Unit->GetActorLocation().Y});
            else
                Positions.push_back({std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()});
        }
        const std::size_t Required = pd::RequiredSupport(Formation.members.size());
        Earning.clear();
        for (std::size_t Index = 0; Index < Formation.members.size(); ++Index)
        {
            const pf::MemberSlot& Member = Formation.members[Index];
            // Independent: an individual order took the member off the formation's current move.
            const bool bIndependent = Formation.hasDestination && !Member.following;
            const bool bSupported = pd::SupportCount(Positions, Index) >= Required;
            const bool bEarning = !bIndependent && bSupported;
            MemberService.Add(Member.id, {bEarning ? pd::Service::MemberEarning : pd::Service::MemberHolding});
            if (bEarning) Earning.push_back(Member.id);
        }
        // The mass-formation ceiling counts this evaluation's earning members only. Members of one
        // formation share an owner and therefore a doctrine.
        const APolitikaPrototypeUnit* Representative = Formation.members.empty() ? nullptr : FindUnitByStableId(Formation.members.front().id);
        const pd::Doctrine Doctrine = Representative ? DisciplineDoctrineOf(*Representative) : pd::Doctrine::Standard;
        const int32 Ceiling = pd::FormationCeiling(Doctrine, Earning.size());
        for (const uint64 MemberId : Earning) MemberService.FindChecked(MemberId).Ceiling = Ceiling;
#if !UE_BUILD_SHIPPING
        if (Ceiling != pd::kNormalCeiling && SimulationStepsExecuted % 100 == 0)
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_DISCIPLINE_MASS formation=%u participant=%u doctrine=%s earning=%d ceiling=%d step=%llu"),
                Formation.id.value, Formation.owner, DoctrineName(Doctrine), static_cast<int32>(Earning.size()), Ceiling,
                SimulationStepsExecuted);
#endif
    }

    // Soldiers outside any formation with no living friendly soldier within the support radius.
    // Each lone unit stops at its first comrade, so the scan is cheap when units stand together.
    auto IsAlone = [this](const APolitikaPrototypeUnit& Unit)
    {
        const FVector Location = Unit.GetActorLocation();
        const double RadiusSquared = FMath::Square(pd::kSupportRadiusCm);
        for (const APolitikaPrototypeUnit* Other : Units)
        {
            if (Other == &Unit || !IsValid(Other) || !Other->IsAlive() || !Other->IsMilitary() || !AreComrades(Unit, *Other)) continue;
            if (FVector::DistSquared2D(Other->GetActorLocation(), Location) <= RadiusSquared) return false;
        }
        return true;
    };

#if !UE_BUILD_SHIPPING
    const bool bSample = SimulationStepsExecuted % ResolveSampleIntervalSteps == 0;
    TMap<uint32, FResolveTally> ByFormation;
    TMap<uint32, FResolveTally> ByParticipant;
    TMap<uint32, FResolveTally> OutsideByParticipant;
#endif
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit) || !Unit->IsAlive()) continue;
        const FMemberEvaluation* Found = MemberService.Find(Unit->GetStableId());
        // Only military units serve; Form refuses civilians, so this is a safeguard.
        pd::Service Service = Found && Unit->IsMilitary() ? Found->Service : pd::Service::Outside;
        bool bAlone = false;
        pd::Doctrine Doctrine = pd::Doctrine::Standard;
        if (Service == pd::Service::Outside && Unit->IsMilitary())
        {
            bAlone = IsAlone(*Unit);
            if (bAlone)
            {
                Doctrine = DisciplineDoctrineOf(*Unit);
                if (pd::IsolationApplies(Doctrine, pd::UnitCategory::Military)) Service = pd::Service::Isolated;
            }
        }
        const int32 Before = Unit->GetResolveMilli();
        Unit->ApplyResolve(pd::Evaluate(Before, Service, Found ? Found->Ceiling : pd::kNormalCeiling));
#if !UE_BUILD_SHIPPING
        const int32 After = Unit->GetResolveMilli();
        // Lone soldiers are few; list them, exempt or not, so isolation can be checked per unit.
        if (bAlone && SimulationStepsExecuted % ResolveSampleIntervalSteps == 0)
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_DISCIPLINE_ISOLATED unit=%llu participant=%u doctrine=%s exempt=%d value=%d step=%llu"),
                Unit->GetStableId(), Unit->GetOwnerPlayer(), DoctrineName(Doctrine), Service == pd::Service::Isolated ? 0 : 1,
                After, SimulationStepsExecuted);
        if (const std::optional<std::int32_t> Band = pd::CrossedBand(Before, After))
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_DISCIPLINE_BAND unit=%llu participant=%u band=%d direction=%s value=%d service=%s formation=%u step=%llu"),
                Unit->GetStableId(), Unit->GetOwnerPlayer(), *Band, After > Before ? TEXT("rising") : TEXT("fading"), After,
                ServiceName(Service), Formations.FormationOf(Unit->GetStableId()).value, SimulationStepsExecuted);
        if (bSample && Unit->GetOwnerPlayer() != 0)
        {
            const pf::FormationId Formation = Formations.FormationOf(Unit->GetStableId());
            if (Formation) ByFormation.FindOrAdd(Formation.value).Add(After, Service);
            else OutsideByParticipant.FindOrAdd(Unit->GetOwnerPlayer()).Add(After, Service);
            ByParticipant.FindOrAdd(Unit->GetOwnerPlayer()).Add(After, Service);
            // Members that are not earning are few; list them so a hold can be checked per unit.
            if (Service == pd::Service::MemberHolding)
                UE_LOG(LogPolitikaStartup, Display,
                    TEXT("RTS_DISCIPLINE_HOLDING unit=%llu formation=%u value=%d step=%llu"),
                    Unit->GetStableId(), Formation.value, After, SimulationStepsExecuted);
        }
#endif
    }
#if !UE_BUILD_SHIPPING
    if (!bSample) return;
    ByFormation.KeySort(TLess<uint32>());
    for (const TPair<uint32, FResolveTally>& Entry : ByFormation)
    {
        const pf::Formation* Formation = Formations.Find(pf::FormationId{Entry.Key});
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_DISCIPLINE_SAMPLE formation=%u participant=%u members=%d earning=%d holding=%d min=%d avg=%d max=%d step=%llu"),
            Entry.Key, Formation ? Formation->owner : 0u, Entry.Value.Count, Entry.Value.Earning, Entry.Value.Holding,
            Entry.Value.Min, Entry.Value.Average(), Entry.Value.Max, SimulationStepsExecuted);
    }
    ByParticipant.KeySort(TLess<uint32>());
    for (const TPair<uint32, FResolveTally>& Entry : ByParticipant)
    {
        const FResolveTally* Outside = OutsideByParticipant.Find(Entry.Key);
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_DISCIPLINE_PARTICIPANT participant=%u units=%d members=%d earning=%d outside=%d outside_min=%d outside_avg=%d outside_max=%d min=%d avg=%d max=%d step=%llu"),
            Entry.Key, Entry.Value.Count, Entry.Value.Count - (Outside ? Outside->Count : 0), Entry.Value.Earning,
            Outside ? Outside->Count : 0, Outside ? Outside->Min : 0, Outside ? Outside->Average() : 0, Outside ? Outside->Max : 0,
            Entry.Value.Min, Entry.Value.Average(), Entry.Value.Max, SimulationStepsExecuted);
    }
#endif
}
