// Unit resolve: Discipline earned in formation (DISC-D1, RTS_UNIT_DISCIPLINE.md).
//
// Authority boundary: only the server's fixed-step simulation changes a unit's resolve, once
// every politika::discipline::kEvaluationIntervalSteps. Support is measured from the step's
// actual positions among living members of the unit's own formation, so selection, control
// groups, a neighbouring formation or a dead member never count. Clients receive their own
// units' values for the HUD and nothing else. In DISC-D1 the value drives no other attribute.

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
#if !UE_BUILD_SHIPPING
// Samples for the development log every 5 s.
constexpr uint64 ResolveSampleIntervalSteps = 100;

const TCHAR* ServiceName(pd::Service Service)
{
    switch (Service)
    {
        case pd::Service::Outside: return TEXT("outside");
        case pd::Service::MemberHolding: return TEXT("holding");
        case pd::Service::MemberEarning: return TEXT("earning");
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

void APolitikaPrototypeMatch::UpdateUnitResolve()
{
    if (SimulationStepsExecuted == 0 || SimulationStepsExecuted % pd::kEvaluationIntervalSteps != 0) return;

    // Service of every formation member this evaluation; units absent here are outside.
    TMap<uint64, pd::Service> MemberService;
    std::vector<pf::Point> Positions;
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
        for (std::size_t Index = 0; Index < Formation.members.size(); ++Index)
        {
            const pf::MemberSlot& Member = Formation.members[Index];
            // Independent: an individual order took the member off the formation's current move.
            const bool bIndependent = Formation.hasDestination && !Member.following;
            const bool bSupported = pd::SupportCount(Positions, Index) >= Required;
            MemberService.Add(Member.id, !bIndependent && bSupported ? pd::Service::MemberEarning : pd::Service::MemberHolding);
        }
    }

#if !UE_BUILD_SHIPPING
    const bool bSample = SimulationStepsExecuted % ResolveSampleIntervalSteps == 0;
    TMap<uint32, FResolveTally> ByFormation;
    TMap<uint32, FResolveTally> ByParticipant;
    TMap<uint32, FResolveTally> OutsideByParticipant;
#endif
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit) || !Unit->IsAlive()) continue;
        const pd::Service* Found = MemberService.Find(Unit->GetStableId());
        // Only military units serve; Form refuses civilians, so this is a safeguard.
        const pd::Service Service = Found && Unit->IsMilitary() ? *Found : pd::Service::Outside;
        const int32 Before = Unit->GetResolveMilli();
        Unit->ApplyResolve(pd::Evaluate(Before, Service));
#if !UE_BUILD_SHIPPING
        const int32 After = Unit->GetResolveMilli();
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
