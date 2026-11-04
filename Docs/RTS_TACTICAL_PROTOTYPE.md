# Prototype garrison, crush, concealment and building selection

**Status: implemented for owner playtesting. Not accepted; the [TAC-1 record](RTS_TACTICAL_TAC1_QUALIFICATION.md) says what has been tested.** All numbers marked as fixtures are placeholders, not balance.

Rules are in `Source/PolitikaCore/Tactical.h`; the game code is in `Source/Politika/Private/PolitikaTactical.cpp`. `network_protocol` went from 26 to 27 for the new requests and replicated state.

## Controls

| Input | Effect |
| --- | --- |
| Left-click a producer, generator or works | Selects it, with a ring and a line on the HUD. Resource sources cannot be clicked. A unit in it or behind it still wins the click. Selecting a unit ends it. Enemy buildings can be selected to read them. |
| **U**, soldiers selected, cursor on your building | The soldiers walk in. |
| **U**, your garrisoned building selected | Everyone inside comes out. |
| **M**, soldiers selected | Orders them to conceal themselves, or withdraws that order. |

Right-clicking your own healthy building stays a move, because the accepted `BLD` session depends on it, so garrisoning has its own key.

## Rules

**Garrison** (specification section 6.9, U-08). An armed non-vehicle enters its owner's standing producer, generator or captured works within 200 cm (fixture), up to 5 (fixture). Inside, it fires at +15 % range on what comes within reach, under its stance, and it cannot be hit or selected. Only its own team observes it. It comes out where it went in: on U, at 20 % building health, when the building is lost or captured, or when it routs. A formation member must leave its formation first.

Section 6.9's damage split is not implemented, because the game has no damage types: occupants take no damage, and the building takes the fire.

**Crush** (specification U-07). A vehicle that moves over an enemy non-vehicle within 90 cm of its path (fixture) kills it, with the same hostility as automatic fire, so never an ally or the neutral Unaligned. The vehicles are the harvester, recon vehicle, mechanized fireteam and opposing patrol. A harvester on its routine does not crush: its route runs through bases without aiming at anyone, and it killed soldiers standing there, so it crushes only once a player's order has taken it off the routine. U-07 recommended deferring this until there was navigation evidence; the owner asked for it. Units do not collide, so a vehicle never slows or turns for a soldier, and the risk U-07 names is unchanged: pathing does not aim at or avoid anyone.

**Concealment** (specification section 9.1). There is no invisibility. A soldier under the order who has stood still for 3 s is observed only within `max(250 cm, 35 % of the observer's sight)`. Firing or being hit reveals it for 4 s, and moving faster than half speed breaks it. While under the order and idle it holds its fire, but an explicit attack order still fires. Concealment also limits targeting in the fogless offline sandbox. Nothing conceals on its own, which leaves every existing session unchanged. No `DETECT.CONCEALED` detector exists yet.

## Cues

These events have no authored sound, so they are silent. Each one logs `RTS_TACTICAL_CUE` where a sound would play, and the packaged session checks that it reaches the right client.

| Event | Cue | Reaches |
| --- | --- | --- |
| Building selected | `Select` | The player selecting it |
| Garrison ordered | `Garrison` | The player ordering it |
| Soldier enters or leaves | `Enter`, `Exit` | Anyone observing the building |
| Concealed, revealed | `StealthOn`, `StealthOff` | Anyone who can see the soldier at that moment |
| Crush | `Crush`, at most once in 2 s per vehicle | Anyone observing the vehicle |

## Reference evidence

The only Tempest Rising evidence for these features is the tag row already in the specification's reference table (`DefinitionTags.ini` lines 479, 499, 523: `PassiveAbility.Detection`, `.Marking`, `.Stealth`, tags only, redefined as concealment with no invisibility). No further Tempest evidence was searched for or used. The rule numbers come from POLITIKA's own specification.

## Verification

- Portable rules: `tools/core-test.sh`.
- Authority rules: Automation `Politika.RTS.Multiplayer.Tactical`.
- Real input, networked, rendered clients: `rts_multiplayer --tactical`.

Results belong to the package tested and are recorded with the run, not here. Not covered: a listen-server host, a captured building ejecting its occupants in a packaged run, the sandbox's vehicles (recon, fireteam, patrol), and whether the fixture numbers play well.
