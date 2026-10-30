# FORM-F4 — Combat stances and formation adherence: acceptance record

**Date:** 30 October 2026 · **Scope:** the FORM-F4 increment of [Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) §5 and §10: four combat stances on any own unit, a formation stance with per-unit overrides, leashed idle engagement with return to post, explicit-order precedence over Hold Fire, and Flexible / Maintain / Free adherence. Tested on the multiplayer skirmish test map at its owner cap of 20 units per participant. Withdraw, Cohesion and Discipline effects (FORM-F5 to F8) are out of scope and do not exist.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map; packaged formation-stance PASS (14/14, two runs with an identical engagement timeline), Automation 41/41, regression matrix unchanged from the FORM-F3 baseline, and the protocol-3 client is now refused.**

## A. What changed (commit `677560f`, harness fix `2273558`)

- **Stance decides what an idle unit does on its own; it is not an order.** `Source/PolitikaCore/Stance.h` holds the rules. **Hold Ground** (wire value 0 and the default) is the behaviour before stances existed: acquire and fire at enemies in weapon range, never move. **Guard** chases an enemy it can hit from within 600 cm of its post. **Aggressive** chases within 1500 cm and also looks out to that distance, still only at enemies its team sees. **Hold Fire** never acquires a target and never fires on its own, even when shot at.
- **Explicit orders take precedence and keep the stance.** Attack, Attack-move and the existing Guard *order* fire whatever the stance, so an explicit Attack on a Hold Fire unit fires at that target. A stance change never replaces the current order, and an order never clears the stance. The Guard order and the Guard stance are separate interfaces (§5).
- **Precedence.** Effective stance = the unit's own override, else its formation's stance, else the default. **H** with exactly one whole formation selected sets the formation's stance and clears members' overrides. On any other selection it sets each selected unit's own stance. Form and Release never change a unit's effective stance: when a unit joins or leaves a formation, its override is re-derived from the stance it had before (`OverrideFor`). A formation formed from members that share one stance takes that stance without overrides. A member that survives its formation's dissolution keeps the formation stance as its own.
- **Leashed chase and return to post.** An idle Guard or Aggressive unit records its post when it goes idle. It engages only a target it can fire at from inside its leash (`WithinReach`: post-to-target ≤ leash + weapon range), so it never chases out, gives up and walks back forever. The chase destination is clamped to the leash circle. When the target is lost, dies or leaves the leash, the unit disengages and walks back to its post unless it is already within 50 cm. A chase whose route fails backs off for 40 authoritative steps (2 s) before one retry, so a blocked chase cannot cause a path-query storm. Autonomous chase and return never become orders, never make a formation member independent and never end or replace an order.
- **Adherence (J, whole formation only).** **Flexible** (default) uses the stance's full leash and returns to the slot. **Maintain** halves the leash (Guard 300 cm, Aggressive 750 cm). **Free** uses the full leash, stays where a fight ends and gives up closing ranks: a pending regroup is cancelled and losses schedule none, while membership is kept. J on anything other than one whole formation sends nothing.
- **Network.** Stance and adherence travel as two new ops (`SetStance`, `SetAdherence`) on the existing formation intent, with a value byte; a stance addresses a formation or up to the release bound of unit IDs, adherence a formation only. The same `FormationRejection` codes apply (malformed, duplicate, unauthorized, unknown formation and so on). A rejected request changes nothing. The owner view carries each formation's stance and adherence; the opposing client receives nothing. `network_protocol` moves from 3 to 4; a protocol-3 client is refused at join.
- **HUD.** The selected-formation line shows the stance and adherence, and a single unit's line shows `STANCE <name>` with `(own)` when it is the unit's own override rather than inherited. The formation's following count now reads `N following its move` instead of `N in formation`, which read `0 in formation` right after Form (FORM-F3 open item). The help line adds `H stance · J adherence`.
- **Input.** `PrototypeCycleStance` (H) and `PrototypeCycleAdherence` (J) in `DefaultInput.ini`; no shared chord with existing bindings.
- **Development logs.** `RTS_STANCE_SET`, `RTS_STANCE_APPLIED`, `RTS_STANCE_ENGAGE`, `RTS_STANCE_DISENGAGE`, `RTS_STANCE_RETURN` (start, arrived, failed). The Development-only fixture switch `-PolitikaMPStanceProbe` places one enemy probe unit next to the formation on the harness's cue and logs `MP_STANCE_PROBE`.

## B. Verification

**Identity.** Packages from clean commit `677560f` (`git_dirty=false`, `network_protocol` 4, `game_version` 0.1.0-dev.0). `2273558` changes only `Tools/Shell/src/bin/rts_multiplayer.rs`, so the game source is identical; the harness binaries used for the evidence below are built from `2273558`. Client `dev-677560f/…/Politika` `f1e0a0f135cc4b5e7d2d204decb55827b9c734e4e8a7217a6a37c736bbf83741`; server `server-677560f/…/PolitikaServer` `94198330e26a695e21c353526c84e487b2ffdaaa5b365fc1e86cec3654ba3232`. Evidence: `/mnt/data/politika-qualification/FORM-F4-20261030/final-677560f/`.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`): stance/adherence cycle order and wire validity, precedence and `OverrideFor`, `BehaviorFor` per stance × adherence (leash, acquire, return), `WithinReach` and `ClampToLeash`, SetStance/SetAdherence intent shape (exactly one address, adherence needs a formation, duplicate IDs), Free cancels a pending regroup and schedules none on loss | PASS (`core-test.out`) |
| `quality -- audit` (`input.bindings`, no shared chord) | PASS: 26 order bindings share no key with 5 axis keys (`quality-audit.out`) |
| Editor build | exit 0 |
| Automation, focused: new `Politika.RTS.Multiplayer.FormationStance` plus `FormationLifecycle`, `GroupMoveShape`, `GroupSelectionAndOrders`, `ControlGroups` | 5/5 (`auto-focused-1`, and `auto-focused-2` after the last source change) |
| Automation, all Politika tests (`auto-all/`) | 41/41 (40 earlier + `FormationStance`); state and warning count identical per test to the FORM-F3 baseline. The new test has one warning, the SDL start-up line |
| Packaged `rts_multiplayer --formation-stance`, two runs | 14/14 and 14/14 (`formation-stance-1/`, `-2/`) |
| Regression matrix (`final-677560f/summary.txt`) | Unchanged from the FORM-F3 baseline: formation lifecycle 20/20, group-move shape 16/16, group orders 8/8, control groups 17/17, unit-scale 73/73, offline PAC-10 and participant authority PASS, dedicated 1v1 23/23, listen 1v1 18/18, listen layouts 74, lag-loss 47, engagement 9/9, fog memory 7/7. Dedicated layouts 75/75: the run supplied the FORM-F3 protocol-3 client and `MP2-PROTOCOL-LEGACY-REFUSED` passes (`MP_PRELOGIN REJECTED … protocol=3 … MP.PROTOCOL.MISMATCH`). The only non-PASS lines are the three planned NOT_RUNs (legacy client not supplied to listen layouts and lag-loss, no allies in 1v1) |

**Packaged formation-stance run** (dedicated 1v1, 20 units per seat, fixture held idle, client A rendered on Xvfb at 1280×720 and driven with real X keyboard and mouse input through xdotool):

| Verdict | What it shows |
|---|---|
| `FORM-FIXTURE-20-PER-SEAT` | 20 own units on the seat. |
| `FORM-F4-DEFAULTS` | T forms formation 1 (Line, 20 members); the owner view reads `stance=hold_ground:adherence=flexible`. |
| `FORM-F4-SETUP-LINE-ARRIVED` | A formation Move puts 20/20 at their slots, 0 navigation failures, minimum separation 149.3 cm. |
| `FORM-F4-STANCE-IS-NOT-AN-ORDER` | H sets the formation to Guard: one accepted stance outcome, `orders=0`, every member's route untouched. |
| `FORM-F4-GUARD-ENGAGES` | The probe enemy appears 741 cm from the nearest member. The four members that can reach it from inside their 600 cm leash (10, 13, 20, 22) engage and kill it. |
| `FORM-08-GUARD-LEASH-BOUNDED` | Four disengagements; the largest distance any chaser reached from its post was 136.5 cm (leash 600). |
| `FORM-F4-GUARD-RETURNS-TO-POST` | The four chasers walk back; 20/20 living members end within 30 cm of their post. |
| `FORM-07-AUTONOMY-IS-NOT-AN-ORDER` | No unit order was issued during the engagement and no member became independent. |
| `FORM-F4-ADHERENCE-CYCLES` | J three times: Maintain, Free, Flexible, each replicated to the owner view for all 20 members. |
| `FORM-F4-HOLD-FIRE-HOLDS` | H to Hold Fire, then a second probe 451 cm away fires on the formation for 5 s: 25 probe shots, 0 shots and 0 chases from the formation, which takes a loss (unit 10) without retaliating. |
| `FORM-F4-HOLD-GROUND-FIRES-AGAIN` | H to Hold Ground: the formation fires again (32 shots) and kills the probe without moving (0 chases). |
| `FORM-F4-OWNER-ONLY` | The opposing client logs no formation or stance line. |
| `FORM-MATCH-STAYS-ACTIVE` | No objective capture or terminal phase during the run. |
| `FORM-15-TERMINAL-RETIRES-ONCE` | Ending the match clears formation 1 once (`match_ended`). |

Screenshots (`formation-stance-*/client-a-*.png`): the Guard engagement, the line back at its posts, and Hold Fire under fire with the HUD stance line.

**Repeatability.** The autonomous engagement is identical in both runs: probe at step 334, engagements at steps 468, 474, 482, 491, all four disengagements at 517 and arrivals at 527–530, with the same displacements. Events triggered by real keyboard input (the formation Move, the J and H presses) are 1–2 steps apart between runs, because input arrives at a slightly different step each time. This is a same-machine check, not cross-machine proof.

**Harness fix and superseded attempt.** The first stance run (`formation-stance-attempt0-early-judgement/`, 12 PASS, 2 FAIL) judged the leash and the return one step after the probe died, before the chasers had disengaged and walked back. Harness fix `2273558` (Tools/Shell only) waits until every chaser has disengaged and arrived; the same packages then passed 14/14 twice. The session running the matrix was interrupted during lag-loss; that partial run is kept as `profile-lag-loss-attempt0-interrupted/` and the step was rerun from the start against the same packages.

## C. Map to the FORM acceptance matrix

| ID | Status in this increment |
|---|---|
| FORM-01 | The enemy cannot set another participant's stance (Automation); adherence uses the same owned-formation lookup, so another participant's formation is an unknown formation. The opposing client sees no stance (packaged). |
| FORM-03 | Unknown stance or adherence values, adherence without a formation, two addresses and duplicate IDs fail closed and change nothing (core, Automation); a protocol-3 client is refused at join (packaged `matrix-none`). |
| FORM-07 | Autonomous chase and return issue no order, make no member independent and never end or replace an order (Automation, packaged). |
| FORM-08 | Chase is bounded by the leash (packaged maximum 136.5 cm of 600), a failed chase backs off 40 steps before one retry (Automation), and units return once to their post. |
| FORM-09 | Form, Release and loss keep each unit's effective stance; a survivor of a dissolved formation keeps the formation stance as its own (Automation). |
| FORM-10 | Shape, stance, adherence and an explicit order are independent: a stance change replaces no order, an explicit Attack fires under Hold Fire and leaves the stance in place, and the HUD shows each one (Automation, packaged). |
| FORM-15 | Terminal retires the formation, and its stance with it, once (packaged). |
| FORM-16 | Two packaged runs produced the identical autonomous engagement timeline; input-driven events differ by 1–2 steps. Same machine only. |
| FORM-18 | Rendered cooked Linux Development client shows the stance and adherence lines (screenshots). |
| FORM-11–14, 17, 19, 20 | Not in scope: Cohesion, faction effects, AI and scale/device work (FORM-F5+). |

## D. Decisions taken for §5 and §12 (provisional)

- **Hold Fire and explicit Attack:** an explicit Attack, Attack-move or Guard order fires under Hold Fire; Hold Fire only suppresses autonomous acquisition, including retaliation.
- **Default stance:** Hold Ground, so existing idle behaviour is unchanged for every unit and every earlier test.
- **Leashes:** Guard 600 cm, Aggressive 1500 cm (also its acquisition range), Maintain halves both, return tolerance 50 cm, chase back-off 40 steps. These are first values for the skirmish map, not balance.
- **Adherence semantics:** Maintain is a tighter leash, Flexible the full leash with return, Free no return and no regroup. Local cover seeking and slot restoration in transit (§5 Flexible) do not exist yet.
- **Override storage:** per-unit overrides live on the unit, the formation stance on the registry entry; Form/Release re-derive overrides so the effective stance never changes silently.
- **Save/net schema:** stances are not saved; `network_protocol` 4.

## E. Not covered

- **Withdraw** and any retreat destination or cornered-unit policy.
- **Guard stance around an assigned area or unit.** Guard defends the unit's own post; defending another unit or a marked area needs the future assignment interface.
- **Aggressive and Maintain in the packaged run.** Both go through the same leash code and are covered by core and Automation; the packaged run drives Guard, Hold Fire and Hold Ground, and only cycles adherence.
- **Per-unit stance in the packaged run.** H on a partial selection is covered by Automation only.
- **A stance panel or per-unit stance icons**; H/J cycle and the HUD reports the result.
- **Chase across fog.** Acquisition uses what the unit's team sees; there is no memory-based pursuit.
- **Formation speed matching, Reinforce, facing/width drag** (FORM-F3 open items) are unchanged.
