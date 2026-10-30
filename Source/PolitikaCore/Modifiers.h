#pragma once

// Unit attribute modifiers (RTS_UNIT_DISCIPLINE.md section 7, DISC-D3).
//
// One registry per unit lists every system that scales one of the unit's attributes, by source.
// Each source holds exactly one multiplier per attribute in integer permille (1000 = no change),
// and setting it again replaces the previous one, so a source such as Discipline can never stack
// with itself. The consuming system reads the combined multiplier of all sources, once per use.

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace politika::modifiers {

// Wire and log values are stable; append new attributes at the end.
enum class Attribute : std::uint8_t {
    // Damage taken from every weapon hit, after armour and before health.
    DamageTaken,
    // Indirect-fire scatter radius. Direct fire never rolls (UNIT_SPECIFICATION_FRAMEWORK.md 6.6),
    // and no indirect-fire weapon exists yet, so nothing consumes it.
    Spread,
    // Time between aimed shots.
    FireInterval,
    // Magazine reload time.
    ReloadTime,
    // Movement speed while walking back to the stance post.
    ReturnSpeed,
    // Delay before a formation closes its ranks after a loss.
    RegroupDelay,
};
inline constexpr std::size_t kAttributeCount = 6;

enum class Source : std::uint8_t { Discipline };
inline constexpr std::size_t kSourceCount = 1;

inline constexpr std::int32_t kNeutral = 1000;

[[nodiscard]] constexpr std::string_view AttributeName(Attribute attribute) noexcept {
    switch (attribute) {
        case Attribute::DamageTaken: return "damage_taken";
        case Attribute::Spread: return "spread";
        case Attribute::FireInterval: return "fire_interval";
        case Attribute::ReloadTime: return "reload_time";
        case Attribute::ReturnSpeed: return "return_speed";
        case Attribute::RegroupDelay: return "regroup_delay";
    }
    return "invalid";
}

[[nodiscard]] constexpr std::string_view SourceName(Source source) noexcept {
    switch (source) {
        case Source::Discipline: return "discipline";
    }
    return "invalid";
}

class Registry {
public:
    constexpr Registry() noexcept {
        for (auto& bySource : permille_) bySource.fill(kNeutral);
    }

    // Replaces `source`'s multiplier for `attribute`. Negative values are treated as zero.
    constexpr void Set(Source source, Attribute attribute, std::int32_t permille) noexcept {
        permille_[Index(source)][Index(attribute)] = permille < 0 ? 0 : permille;
    }

    constexpr void Clear(Source source) noexcept { permille_[Index(source)].fill(kNeutral); }

    [[nodiscard]] constexpr std::int32_t Of(Source source, Attribute attribute) const noexcept {
        return permille_[Index(source)][Index(attribute)];
    }

    // Product of every source's multiplier, rounded to the nearest permille.
    [[nodiscard]] constexpr std::int32_t Combined(Attribute attribute) const noexcept {
        std::int64_t combined = kNeutral;
        for (const auto& bySource : permille_)
            combined = (combined * bySource[Index(attribute)] + kNeutral / 2) / kNeutral;
        return static_cast<std::int32_t>(combined);
    }

private:
    template <class Enum>
    [[nodiscard]] static constexpr std::size_t Index(Enum value) noexcept { return static_cast<std::size_t>(value); }

    std::array<std::array<std::int32_t, kAttributeCount>, kSourceCount> permille_{};
};

} // namespace politika::modifiers
