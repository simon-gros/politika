//! Bounded observer of Unreal measurements, not a path planner or movement driver.
use std::{
    collections::BTreeMap,
    env, fs,
    path::{Path, PathBuf},
    process::{Command, Stdio},
    thread,
    time::{Duration, Instant, SystemTime, UNIX_EPOCH},
};

type Record<'a> = BTreeMap<&'a str, &'a str>;
type Point = [f64; 3];
fn records<'a>(log: &'a str, marker: &str) -> Vec<Record<'a>> {
    log.lines()
        .filter_map(|line| {
            let (_, tail) = line.split_once(&format!("{marker} "))?;
            Some(
                tail.split_whitespace()
                    .filter_map(|field| field.split_once('='))
                    .collect(),
            )
        })
        .collect()
}
fn field<'a>(r: &Record<'a>, key: &str) -> Result<&'a str, String> {
    r.get(key)
        .copied()
        .ok_or_else(|| format!("missing {key} in {r:?}"))
}
fn number(r: &Record<'_>, key: &str) -> Result<f64, String> {
    let value: f64 = field(r, key)?
        .parse()
        .map_err(|_| format!("invalid {key}"))?;
    if !value.is_finite() {
        return Err(format!("non-finite {key}"));
    }
    Ok(value)
}
fn point(r: &Record<'_>, key: &str) -> Result<Point, String> {
    let values: Vec<f64> = field(r, key)?
        .split(',')
        .map(|v| v.parse::<f64>())
        .collect::<Result<_, _>>()
        .map_err(|e| e.to_string())?;
    if values.len() != 3 || values.iter().any(|v| !v.is_finite()) {
        return Err(format!("invalid point {key}"));
    }
    Ok([values[0], values[1], values[2]])
}
fn require(condition: bool, detail: &str) -> Result<(), String> {
    if condition {
        Ok(())
    } else {
        Err(detail.into())
    }
}
fn distance(a: Point, b: Point) -> f64 {
    (0..3).map(|i| (a[i] - b[i]).powi(2)).sum::<f64>().sqrt()
}
fn touches(a: Point, b: Point, min: Point, max: Point) -> bool {
    let (mut enter, mut exit) = (0.0_f64, 1.0_f64);
    for i in 0..3 {
        let delta = b[i] - a[i];
        if delta.abs() < 1e-12 {
            if a[i] < min[i] || a[i] > max[i] {
                return false;
            }
        } else {
            let x = (min[i] - a[i]) / delta;
            let y = (max[i] - a[i]) / delta;
            enter = enter.max(x.min(y));
            exit = exit.min(x.max(y));
            if enter > exit {
                return false;
            }
        }
    }
    true
}
fn route_distance(p: Point, route: &[Point]) -> f64 {
    route
        .windows(2)
        .map(|s| {
            let delta: Point = std::array::from_fn(|i| s[1][i] - s[0][i]);
            let norm = delta.iter().map(|v| v * v).sum::<f64>();
            let t = if norm > 0.0 {
                ((0..3).map(|i| (p[i] - s[0][i]) * delta[i]).sum::<f64>() / norm).clamp(0.0, 1.0)
            } else {
                0.0
            };
            distance(p, std::array::from_fn(|i| s[0][i] + t * delta[i]))
        })
        .fold(f64::INFINITY, f64::min)
}
// Rounded footprint of the actual authored sphere, rather than its visual AABB.
fn sphere_touches(a: Point, b: Point, min: Point, max: Point, radius: f64) -> bool {
    if a[2].min(b[2]) - radius > max[2] || a[2].max(b[2]) + radius < min[2] {
        return false;
    }
    let a2 = [a[0], a[1], 0.0];
    let b2 = [b[0], b[1], 0.0];
    if touches(a2, b2, [min[0], min[1], -1.0], [max[0], max[1], 1.0]) {
        return true;
    }
    let endpoint_distance = |p: Point| {
        ((p[0] - p[0].clamp(min[0], max[0])).powi(2) + (p[1] - p[1].clamp(min[1], max[1])).powi(2))
            .sqrt()
    };
    let mut clearance = endpoint_distance(a).min(endpoint_distance(b));
    for x in [min[0], max[0]] {
        for y in [min[1], max[1]] {
            clearance = clearance.min(route_distance([x, y, 0.0], &[a2, b2]));
        }
    }
    clearance <= radius
}
fn one<'a>(log: &'a str, marker: &str) -> Result<Record<'a>, String> {
    let found = records(log, marker);
    require(
        found.len() == 1,
        &format!("expected one {marker}, got {}", found.len()),
    )?;
    Ok(found.into_iter().next().unwrap())
}
fn verify(log: &str) -> Result<String, String> {
    let fixture = one(log, "NAV_ACCEPTANCE_FIXTURE")?;
    let direct = one(log, "NAV_ACCEPTANCE_DIRECT_BLOCKED")?;
    let query = one(log, "NAV_ACCEPTANCE_QUERY")?;
    let path = one(log, "NAV_ACCEPTANCE_PATH_COMPLETE")?;
    let result = one(log, "NAV_ACCEPTANCE_RESULT")?;
    let submitted = one(log, "NAV_ACCEPTANCE_SUBMITTED")?;
    require(
        log.contains("CONTENT_VALIDATION_PASSED map=map.test.static_obstacle_detour")
            && log.contains("MATCH_LIFECYCLE phase=4"),
        "validated Active admission missing",
    )?;
    require(
        field(&fixture, "map")? == "map.test.static_obstacle_detour"
            && field(&fixture, "level")?.ends_with("/Navigation/W_StaticObstacleDetour"),
        "wrong cooked map",
    )?;
    for key in ["hit", "collision", "nav_excluded"] {
        require(
            field(&direct, key)? == "1",
            "direct blocking/collision/NavMesh exclusion failed",
        )?;
    }
    require(
        field(&direct, "actor")? == field(&fixture, "wall")?,
        "trace hit unrelated obstacle",
    )?;
    require(
        field(&query, "partial")? == "0"
            && field(&query, "profile")? == "movement.prototype_ground"
            && field(&query, "nav")? == field(&fixture, "nav")?,
        "partial or wrong navigation data",
    )?;
    // ENavigationQueryResult::Success is 3 in the pinned UE enum.
    require(
        field(&query, "result")? == "3" && field(&fixture, "runtime_generation")? == "0",
        "query failed or fixture is not static navigation",
    )?;
    for r in [&query, &submitted, &result] {
        for key in ["unit", "generation"] {
            require(
                field(r, key)? == field(&fixture, key)?,
                "stale unit/generation",
            )?;
        }
        require(
            field(r, "sequence")? == field(&path, "sequence")?,
            "stale order",
        )?;
    }
    let start = point(&fixture, "start")?;
    let goal = point(&fixture, "goal")?;
    let extent = point(&fixture, "extent")?;
    let wall_min = point(&fixture, "min")?;
    let wall_max = point(&fixture, "max")?;
    let min = wall_min;
    let max = wall_max;
    let radius = number(&fixture, "radius")?;
    require(
        extent.iter().all(|v| *v > 0.0)
            && radius > 0.0
            && sphere_touches(start, goal, min, max, radius),
        "direct body segment does not intersect blocker",
    )?;
    let points = records(log, "NAV_ACCEPTANCE_POINT");
    let mut route = Vec::new();
    for (i, p) in points.iter().enumerate() {
        require(number(p, "index")? == i as f64, "unordered path points")?;
        route.push(point(p, "position")?);
    }
    require(
        route.len() >= 2 && number(&query, "points")? == route.len() as f64,
        "empty/incomplete route data",
    )?;
    let length: f64 = route.windows(2).map(|s| distance(s[0], s[1])).sum();
    require(
        length > distance(start, goal) + 0.1 && (length - number(&path, "length")?).abs() < 0.01,
        "not a material detour",
    )?;
    require(
        route
            .windows(2)
            .all(|s| !sphere_touches(s[0], s[1], min, max, radius)),
        "route collision body crosses blocker",
    )?;
    let resolved = point(&query, "resolved")?;
    require(
        distance(resolved, goal) <= 150.0
            && distance(route[0], start) < 0.01
            && distance(*route.last().unwrap(), resolved) < 0.01,
        "unrelated projected endpoints",
    )?;
    let samples = records(log, "NAV_ACCEPTANCE_MOVEMENT_SAMPLE");
    let substeps = records(log, "NAV_ACCEPTANCE_TRAVERSE");
    let speed = number(&fixture, "speed")?;
    let mut previous = start;
    let mut max_travel = 0.0_f64;
    let mut traversed = 0;
    let arrival = one(log, "NAV_ACCEPTANCE_ARRIVED")?;
    let arrival_step = number(&arrival, "step")?;
    let mut previous_waypoint = 1.0;
    for (i, s) in samples.iter().enumerate() {
        let step = number(s, "step")?;
        require(step == (i + 1) as f64, "missing authoritative step sample")?;
        for key in ["unit", "generation", "owner"] {
            require(
                field(s, key)? == field(&fixture, key)?,
                "movement authority changed",
            )?;
        }
        require(
            field(s, "sequence")? == field(&path, "sequence")?
                && field(s, "active")? == "1"
                && field(s, "clear")? == "1"
                && field(s, "valid")? == "1",
            "movement invariant failed",
        )?;
        let mut travel = 0.0;
        for (index, p) in substeps
            .iter()
            .filter(|p| p.get("step") == s.get("step"))
            .enumerate()
        {
            require(
                number(p, "index")? == index as f64,
                "missing swept subsegment",
            )?;
            let next = point(p, "position")?;
            require(
                !sphere_touches(previous, next, min, max, radius)
                    && route_distance(next, &route) <= 2.1
                    && (next[2] - start[2]).abs() < 0.001,
                "traversal penetrates obstacle or leaves computed route",
            )?;
            travel += distance(previous, next);
            previous = next;
            traversed += 1;
        }
        require(
            distance(previous, point(s, "position")?) < 0.0001,
            "unobserved movement/teleport",
        )?;
        require(
            (travel - number(s, "travel")?).abs() < 0.0001
                && travel <= speed * 0.05 + 0.01
                && (number(s, "budget")? - speed * 0.05).abs() < 0.0001,
            "fixed-step authored speed exceeded",
        )?;
        max_travel = max_travel.max(travel);
        if step < arrival_step {
            let waypoint = number(s, "waypoint")?;
            require(
                waypoint >= previous_waypoint
                    && waypoint < route.len() as f64
                    && field(s, "order")? == "1",
                "wrong path cursor/order",
            )?;
            previous_waypoint = waypoint;
        } else {
            require(
                field(s, "order")? == "0" && field(s, "waypoint")? == "0",
                "completed path remains executable",
            )?;
        }
    }
    require(
        traversed == substeps.len() && samples.len() > 20,
        "insufficient traversal samples",
    )?;
    let ended = one(log, "NAV_ACCEPTANCE_TERMINATION")?;
    require(
        field(&ended, "reason")? == "5" && field(&ended, "step")? == field(&arrival, "step")?,
        "wrong termination identity",
    )?;
    let emitted: Vec<_> = records(log, "RTS_GAMEPLAY_EVENT")
        .into_iter()
        .filter(|r| {
            r.get("kind") == Some(&"OrderChanged")
                && r.get("entity") == fixture.get("unit")
                && r.get("reason") == Some(&"5")
        })
        .collect();
    require(
        emitted.len() == 1 && field(&emitted[0], "step")? == field(&arrival, "step")?,
        "arrival publication is not singular",
    )?;
    for key in ["commands", "terminations", "arrivals", "retired"] {
        require(
            field(&result, key)? == "1",
            "exactly-once/retirement failed",
        )?;
    }
    require(
        field(&result, "result")? == "PASS"
            && field(&result, "failures")? == "0"
            && number(&result, "final_step")? - arrival_step >= 20.0
            && number(&result, "samples")? == samples.len() as f64
            && distance(previous, resolved) <= 18.0,
        "terminal result/arrival hold failed",
    )?;
    Ok(format!(
        "requirement\tverdict\tdetail\nNAV-A25\tPASS\tcooked production detour; {} points; length={length:.6}; direct={:.6}; {} steps; {} swept subsegments; max_travel={max_travel:.6}; budget={:.6}; arrival_step={arrival_step}; post_arrival_steps={}; arrivals=1; terminations=1; error={:.6}\n",
        route.len(),
        distance(start, goal),
        samples.len(),
        substeps.len(),
        speed * 0.05,
        samples.len() as f64 - arrival_step,
        distance(previous, resolved)
    ))
}
fn read(path: &Path) -> Result<String, String> {
    fs::read_to_string(path).map_err(|e| format!("{}: {e}", path.display()))
}
fn hash(path: &Path) -> Result<String, String> {
    let out = Command::new("sha256sum")
        .arg(path)
        .output()
        .map_err(|e| e.to_string())?;
    require(out.status.success(), "sha256sum failed")?;
    Ok(String::from_utf8_lossy(&out.stdout)
        .split_whitespace()
        .next()
        .ok_or("no hash")?
        .into())
}
fn launch(binary: &Path, evidence: &Path, name: &str, scenario: bool) -> Result<String, String> {
    launch_with_schedule(binary, evidence, name, scenario, None)
}
fn launch_with_schedule(
    binary: &Path,
    evidence: &Path,
    name: &str,
    scenario: bool,
    schedule: Option<&str>,
) -> Result<String, String> {
    launch_scenario(binary, evidence, name, scenario, schedule, None)
}
fn launch_scenario(
    binary: &Path,
    evidence: &Path,
    name: &str,
    scenario: bool,
    schedule: Option<&str>,
    interruption: Option<&str>,
) -> Result<String, String> {
    let log = evidence.join(format!("{name}.log"));
    let mut command = Command::new(binary);
    if let Some(interruption) = interruption {
        command.arg(format!("-PolitikaNavigationInterruption={interruption}"));
    }
    if let Some(schedule) = schedule {
        command.arg(format!("-PolitikaNavigationSchedule={schedule}"));
    }
    if scenario {
        command.args([
            "/Game/Politika/TestFixtures/Navigation/W_StaticObstacleDetour",
            "-PolitikaTestFixture=StaticObstacleDetour",
        ]);
    } else {
        command.arg("-ExecCmds=Quit");
    }
    let mut child = command
        .args(["-nullrhi", "-unattended", "-nosound", "-nosplash"])
        .arg(format!(
            "-UserDir={}/",
            evidence.join(format!("{name}-user")).display()
        ))
        .arg(format!("-ABSLOG={}", log.display()))
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| e.to_string())?;
    let deadline = Instant::now() + Duration::from_secs(90);
    loop {
        match child.try_wait() {
            Ok(Some(status)) => {
                let text = read(&log)?;
                require(status.success(), &format!("{name} crashed/exited {status}"))?;
                if !scenario {
                    require(
                        text.contains("LoadMap: /Game/Maps/MainMenu")
                            && !text.contains("NAV_ACCEPTANCE_"),
                        "ordinary startup changed",
                    )?;
                }
                return Ok(text);
            }
            Ok(None) => {}
            Err(e) => {
                let _ = child.kill();
                let _ = child.wait();
                return Err(e.to_string());
            }
        }
        if Instant::now() >= deadline {
            let _ = child.kill();
            let _ = child.wait();
            return Err(format!(
                "{name} timeout; no qualification; log={}",
                log.display()
            ));
        }
        thread::sleep(Duration::from_millis(100));
    }
}
fn verify_disconnected(log: &str) -> Result<(String, String), String> {
    let fixture = one(log, "NAV_FAILURE_FIXTURE")?;
    let query = one(log, "NAV_FAILURE_QUERY")?;
    let submitted = one(log, "NAV_FAILURE_SUBMITTED")?;
    let result = one(log, "NAV_FAILURE_RESULT")?;
    require(
        log.contains("CONTENT_VALIDATION_PASSED map=map.test.disconnected_navigation")
            && log.contains("MATCH_LIFECYCLE phase=4"),
        "disconnected fixture did not pass content admission and reach Active",
    )?;
    require(
        field(&fixture, "scenario")? == "Disconnected"
            && field(&fixture, "map")? == "map.test.disconnected_navigation"
            && field(&fixture, "level")?.ends_with("/Navigation/W_DisconnectedNavigation")
            && field(&fixture, "profile")? == "movement.prototype_ground"
            && field(&fixture, "runtime_generation")? == "0"
            && field(&query, "nav")? == field(&fixture, "nav")?
            && field(&query, "profile")? == field(&fixture, "profile")?,
        "wrong cooked fixture, movement profile, or nav data",
    )?;
    for key in ["generation", "unit"] {
        require(
            field(&query, key)? == field(&fixture, key)?,
            "query unit/generation mismatch",
        )?;
        require(
            field(&submitted, key)? == field(&fixture, key)?,
            "submission unit/generation mismatch",
        )?;
        require(
            field(&result, key)? == field(&fixture, key)?,
            "result unit/generation mismatch",
        )?;
    }
    for record in [&query, &submitted, &result] {
        require(
            field(record, "sequence")? == field(&result, "sequence")?,
            "stale command sequence in failure trace",
        )?;
    }
    require(
        field(&query, "allow_partial")? == "0"
            && field(&query, "require_navigable_end")? == "1"
            && field(&query, "failure")? == "NoCompletePath"
            && field(&query, "result")? != "3"
            && field(&query, "partial")? == field(&result, "partial")?,
        "production query did not produce a no-complete-path result or its actual partial flag was lost",
    )?;
    require(
        number(&fixture, "start_offset")? <= 80.0
            && number(&fixture, "goal_offset")? <= 80.0
            && number(&query, "result")? != 3.0
            && number(&query, "points")? >= 0.0,
        "endpoint projections failed or the result code indicates a complete Success route",
    )?;
    let start = point(&fixture, "start")?;
    let goal = point(&fixture, "goal")?;
    let projected_start = point(&fixture, "projected_start")?;
    let projected_goal = point(&fixture, "projected_goal")?;
    require(
        distance(start, projected_start) <= 80.0 && distance(goal, projected_goal) <= 80.0,
        "projected endpoint offset exceeded tolerance",
    )?;
    let bounds_min: Vec<f64> = field(&fixture, "bounds_min")?
        .split(',')
        .map(str::parse::<f64>)
        .collect::<Result<_, _>>()
        .map_err(|e| e.to_string())?;
    let bounds_max: Vec<f64> = field(&fixture, "bounds_max")?
        .split(',')
        .map(str::parse::<f64>)
        .collect::<Result<_, _>>()
        .map_err(|e| e.to_string())?;
    let barrier_min = point(&fixture, "barrier_min")?;
    let barrier_max = point(&fixture, "barrier_max")?;
    require(
        bounds_min.len() == 2
            && bounds_max.len() == 2
            && start[0] >= bounds_min[0]
            && start[0] <= bounds_max[0]
            && start[1] >= bounds_min[1]
            && start[1] <= bounds_max[1]
            && goal[0] >= bounds_min[0]
            && goal[0] <= bounds_max[0]
            && goal[1] >= bounds_min[1]
            && goal[1] <= bounds_max[1]
            && start[0] < barrier_min[0]
            && goal[0] > barrier_max[0]
            && barrier_min[1] < bounds_min[1]
            && barrier_max[1] > bounds_max[1],
        "positions are out of bounds or barrier does not divide the playable region",
    )?;
    let events = records(log, "NAV_FAILURE_EVENT");
    let commands: Vec<_> = events
        .iter()
        .filter(|e| e.get("kind") == Some(&"CommandOutcome"))
        .collect();
    let changes: Vec<_> = events
        .iter()
        .filter(|e| e.get("kind") == Some(&"OrderChanged"))
        .collect();
    require(
        commands.len() == 1 && changes.len() == 1,
        "expected exactly one command and one order publication",
    )?;
    let command = commands[0];
    let changed = changes[0];
    require(
        field(command, "sequence")? == field(&result, "sequence")?
            && field(command, "correlation")? == field(&result, "sequence")?
            && field(command, "accepted")? == "1"
            && field(command, "order")? == "0"
            && field(command, "rejection")? == "0"
            && field(command, "valid")? == "1"
            && field(changed, "reason")? == "7"
            && field(changed, "failure")? == "1"
            && field(changed, "arrival")? == "0",
        "typed Move admission or NavigationFailed event contract changed",
    )?;
    let steps = records(log, "NAV_FAILURE_STEP");
    require(
        steps.len() == 21,
        &format!(
            "expected 21 inert authoritative step samples, got {}",
            steps.len()
        ),
    )?;
    for (index, sample) in steps.iter().enumerate() {
        require(
            number(sample, "step")? == number(changed, "step")? + index as f64
                && distance(point(sample, "position")?, start) <= 0.001
                && field(sample, "order")? == "4"
                && field(sample, "points")? == "0"
                && field(sample, "cursor")? == "0"
                && field(sample, "route_sequence")? == "0"
                && field(sample, "route_generation")? == "0"
                && field(sample, "alive")? == "1"
                && field(sample, "active")? == "1"
                && field(sample, "pending")? == "0"
                && field(sample, "arrivals")? == "0"
                && field(sample, "valid")? == "1",
            &format!(
                "failed route was executable or moved at post-failure step {}",
                index
            ),
        )?;
    }
    require(
        field(&result, "result")? == "PASS"
            && field(&result, "queries")? == "1"
            && field(&result, "partial")? == field(&query, "partial")?
            && field(&result, "accepted_commands")? == "1"
            && field(&result, "failure_events")? == "1"
            && field(&result, "arrivals")? == "0"
            && field(&result, "inert_steps")? == "21"
            && records(log, "RTS_NAV_QUERY").len() == 1,
        "authoritative failure result was incomplete or repeated",
    )?;
    let probe = one(log, "NAV_FAILURE_PARTIAL_PROBE")?;
    let partial_verdict = if field(&probe, "partial")? == "1"
        && field(&probe, "path_valid")? == "1"
        && number(&probe, "endpoint_error")? > 50.0
        && field(&probe, "route_admission")? == "REJECTED"
        && field(&probe, "installed")? == "0"
        && !field(&probe, "rejection")?.is_empty()
    {
        format!(
            "PASS genuine partial result={} endpoint_error={} rejection={} installed=0",
            field(&probe, "result")?,
            field(&probe, "endpoint_error")?,
            field(&probe, "rejection")?
        )
    } else {
        format!(
            "INCONCLUSIVE actual partial not reproduced: result={} partial={} points={} endpoint_error={} admission={} installed={}",
            field(&probe, "result")?,
            field(&probe, "partial")?,
            field(&probe, "points")?,
            field(&probe, "endpoint_error")?,
            field(&probe, "route_admission")?,
            field(&probe, "installed")?
        )
    };
    Ok((
        format!(
            "PASS result={} partial={} start={} goal={} query_points={} post_failure_steps={} events=accepted_move:1,navigation_failed:1,arrived:0",
            field(&query, "result")?,
            field(&query, "partial")?,
            field(&fixture, "start")?,
            field(&fixture, "goal")?,
            field(&query, "points")?,
            steps.len()
        ),
        partial_verdict,
    ))
}

fn launch_disconnected(binary: &Path, evidence: &Path, name: &str) -> Result<String, String> {
    let log = evidence.join(format!("{name}.log"));
    let user_dir = evidence.join(format!("{name}-user"));
    fs::create_dir_all(&user_dir).map_err(|e| e.to_string())?;
    let mut child = Command::new(binary)
        .args([
            "/Game/Politika/TestFixtures/Navigation/W_DisconnectedNavigation",
            "-PolitikaTestFixture=DisconnectedNavigation",
            "-PolitikaNavigationFailure=Disconnected",
            "-nullrhi",
            "-unattended",
            "-nosound",
            "-nosplash",
        ])
        .arg(format!("-UserDir={}/", user_dir.display()))
        .arg(format!("-ABSLOG={}", log.display()))
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| format!("launch {name}: {e}"))?;
    let deadline = Instant::now() + Duration::from_secs(90);
    loop {
        if let Some(status) = child.try_wait().map_err(|e| e.to_string())? {
            let text = read(&log)?;
            require(
                status.success(),
                &format!(
                    "{name} failed process outcome {status}; log={}",
                    log.display()
                ),
            )?;
            fs::copy(&log, evidence.join(format!("{name}-runtime.log")))
                .map_err(|e| e.to_string())?;
            return Ok(text);
        }
        if Instant::now() >= deadline {
            let _ = child.kill();
            let _ = child.wait();
            return Err(format!("{name} timed out; log={}", log.display()));
        }
        thread::sleep(Duration::from_millis(100));
    }
}

fn launch_unprojectable(binary: &Path, evidence: &Path, name: &str) -> Result<String, String> {
    let log = evidence.join(format!("{name}.log"));
    let user_dir = evidence.join(format!("{name}-user"));
    fs::create_dir_all(&user_dir).map_err(|e| e.to_string())?;
    let mut child = Command::new(binary)
        .args([
            "/Game/Politika/TestFixtures/Navigation/W_UnprojectableDestination",
            "-PolitikaTestFixture=UnprojectableDestination",
            "-PolitikaNavigationFailure=UnprojectableDestination",
            "-nullrhi",
            "-unattended",
            "-nosound",
            "-nosplash",
        ])
        .arg(format!("-UserDir={}/", user_dir.display()))
        .arg(format!("-ABSLOG={}", log.display()))
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| format!("launch {name}: {e}"))?;
    let deadline = Instant::now() + Duration::from_secs(90);
    loop {
        if let Some(status) = child.try_wait().map_err(|e| e.to_string())? {
            let text = read(&log)?;
            require(
                status.success(),
                &format!(
                    "{name} failed process outcome {status}; log={}",
                    log.display()
                ),
            )?;
            fs::copy(&log, evidence.join(format!("{name}-runtime.log")))
                .map_err(|e| e.to_string())?;
            return Ok(text);
        }
        if Instant::now() >= deadline {
            let _ = child.kill();
            let _ = child.wait();
            return Err(format!("{name} timed out; log={}", log.display()));
        }
        thread::sleep(Duration::from_millis(100));
    }
}

fn verify_unprojectable(log: &str) -> Result<String, String> {
    let fixture = one(log, "NAV_FAILURE_FIXTURE")?;
    let projection = one(log, "NAV_FAILURE_PROJECTION")?;
    let ground = one(log, "NAV_FAILURE_GROUND")?;
    let submitted = one(log, "NAV_FAILURE_SUBMITTED")?;
    let result = one(log, "NAV_FAILURE_RESULT")?;
    require(
        log.contains("CONTENT_VALIDATION_PASSED map=map.test.unprojectable_destination")
            && log.contains("MATCH_LIFECYCLE phase=4"),
        "unprojectable destination fixture did not validate and reach Active",
    )?;
    require(
        field(&fixture, "scenario")? == "UnprojectableDestination"
            && field(&fixture, "map")? == "map.test.unprojectable_destination"
            && field(&fixture, "level")?.ends_with("/Navigation/W_UnprojectableDestination")
            && field(&fixture, "profile")? == "movement.prototype_ground"
            && field(&fixture, "runtime_generation")? == "0"
            && field(&fixture, "origin_projected")? == "1"
            && field(&fixture, "goal_projected")? == "0"
            && field(&projection, "origin_projected")? == "1"
            && field(&projection, "goal_projected")? == "0"
            && field(&projection, "query_executed")? == "0"
            && field(&projection, "failure")? == "DestinationNotNavigable",
        "wrong fixture, origin/goal projection, query distinction, or failure reason",
    )?;
    require(
        field(&ground, "ground_hit")? == "1"
            && field(&ground, "bounds_valid")? == "1"
            && field(&ground, "nav_excluded")? == "1"
            && distance(point(&ground, "goal")?, point(&fixture, "goal")?) <= 0.001,
        "goal is not the physically hit authored ground at the declared in-bounds location",
    )?;
    require(
        !log.contains("NAV_FAILURE_QUERY ") && !log.contains("RTS_NAV_QUERY "),
        "a navigation path query ran despite failed destination projection",
    )?;
    for key in ["generation", "unit", "sequence"] {
        require(
            field(&submitted, key)? == field(&result, key)?,
            "submission/result identity mismatch",
        )?;
    }
    require(
        field(&result, "result")? == "PASS"
            && field(&result, "failure_classification")? == "DestinationNotNavigable"
            && field(&result, "projection_checks")? == "2"
            && field(&result, "origin_projected")? == "1"
            && field(&result, "goal_projected")? == "0"
            && field(&result, "queries")? == "0"
            && field(&result, "query_result")? == "NA"
            && field(&result, "accepted_commands")? == "1"
            && field(&result, "failure_events")? == "1"
            && field(&result, "arrivals")? == "0"
            && field(&result, "inert_steps")? == "21"
            && field(&result, "retired")? == "1",
        "terminal production assertions failed",
    )?;
    let events = records(log, "NAV_FAILURE_EVENT");
    let steps = records(log, "NAV_FAILURE_STEP");
    require(
        events.len() == 2 && steps.len() == 21,
        "expected one accepted command, one failure publication, and 21 stationary step records",
    )?;
    require(
        events
            .iter()
            .any(|e| e.get("kind") == Some(&"CommandOutcome") && e.get("accepted") == Some(&"1"))
            && events
                .iter()
                .any(|e| e.get("kind") == Some(&"OrderChanged") && e.get("failure") == Some(&"1"))
            && steps.iter().all(|step| step.get("valid") == Some(&"1")),
        "command/event timeline or inert-step evidence is invalid",
    )?;
    Ok(format!(
        "PASS goal={} origin_projected=1 goal_projected=0 query_executed=0 failure=DestinationNotNavigable accepted_move=1 failure_event=1 arrived=0 inert_steps=20",
        field(&fixture, "goal")?
    ))
}

pub fn run_destination_failure() -> Result<(), String> {
    let binary = PathBuf::from(env::args_os().nth(1).ok_or("missing fixture binary")?)
        .canonicalize()
        .map_err(|e| e.to_string())?;
    let normal = PathBuf::from(env::args_os().nth(3).ok_or(
        "usage: rts_interaction FIXTURE_BINARY --navigation-destination-failure NORMAL_BINARY",
    )?)
    .canonicalize()
    .map_err(|e| e.to_string())?;
    let package = binary.ancestors().nth(4).ok_or("fixture package layout")?;
    let normal_package = normal.ancestors().nth(4).ok_or("normal package layout")?;
    let epoch = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_nanos();
    let evidence = package.join(format!("navigation-destination-failure-{epoch}"));
    fs::create_dir_all(&evidence).map_err(|e| e.to_string())?;
    let outcome = (|| -> Result<String, String> {
        let source = read(&package.join("QualificationSource.sha256"))?;
        require(
            !source.trim().is_empty()
                && source == read(&normal_package.join("QualificationSource.sha256"))?
                && read(&package.join("BuildIdentity.json"))?
                    == read(&normal_package.join("BuildIdentity.json"))?
                && hash(&binary)? == hash(&normal)?,
            "normal/fixture package source identity mismatch",
        )?;
        let fixture_manifest = read(&package.join("Manifest_UFSFiles_Linux.txt"))?;
        let normal_manifest = read(&normal_package.join("Manifest_UFSFiles_Linux.txt"))?;
        for asset in [
            "Navigation/W_UnprojectableDestination.umap",
            "Navigation/DA_UnprojectableDestination.uasset",
        ] {
            require(
                fixture_manifest.contains(asset),
                &format!("fixture package missing {asset}"),
            )?;
        }
        require(
            !normal_manifest.contains("TestFixtures/"),
            "normal package includes test fixtures",
        )?;
        let mut report = String::from("run\tverdict\tdetail\n");
        for name in ["DP-01-A", "DP-01-B"] {
            let log = launch_unprojectable(&binary, &evidence, name)?;
            let detail = verify_unprojectable(&log)?;
            report.push_str(&format!("{name}\tPASS\t{detail}\n"));
        }
        fs::write(evidence.join("acceptance.tsv"), &report).map_err(|e| e.to_string())?;
        fs::write(evidence.join("artifact-identity.txt"), format!(
            "fixture_binary={}\nfixture_sha256={}\nnormal_binary={}\nnormal_sha256={}\nsource={}\nfixture_identity={}\n",
            binary.display(), hash(&binary)?, normal.display(), hash(&normal)?, source.trim(),
            read(&package.join("BuildIdentity.json"))?.trim().replace('\n', " "))).map_err(|e| e.to_string())?;
        Ok(report)
    })();
    match outcome {
        Ok(report) => {
            println!(
                "PASS DP-01: packaged unprojectable destination; evidence {}\n{}",
                evidence.display(),
                report
            );
            Ok(())
        }
        Err(error) => {
            let report = format!(
                "run\tverdict\tdetail\nDP-01\tFAIL\t{}\n",
                error.replace(['\n', '\t'], " ")
            );
            let _ = fs::write(evidence.join("acceptance.tsv"), report);
            Err(format!("{error}; evidence={}", evidence.display()))
        }
    }
}

pub fn run_unreachable() -> Result<(), String> {
    let binary = PathBuf::from(env::args_os().nth(1).ok_or("missing fixture binary")?)
        .canonicalize()
        .map_err(|e| e.to_string())?;
    let normal =
        PathBuf::from(env::args_os().nth(3).ok_or(
            "usage: rts_interaction FIXTURE_BINARY --navigation-unreachable NORMAL_BINARY",
        )?)
        .canonicalize()
        .map_err(|e| e.to_string())?;
    let package = binary.ancestors().nth(4).ok_or("fixture package layout")?;
    let normal_package = normal.ancestors().nth(4).ok_or("normal package layout")?;
    let epoch = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_nanos();
    let evidence = package.join(format!("navigation-unreachable-{epoch}"));
    fs::create_dir_all(&evidence).map_err(|e| e.to_string())?;
    let outcome = (|| -> Result<String, String> {
        let source = read(&package.join("QualificationSource.sha256"))?;
        require(
            !source.trim().is_empty()
                && source == read(&normal_package.join("QualificationSource.sha256"))?
                && read(&package.join("BuildIdentity.json"))?
                    == read(&normal_package.join("BuildIdentity.json"))?
                && hash(&binary)? == hash(&normal)?,
            "normal/fixture package source identity mismatch",
        )?;
        let fixture_manifest = read(&package.join("Manifest_UFSFiles_Linux.txt"))?;
        let normal_manifest = read(&normal_package.join("Manifest_UFSFiles_Linux.txt"))?;
        for asset in [
            "Navigation/W_DisconnectedNavigation.umap",
            "Navigation/DA_DisconnectedNavigation.uasset",
        ] {
            require(
                fixture_manifest.contains(asset),
                &format!("fixture package missing {asset}"),
            )?;
        }
        require(
            !normal_manifest.contains("TestFixtures/"),
            "normal package includes negative fixtures",
        )?;
        let mut verdicts = String::from(
            "run\tverdict\tquery_result\tpartial\tpost_failure_steps\tevents\tdetail\n",
        );
        for repetition in 1..=2 {
            let name = format!("NQ-01-{}", if repetition == 1 { "A" } else { "B" });
            let log = launch_disconnected(&binary, &evidence, &name)?;
            let (report, partial_report) = verify_disconnected(&log)?;
            let query = one(&log, "NAV_FAILURE_QUERY")?;
            let result = one(&log, "NAV_FAILURE_RESULT")?;
            verdicts.push_str(&format!("{name}\tPASS\t{}\t{}\t{}\taccepted_move:1,navigation_failed:1,arrived:0\t{report}\n",
                field(&query, "result")?, field(&query, "partial")?, field(&result, "inert_steps")?));
            verdicts.push_str(&format!(
                "NQ-02-{}\t{}\t{}\t{}\tNA\tpartial_route_admission\t{}\n",
                if repetition == 1 { "A" } else { "B" },
                if partial_report.starts_with("PASS") {
                    "PASS"
                } else {
                    "INCONCLUSIVE"
                },
                field(&one(&log, "NAV_FAILURE_PARTIAL_PROBE")?, "result")?,
                field(&one(&log, "NAV_FAILURE_PARTIAL_PROBE")?, "partial")?,
                partial_report
            ));
        }
        fs::write(evidence.join("acceptance.tsv"), &verdicts).map_err(|e| e.to_string())?;
        fs::write(evidence.join("artifact-identity.txt"), format!(
            "fixture_binary={}\nfixture_sha256={}\nnormal_binary={}\nnormal_sha256={}\nsource={}\nfixture_identity={}\n",
            binary.display(), hash(&binary)?, normal.display(), hash(&normal)?, source.trim(),
            read(&package.join("BuildIdentity.json"))?.trim().replace('\n', " "))).map_err(|e| e.to_string())?;
        Ok(verdicts)
    })();
    match outcome {
        Ok(report) => {
            println!(
                "PASS NQ-01: production disconnected-route failure; evidence {}\n{}",
                evidence.display(),
                report
            );
            Ok(())
        }
        Err(error) => {
            let report = format!(
                "run\tverdict\tdetail\nNQ-01\tFAIL\t{}\n",
                error.replace(['\n', '\t'], " ")
            );
            let _ = fs::write(evidence.join("acceptance.tsv"), report);
            Err(format!("{error}; evidence={}", evidence.display()))
        }
    }
}

pub fn run() -> Result<(), String> {
    let binary = PathBuf::from(env::args_os().nth(1).ok_or("missing fixture binary")?)
        .canonicalize()
        .map_err(|e| e.to_string())?;
    let normal = PathBuf::from(
        env::args_os()
            .nth(3)
            .ok_or("usage: rts_interaction FIXTURE_BINARY --static-obstacle NORMAL_BINARY")?,
    )
    .canonicalize()
    .map_err(|e| e.to_string())?;
    let package = binary.ancestors().nth(4).ok_or("package layout")?;
    let normal_package = normal.ancestors().nth(4).ok_or("normal package layout")?;
    let epoch = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_secs();
    let evidence = package.join(format!("navigation-obstacle-acceptance-{epoch}"));
    fs::create_dir_all(&evidence).map_err(|e| e.to_string())?;
    let outcome: Result<String, String> = (|| {
        require(
            read(&package.join("BuildIdentity.json"))?
                == read(&normal_package.join("BuildIdentity.json"))?,
            "package revision mismatch",
        )?;
        let source = read(&package.join("QualificationSource.sha256"))?;
        require(
            !source.trim().is_empty()
                && source == read(&normal_package.join("QualificationSource.sha256"))?,
            "exact dirty source identity mismatch",
        )?;
        require(
            hash(&binary)? == hash(&normal)?,
            "normal/fixture executable mismatch",
        )?;
        let manifest = read(&package.join("Manifest_UFSFiles_Linux.txt"))?;
        for asset in [
            "Navigation/W_StaticObstacleDetour.umap",
            "Navigation/DA_StaticObstacleDetour.uasset",
        ] {
            require(
                manifest.contains(asset),
                &format!("BLOCKED missing cooked {asset}"),
            )?;
        }
        let normal_manifest = read(&normal_package.join("Manifest_UFSFiles_Linux.txt"))?;
        require(
            !normal_manifest.contains("TestFixtures/"),
            "test content leaked into normal cook",
        )?;
        launch(&normal, &evidence, "normal-startup", false)?;
        launch(&binary, &evidence, "fixture-ordinary-startup", false)?;
        let log = launch(&binary, &evidence, "static-obstacle", true)?;
        let report = verify(&log)?;
        fs::write(
            evidence.join("artifact-identity.txt"),
            format!(
                "binary={}\nsha256={}\nsource={}\nnormal={}\n",
                binary.display(),
                hash(&binary)?,
                source.trim(),
                normal.display()
            ),
        )
        .map_err(|e| e.to_string())?;
        Ok(report)
    })();
    let report = match &outcome {
        Ok(report) => report.clone(),
        Err(e) => format!("requirement\tverdict\tdetail\nNAV-A25\tFAIL\t{e}\n"),
    };
    fs::write(evidence.join("results.tsv"), report).map_err(|e| e.to_string())?;
    println!("Navigation packaged evidence: {}", evidence.display());
    outcome.map(|_| ())
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn conservative_swept_body_detects_crossing_and_corner_contact() {
        let min = [-142.5, -892.5, -267.5];
        let max = [142.5, -307.5, 1867.5];
        assert!(touches([-700., -600., 42.], [800., -600., 42.], min, max));
        assert!(touches([-700., -892.5, 42.], [800., -892.5, 42.], min, max));
        assert!(!touches([-700., -950., 42.], [800., -950., 42.], min, max));
    }
    #[test]
    fn missing_or_fabricated_terminal_pass_is_rejected() {
        assert!(verify("").is_err());
        assert!(verify("NAV_ACCEPTANCE_RESULT result=PASS").is_err());
    }
    #[test]
    fn round_body_clears_corner_where_square_bound_would_falsely_reject() {
        let min = [-100.0, -850.0, -200.0];
        let max = [100.0, -350.0, 1800.0];
        assert!(!sphere_touches(
            [-162.0, -850.0, 42.0],
            [-100.0, -912.0, 42.0],
            min,
            max,
            42.5
        ));
        assert!(sphere_touches(
            [-120.0, -860.0, 42.0],
            [0.0, -870.0, 42.0],
            min,
            max,
            42.5
        ));
    }
}

// Frozen NAV-A20 contract. Rust observes UE records; it never advances gameplay.
const POSITION_TOLERANCE: f64 = 0.1;
const WAYPOINT_TOLERANCE: f64 = 0.1;
const LENGTH_TOLERANCE: f64 = 0.25;
const TRAVEL_TOLERANCE: f64 = 0.1;

fn schedule_values(schedule: usize) -> &'static [f64] {
    match schedule {
        1 => &[1.0 / 120.0],
        2 => &[1.0 / 60.0],
        3 => &[1.0 / 30.0],
        4 => &[0.0125, 0.0375],
        5 => &[0.01, 0.20, 0.04],
        _ => &[],
    }
}

fn verify_updates(log: &str, schedule: usize) -> Result<(), String> {
    let begin = one(log, "NAV_REPEATABILITY_BEGIN")?;
    require(
        field(&begin, "schedule")? == format!("RS-{schedule:02}"),
        "wrong schedule",
    )?;
    let submitted = one(log, "NAV_ACCEPTANCE_SUBMITTED")?;
    require(
        field(&submitted, "boundary_step")? == "0",
        "command not submitted before step 1",
    )?;
    let updates = records(log, "NAV_REPEATABILITY_UPDATE");
    require(!updates.is_empty(), "missing outer updates")?;
    let mut last = 0.0;
    let mut accumulator = 0.0;
    let mut catch_up = false;
    for (i, update) in updates.iter().enumerate() {
        let delta = number(update, "delta")?;
        let steps = number(update, "steps")?;
        let first = number(update, "first")?;
        let end = number(update, "last")?;
        let remaining = number(update, "accumulator")?;
        require(
            number(update, "ordinal")? == (i + 1) as f64,
            "outer update gap",
        )?;
        require(
            (delta - schedule_values(schedule)[i % schedule_values(schedule).len()]).abs() < 1e-7,
            "schedule input changed",
        )?;
        require(
            (number(update, "clamped")? - delta.min(0.25)).abs() < 1e-7,
            "clamp changed",
        )?;
        require(
            first == last && end - first == steps && (0.0..=5.0).contains(&steps),
            "update/step coverage mismatch",
        )?;
        require(
            (accumulator + delta.min(0.25) - steps * 0.05 - remaining).abs() < 1e-6,
            "accumulator continuity changed",
        )?;
        require(remaining >= -1e-7 && remaining < 0.25, "invalid remainder")?;
        require(
            field(update, "cap_reached")? == if steps == 5.0 { "1" } else { "0" },
            "cap telemetry inconsistent",
        )?;
        last = end;
        accumulator = remaining;
        catch_up |= steps > 1.0;
    }
    require(
        last == number(&one(log, "NAV_ACCEPTANCE_RESULT")?, "final_step")?,
        "untraced steps after final sample",
    )?;
    if schedule == 5 {
        require(catch_up, "RS-05 did not exercise catch-up")?;
    }
    Ok(())
}

#[derive(Default, Debug)]
struct Deviation {
    position: f64,
    waypoint: f64,
    length: f64,
    travel: f64,
}

fn compare(reference: &str, actual: &str) -> Result<Deviation, String> {
    let mut max = Deviation::default();
    // All keys in these records are compared. Only generation is incidental;
    // verify() already enforces its consistency throughout each process.
    for marker in [
        "NAV_ACCEPTANCE_FIXTURE",
        "NAV_ACCEPTANCE_QUERY",
        "NAV_ACCEPTANCE_PATH_COMPLETE",
        "NAV_ACCEPTANCE_POINT",
        "NAV_ACCEPTANCE_MOVEMENT_SAMPLE",
        "NAV_ACCEPTANCE_TRAVERSE",
        "NAV_ACCEPTANCE_SUBMITTED",
        "NAV_ACCEPTANCE_TERMINATION",
        "NAV_ACCEPTANCE_ARRIVED",
        "NAV_ACCEPTANCE_RESULT",
        "RTS_GAMEPLAY_EVENT",
        "MATCH_AUTHORITATIVE_UNIT",
        "MATCH_PARTICIPANTS_ADMITTED",
    ] {
        let expected = records(reference, marker);
        let observed = records(actual, marker);
        require(!expected.is_empty(), &format!("missing reference {marker}"))?;
        require(
            expected.len() == observed.len(),
            &format!(
                "{marker} count differs: {} vs {}",
                expected.len(),
                observed.len()
            ),
        )?;
        for (index, (a, b)) in expected.iter().zip(&observed).enumerate() {
            require(
                a.keys().eq(b.keys()),
                &format!("{marker} record {index} fields differ"),
            )?;
            for (key, value) in a {
                if *key == "generation" {
                    continue;
                }
                let detail = || {
                    format!(
                        "{marker} record {index} step={:?} field={key} expected={value} actual={:?}",
                        a.get("step"),
                        b.get(key)
                    )
                };
                if ["position", "projected_start", "projected_goal", "resolved"].contains(key) {
                    let error = distance(point(a, key)?, point(b, key)?);
                    let tolerance = if marker == "NAV_ACCEPTANCE_POINT" {
                        max.waypoint = max.waypoint.max(error);
                        WAYPOINT_TOLERANCE
                    } else {
                        max.position = max.position.max(error);
                        POSITION_TOLERANCE
                    };
                    require(error <= tolerance, &detail())?;
                } else if *key == "length" {
                    let error = (number(a, key)? - number(b, key)?).abs();
                    max.length = max.length.max(error);
                    require(error <= LENGTH_TOLERANCE, &detail())?;
                } else if ["travel", "max_travel"].contains(key) {
                    let error = (number(a, key)? - number(b, key)?).abs();
                    max.travel = max.travel.max(error);
                    require(error <= TRAVEL_TOLERANCE, &detail())?;
                } else {
                    require(b.get(key) == Some(value), &detail())?;
                }
            }
        }
    }
    Ok(max)
}

pub fn run_repeatability() -> Result<(), String> {
    run_repeatability_report().map(|_| ())
}
fn run_repeatability_report() -> Result<PathBuf, String> {
    let binary = PathBuf::from(env::args_os().nth(1).ok_or("missing fixture binary")?)
        .canonicalize()
        .map_err(|e| e.to_string())?;
    let normal = PathBuf::from(env::args_os().nth(3).ok_or("missing normal binary")?)
        .canonicalize()
        .map_err(|e| e.to_string())?;
    let package = binary.ancestors().nth(4).ok_or("package layout")?;
    let normal_package = normal.ancestors().nth(4).ok_or("normal package layout")?;
    let epoch = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_nanos();
    let evidence = package.join(format!("navigation-repeatability-{epoch}"));
    fs::create_dir(&evidence).map_err(|e| e.to_string())?;
    println!("Repeatability evidence: {}", evidence.display());
    let identity = hash(&binary)?;
    let source = read(&package.join("QualificationSource.sha256"))?;
    require(
        !source.trim().is_empty()
            && source == read(&normal_package.join("QualificationSource.sha256"))?,
        "source mismatch",
    )?;
    require(
        identity == hash(&normal)?
            && read(&package.join("BuildIdentity.json"))?
                == read(&normal_package.join("BuildIdentity.json"))?,
        "executable/build mismatch",
    )?;
    let manifest = read(&package.join("Manifest_UFSFiles_Linux.txt"))?;
    for asset in [
        "Navigation/W_StaticObstacleDetour.umap",
        "Navigation/DA_StaticObstacleDetour.uasset",
    ] {
        require(manifest.contains(asset), "fixture missing from cook")?;
    }
    require(
        !read(&normal_package.join("Manifest_UFSFiles_Linux.txt"))?.contains("TestFixtures/"),
        "normal cook contains fixtures",
    )?;
    launch(&normal, &evidence, "normal-startup", false)?;
    launch(&binary, &evidence, "fixture-ordinary-startup", false)?;
    let mut summary = String::from(
        "run\tschedule\trepetition\texecutable_sha256\tsource_sha256\tmap\tsteps\tarrival_step\tquery\tmax_position_cm\tmax_waypoint_cm\tmax_length_cm\tmax_travel_cm\tevents\tobstacle\trepetition_equivalent\tverdict\tfirst_divergence\n",
    );
    let mut canonical = String::new();
    let mut failed = false;
    for schedule in 1..=5 {
        let mut first = String::new();
        for repetition in 1..=2 {
            let name = format!("RS-{schedule:02}-rep-{repetition}");
            let dir = evidence.join(&name);
            fs::create_dir(&dir).map_err(|e| e.to_string())?;
            let outcome: Result<String, String> = (|| {
                require(hash(&binary)? == identity, "executable changed mid-matrix")?;
                let log = launch_with_schedule(
                    &binary,
                    &dir,
                    "runtime",
                    true,
                    Some(&format!("RS-{schedule:02}")),
                )?;
                fs::write(
                    dir.join("trace.txt"),
                    log.lines()
                        .filter(|l| l.contains("NAV_") || l.contains("RTS_GAMEPLAY_EVENT"))
                        .collect::<Vec<_>>()
                        .join("\n"),
                )
                .map_err(|e| e.to_string())?;
                let obstacle = verify(&log)?;
                fs::write(dir.join("obstacle.tsv"), obstacle).map_err(|e| e.to_string())?;
                verify_updates(&log, schedule)?;
                if canonical.is_empty() {
                    require(
                        schedule == 1 && repetition == 1,
                        "canonical reference run failed",
                    )?;
                    canonical = log.clone();
                }
                let deviation = compare(&canonical, &log)?;
                if repetition == 1 {
                    first = log.clone();
                } else {
                    compare(&first, &log)?;
                }
                let result = one(&log, "NAV_ACCEPTANCE_RESULT")?;
                Ok(format!(
                    "{name}\tRS-{schedule:02}\t{repetition}\t{identity}\t{}\tmap.test.static_obstacle_detour\t{}\t{}\tSuccessComplete\t{:.6}\t{:.6}\t{:.6}\t{:.6}\tPASS\tPASS\tPASS\tPASS\tnone\n",
                    source.trim(),
                    field(&result, "final_step")?,
                    field(&result, "arrival_step")?,
                    deviation.position,
                    deviation.waypoint,
                    deviation.length,
                    deviation.travel
                ))
            })();
            match outcome {
                Ok(row) => summary.push_str(&row),
                Err(e) => {
                    failed = true;
                    summary.push_str(&format!("{name}\tRS-{schedule:02}\t{repetition}\t{identity}\t{}\tmap.test.static_obstacle_detour\tNA\tNA\tNA\tNA\tNA\tNA\tNA\tINCONCLUSIVE\tINCONCLUSIVE\tINCONCLUSIVE\tFAIL\t{}\n", source.trim(),e.replace(['\n','\t']," ")));
                }
            }
            fs::write(evidence.join("comparison.tsv"), &summary).map_err(|e| e.to_string())?;
        }
    }
    fs::write(
        evidence.join("verdict.tsv"),
        format!(
            "gate\tverdict\nNAV-A20\t{}\n",
            if failed { "FAIL" } else { "PASS" }
        ),
    )
    .map_err(|e| e.to_string())?;
    require(
        !failed,
        "repeatability matrix failed; inspect comparison.tsv",
    )?;
    Ok(evidence)
}

#[cfg(test)]
mod repeatability_tests {
    use super::*;
    fn trace() -> String {
        [
            "NAV_ACCEPTANCE_FIXTURE",
            "NAV_ACCEPTANCE_QUERY",
            "NAV_ACCEPTANCE_PATH_COMPLETE",
            "NAV_ACCEPTANCE_POINT",
            "NAV_ACCEPTANCE_MOVEMENT_SAMPLE",
            "NAV_ACCEPTANCE_TRAVERSE",
            "NAV_ACCEPTANCE_SUBMITTED",
            "NAV_ACCEPTANCE_TERMINATION",
            "NAV_ACCEPTANCE_ARRIVED",
            "NAV_ACCEPTANCE_RESULT",
            "RTS_GAMEPLAY_EVENT",
            "MATCH_AUTHORITATIVE_UNIT",
            "MATCH_PARTICIPANTS_ADMITTED",
        ]
        .map(|marker| format!("{marker} step=1 generation=1 position=1,2,3 order=1\n"))
        .join("")
    }
    #[test]
    fn comparator_rejects_first_step_state_event_and_geometry_divergence() {
        let reference = trace();
        assert!(compare(&reference, &reference).is_ok());
        assert!(
            compare(
                &reference,
                &reference.replace("generation=1", "generation=2")
            )
            .is_ok()
        );
        assert!(compare(&reference, &reference.replacen("order=1", "order=0", 1)).is_err());
        assert!(
            compare(
                &reference,
                &reference.replace("position=1,2,3", "position=1.2,2,3")
            )
            .is_err()
        );
        assert!(compare(&reference, &reference.replace("step=1", "step=2")).is_err());
        assert!(
            compare(
                &reference,
                &reference.replace("RTS_GAMEPLAY_EVENT", "MISSING_EVENT")
            )
            .is_err()
        );
        assert!(
            compare(
                &reference,
                &reference.replace("NAV_ACCEPTANCE_MOVEMENT_SAMPLE", "MISSING_SAMPLE")
            )
            .is_err()
        );
    }
    #[test]
    fn schedule_validation_fails_closed_without_update_or_boundary_evidence() {
        assert!(verify_updates("", 5).is_err());
        assert!(
            verify_updates(
                "NAV_REPEATABILITY_BEGIN schedule=RS-05\nNAV_ACCEPTANCE_SUBMITTED boundary_step=1",
                5
            )
            .is_err()
        );
        assert_eq!(schedule_values(4), &[0.0125, 0.0375]);
        assert_eq!(schedule_values(5), &[0.01, 0.20, 0.04]);
    }
}

// Scenario-aware qualification: all geometry and state originate in Unreal.
fn verify_interruption(log: &str, scenario: &str) -> Result<String, String> {
    let fixture = one(log, "NAV_ACCEPTANCE_FIXTURE")?;
    let result = one(log, "NAV_INTERRUPTION_RESULT")?;
    let terminal = one(log, "NAV_ACCEPTANCE_RESULT")?;
    let submitted = one(log, "NAV_INTERRUPTION_SUBMITTED")?;
    let initial = one(log, "NAV_ACCEPTANCE_SUBMITTED")?;
    require(
        field(&result, "scenario")? == scenario
            && field(&result, "result")? == "PASS"
            && field(&terminal, "result")? == "PASS",
        "Unreal scenario invariant failed",
    )?;
    require(
        log.contains("CONTENT_VALIDATION_PASSED map=map.test.static_obstacle_detour")
            && log.contains("MATCH_PARTICIPANTS_ADMITTED")
            && log.contains("MATCH_LIFECYCLE phase=4"),
        "production validated Active/participant missing",
    )?;
    require(
        field(&fixture, "map")? == "map.test.static_obstacle_detour"
            && field(&fixture, "level")?.ends_with("/Navigation/W_StaticObstacleDetour")
            && field(&fixture, "runtime_generation")? == "0",
        "wrong map/navigation",
    )?;
    let direct = one(log, "NAV_ACCEPTANCE_DIRECT_BLOCKED")?;
    for key in ["hit", "collision", "nav_excluded"] {
        require(field(&direct, key)? == "1", "blocker invalid")?;
    }
    require(
        field(&direct, "actor")? == field(&fixture, "wall")?,
        "unrelated blocker",
    )?;
    require(
        field(&initial, "boundary_step")? == "0"
            && field(&submitted, "boundary")? == "40"
            && field(&submitted, "admission")? == "41",
        "submission boundary changed",
    )?;
    let original = field(&result, "original")?;
    let followup = field(&result, "followup")?;
    require(
        original != followup
            && original == field(&initial, "sequence")?
            && followup == field(&submitted, "sequence")?,
        "command identity mismatch",
    )?;
    let start = point(&fixture, "start")?;
    let wall_min = point(&fixture, "min")?;
    let wall_max = point(&fixture, "max")?;
    let radius = number(&fixture, "radius")?;
    let speed = number(&fixture, "speed")?;
    let queries = records(log, "NAV_ACCEPTANCE_QUERY");
    let paths = records(log, "NAV_ACCEPTANCE_PATH_COMPLETE");
    require(
        queries.len() == if scenario == "OI-02" { 2 } else { 1 } && paths.len() == queries.len(),
        "unexpected route query/install count",
    )?;
    let mut routes = BTreeMap::new();
    for (episode, q) in queries.iter().enumerate() {
        let seq = field(q, "sequence")?;
        require(
            seq == if episode == 0 { original } else { followup }
                && field(q, "result")? == "3"
                && field(q, "partial")? == "0"
                && field(q, "profile")? == "movement.prototype_ground"
                && field(q, "nav")? == field(&fixture, "nav")?,
            "invalid route episode",
        )?;
        for key in ["unit", "generation"] {
            require(
                field(q, key)? == field(&fixture, key)?,
                "route authority mismatch",
            )?;
        }
        let points = records(log, "NAV_ACCEPTANCE_POINT");
        let points: Vec<_> = points
            .iter()
            .filter(|p| p.get("sequence") == Some(&seq))
            .collect();
        let mut route = Vec::new();
        for (i, p) in points.iter().enumerate() {
            require(number(p, "index")? == i as f64, "route geometry gap")?;
            route.push(point(p, "position")?);
        }
        let length: f64 = route.windows(2).map(|s| distance(s[0], s[1])).sum();
        require(
            route.len() >= 2
                && number(q, "points")? == route.len() as f64
                && (length - number(&paths[episode], "length")?).abs() < 0.01
                && field(&paths[episode], "valid")? == "1"
                && route
                    .windows(2)
                    .all(|s| !sphere_touches(s[0], s[1], wall_min, wall_max, radius)),
            "incomplete/colliding route",
        )?;
        require(
            distance(*route.last().unwrap(), point(q, "resolved")?) < 0.01,
            "route endpoint mismatch",
        )?;
        if episode == 0 {
            require(
                distance(route[0], start) < 0.01
                    && length > distance(start, point(&fixture, "goal")?) + 0.1,
                "initial detour missing",
            )?;
        } else {
            require(
                distance(point(q, "resolved")?, start) <= 0.1,
                "replacement not original start",
            )?;
        }
        routes.insert(seq, route);
    }
    let events = records(log, "RTS_GAMEPLAY_EVENT");
    let commands: Vec<_> = events
        .iter()
        .filter(|e| e.get("kind") == Some(&"CommandOutcome"))
        .collect();
    let transitions: Vec<_> = events
        .iter()
        .filter(|e| e.get("kind") == Some(&"OrderChanged"))
        .collect();
    require(
        events.len() == commands.len() + transitions.len(),
        "unrelated gameplay event",
    )?;
    require(
        commands.len() == if scenario == "OI-01" { 3 } else { 2 },
        "command outcome count",
    )?;
    for (i, c) in commands.iter().enumerate() {
        let seq = match i {
            0 => original,
            1 => followup,
            _ => field(&result, "duplicate")?,
        };
        let rejected = scenario == "OI-03" && i == 1;
        require(
            field(c, "sequence")? == seq
                && field(c, "correlation")? == seq
                && number(c, "step")? == [1., 41., 61.][i]
                && field(c, "entity")? == field(&fixture, "unit")?
                && field(c, "accepted")? == if rejected { "0" } else { "1" },
            "command admission identity/timing",
        )?;
        // Exact enum value is taken from the pinned portable contract, not a navigation failure.
        require(
            number(c, "rejection")? == if rejected { 5. } else { 0. },
            "incorrect rejection enum",
        )?;
    }
    if scenario == "OI-03" {
        let destination = point(&submitted, "destination")?;
        let bounds: Vec<f64> = field(&submitted, "bounds_max")?
            .split(',')
            .map(|s| s.parse().unwrap_or(f64::NAN))
            .collect();
        require(
            bounds.len() == 2 && destination[0] > bounds[0] && destination[1] > bounds[1],
            "request not finite out of bounds",
        )?;
    }
    let mut expected = vec![(1., "0", "1", "0")];
    if scenario == "OI-01" {
        expected.push((41., "1", "4", "2"));
    } else if scenario == "OI-02" {
        expected.push((41., "1", "1", "1"));
    }
    let arrival = if scenario == "OI-01" {
        0.
    } else {
        number(&terminal, "arrival_step")?
    };
    if arrival > 0. {
        expected.push((arrival, "1", "0", "5"));
    }
    require(
        transitions.len() == expected.len(),
        "duplicate/stale order transition",
    )?;
    for (e, (step, prev, current, reason)) in transitions.iter().zip(expected) {
        require(
            number(e, "step")? == step
                && field(e, "previous")? == prev
                && field(e, "current")? == current
                && field(e, "reason")? == reason
                && field(e, "entity")? == field(&fixture, "unit")?,
            "order event timing/termination diverged",
        )?;
    }
    let samples = records(log, "NAV_ACCEPTANCE_MOVEMENT_SAMPLE");
    require(
        samples.len() >= 81 && samples.len() as f64 == number(&terminal, "final_step")?,
        "trace incomplete",
    )?;
    let boundary = point(&samples[39], "position")?;
    require(
        field(&samples[39], "order")? == "1"
            && field(&samples[39], "sequence")? == original
            && number(&samples[39], "path_points")? >= 3.
            && distance(boundary, start) > 0.1
            && distance(boundary, point(&fixture, "goal")?) > 18.,
        "original not executing at step40",
    )?;
    let admissions = records(log, "NAV_INTERRUPTION_ADMITTED");
    require(
        !admissions.is_empty()
            && field(&admissions[0], "step")? == "41"
            && distance(point(&admissions[0], "position")?, boundary) < 0.001,
        "admission moved before step41",
    )?;
    if scenario == "OI-02" {
        require(
            distance(
                queries[1]
                    .get("projected_start")
                    .map(|_| point(&queries[1], "projected_start"))
                    .transpose()?
                    .unwrap(),
                boundary,
            ) <= 150.,
            "replacement queried wrong origin",
        )?;
    }
    let segments = records(log, "NAV_ACCEPTANCE_TRAVERSE");
    let mut previous = start;
    let mut segment_count = 0;
    let mut max_travel = 0_f64;
    for (i, s) in samples.iter().enumerate() {
        let step = (i + 1) as f64;
        require(number(s, "step")? == step, "missing/duplicate step")?;
        for key in ["unit", "owner", "generation"] {
            require(
                field(s, key)? == field(&fixture, key)?,
                "sample authority mismatch",
            )?;
        }
        require(
            field(s, "clear")? == "1"
                && field(s, "valid")? == "1"
                && field(s, "active")? == "1"
                && field(s, "failures")? == "0",
            "movement invariant",
        )?;
        let stopped = scenario == "OI-01" && step >= 41.;
        let seq = if stopped {
            "0"
        } else if scenario == "OI-02" && step >= 41. {
            followup
        } else {
            original
        };
        require(
            field(s, "sequence")? == seq
                && field(s, "nav_generation")?
                    == if stopped {
                        "0"
                    } else {
                        field(&fixture, "generation")?
                    },
            "stale executable route",
        )?;
        let mut travel = 0.;
        let mut count = 0;
        for p in segments.iter().filter(|p| p.get("step") == s.get("step")) {
            require(number(p, "index")? == count as f64, "sweep gap")?;
            let next = point(p, "position")?;
            let route = routes.get(seq).ok_or("movement without installed route")?;
            require(
                !sphere_touches(previous, next, wall_min, wall_max, radius)
                    && route_distance(next, route) <= 2.1,
                "obstacle penetration/stale route traversal",
            )?;
            travel += distance(previous, next);
            previous = next;
            count += 1;
            segment_count += 1;
        }
        require(
            distance(previous, point(s, "position")?) < 0.0001
                && (travel - number(s, "travel")?).abs() < 0.0001
                && travel <= speed * 0.05 + 0.01
                && (number(s, "budget")? - speed * 0.05).abs() < 0.0001
                && number(s, "subsegments")? == count as f64,
            "unobserved/illegal movement",
        )?;
        max_travel = max_travel.max(travel);
        if stopped {
            require(
                distance(previous, boundary) <= 0.001
                    && field(s, "order")? == "4"
                    && field(s, "waypoint")? == "0"
                    && field(s, "path_points")? == "0",
                "Stop not retired/stationary",
            )?;
        } else if step >= arrival && arrival > 0. {
            require(
                field(s, "order")? == "0"
                    && field(s, "waypoint")? == "0"
                    && field(s, "path_points")? == "0",
                "arrival route not retired",
            )?;
        } else {
            require(
                field(s, "order")? == "1"
                    && number(s, "waypoint")?
                        < routes.get(seq).ok_or("route missing")?.len() as f64,
                "wrong active cursor/order",
            )?;
        }
    }
    require(segment_count == segments.len(), "unattributed sweep")?;
    if scenario == "OI-01" {
        require(
            samples.len() == 81
                && field(&result, "stopped")? == "1"
                && field(&result, "arrivals")? == "0"
                && records(log, "NAV_ACCEPTANCE_TERMINATION").is_empty()
                && records(log, "NAV_ACCEPTANCE_ARRIVED").is_empty(),
            "Stop duplicate/arrival failure",
        )?;
        one(log, "NAV_INTERRUPTION_DUPLICATE")?;
    } else {
        let ended = one(log, "NAV_ACCEPTANCE_TERMINATION")?;
        let arrived = one(log, "NAV_ACCEPTANCE_ARRIVED")?;
        require(
            number(&ended, "step")? == arrival
                && field(&ended, "reason")? == "5"
                && field(&ended, "sequence")?
                    == if scenario == "OI-02" {
                        followup
                    } else {
                        original
                    }
                && number(&arrived, "step")? == arrival
                && samples.len() as f64 == arrival + 20.
                && distance(previous, point(queries.last().unwrap(), "resolved")?) <= 18.,
            "exactly once arrival/hold failure",
        )?;
    }
    Ok(format!(
        "scenario\tverdict\tsteps\tarrival_step\tqueries\tmax_travel_cm\n{scenario}\tPASS\t{}\t{arrival}\t{}\t{max_travel:.6}\n",
        samples.len(),
        queries.len()
    ))
}

fn compare_interruption(
    reference: &str,
    actual: &str,
    continuity: bool,
) -> Result<Deviation, String> {
    let mut max = Deviation::default();
    let mut markers = vec![
        "NAV_ACCEPTANCE_QUERY",
        "NAV_ACCEPTANCE_POINT",
        "NAV_ACCEPTANCE_PATH_COMPLETE",
        "NAV_ACCEPTANCE_MOVEMENT_SAMPLE",
        "NAV_ACCEPTANCE_TRAVERSE",
        "NAV_ACCEPTANCE_TERMINATION",
        "NAV_ACCEPTANCE_ARRIVED",
        "RTS_GAMEPLAY_EVENT",
    ];
    if !continuity {
        markers.extend([
            "NAV_INTERRUPTION_SUBMITTED",
            "NAV_INTERRUPTION_DUPLICATE",
            "NAV_INTERRUPTION_ADMITTED",
            "NAV_INTERRUPTION_RESULT",
            "NAV_ACCEPTANCE_RESULT",
        ]);
    }
    for marker in markers {
        let filter = |e: &Record<'_>| {
            !(continuity
                && marker == "RTS_GAMEPLAY_EVENT"
                && e.get("kind") == Some(&"CommandOutcome")
                && e.get("accepted") == Some(&"0"))
        };
        let a: Vec<_> = records(reference, marker)
            .into_iter()
            .filter(filter)
            .collect();
        let b: Vec<_> = records(actual, marker).into_iter().filter(filter).collect();
        require(
            a.len() == b.len(),
            &format!("{marker} record count {} != {}", a.len(), b.len()),
        )?;
        for (i, (a, b)) in a.iter().zip(&b).enumerate() {
            require(a.keys().eq(b.keys()), "record fields changed")?;
            for (key, value) in a {
                if *key == "generation" || *key == "nav_generation" && *value != "0" {
                    continue;
                }
                let detail = format!(
                    "{marker} record={i} step={:?} field={key} expected={value} actual={:?}",
                    a.get("step"),
                    b.get(key)
                );
                if [
                    "position",
                    "projected_start",
                    "projected_goal",
                    "resolved",
                    "destination",
                ]
                .contains(key)
                {
                    let d = distance(point(a, key)?, point(b, key)?);
                    if marker == "NAV_ACCEPTANCE_POINT" {
                        max.waypoint = max.waypoint.max(d);
                    } else {
                        max.position = max.position.max(d);
                    }
                    require(d <= 0.1, &detail)?;
                } else if *key == "length" {
                    let d = (number(a, key)? - number(b, key)?).abs();
                    max.length = max.length.max(d);
                    require(d <= 0.25, &detail)?;
                } else if ["travel", "max_travel"].contains(key) {
                    let d = (number(a, key)? - number(b, key)?).abs();
                    max.travel = max.travel.max(d);
                    require(d <= 0.1, &detail)?;
                } else {
                    require(b.get(key) == Some(value), &detail)?;
                }
            }
        }
    }
    Ok(max)
}

pub fn run_interruption() -> Result<(), String> {
    let binary = PathBuf::from(env::args_os().nth(1).ok_or("missing fixture binary")?)
        .canonicalize()
        .map_err(|e| e.to_string())?;
    let normal = PathBuf::from(env::args_os().nth(3).ok_or("missing normal binary")?)
        .canonicalize()
        .map_err(|e| e.to_string())?;
    let package = binary.ancestors().nth(4).ok_or("package layout")?;
    let normal_package = normal.ancestors().nth(4).ok_or("package layout")?;
    let evidence = package.join(format!(
        "navigation-interruption-{}",
        SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .map_err(|e| e.to_string())?
            .as_nanos()
    ));
    fs::create_dir(&evidence).map_err(|e| e.to_string())?;
    println!("Interruption evidence: {}", evidence.display());
    let identity = hash(&binary)?;
    let source = read(&package.join("QualificationSource.sha256"))?;
    require(
        !source.trim().is_empty()
            && source == read(&normal_package.join("QualificationSource.sha256"))?
            && identity == hash(&normal)?
            && read(&package.join("BuildIdentity.json"))?
                == read(&normal_package.join("BuildIdentity.json"))?,
        "package identity mismatch",
    )?;
    let manifest = read(&package.join("Manifest_UFSFiles_Linux.txt"))?;
    require(
        manifest.contains("Navigation/W_StaticObstacleDetour.umap")
            && manifest.contains("Navigation/DA_StaticObstacleDetour.uasset")
            && !read(&normal_package.join("Manifest_UFSFiles_Linux.txt"))?
                .contains("TestFixtures/"),
        "package isolation failed",
    )?;
    fs::write(
        evidence.join("identity.txt"),
        format!(
            "binary={}\nsha256={identity}\nsource={}\n",
            binary.display(),
            source.trim()
        ),
    )
    .map_err(|e| e.to_string())?;
    let mut summary = String::from(
        "scenario\tschedule\trepetition\tverdict\tposition_cm\twaypoint_cm\tlength_cm\ttravel_cm\tfirst_divergence\n",
    );
    let mut failed = false;
    for scenario in ["OI-01", "OI-02", "OI-03"] {
        let mut canonical = String::new();
        for schedule in [2, 5] {
            let mut first = String::new();
            for repetition in 1..=2 {
                let name = format!("{scenario}-RS-{schedule:02}-rep-{repetition}");
                let dir = evidence.join(&name);
                fs::create_dir(&dir).map_err(|e| e.to_string())?;
                let outcome: Result<Deviation, String> = (|| {
                    require(hash(&binary)? == identity, "executable changed")?;
                    let log = launch_scenario(
                        &binary,
                        &dir,
                        "runtime",
                        true,
                        Some(&format!("RS-{schedule:02}")),
                        Some(scenario),
                    )?;
                    let report = verify_interruption(&log, scenario)?;
                    fs::write(dir.join("result.tsv"), report).map_err(|e| e.to_string())?;
                    verify_updates(&log, schedule)?;
                    if canonical.is_empty() {
                        canonical = log.clone();
                    }
                    let d = compare_interruption(&canonical, &log, false)?;
                    if repetition == 1 {
                        first = log;
                    } else {
                        compare_interruption(&first, &log, false)?;
                    }
                    Ok(d)
                })();
                let row = match outcome {
                    Ok(d) => format!(
                        "{scenario}\tRS-{schedule:02}\t{repetition}\tPASS\t{}\t{}\t{}\t{}\tnone\n",
                        d.position, d.waypoint, d.length, d.travel
                    ),
                    Err(e) => {
                        failed = true;
                        format!(
                            "{scenario}\tRS-{schedule:02}\t{repetition}\tFAIL\tNA\tNA\tNA\tNA\t{}\n",
                            e.replace(['\n', '\t'], " ")
                        )
                    }
                };
                fs::write(
                    dir.join("comparison.tsv"),
                    format!("{}{}", summary.lines().next().unwrap(), format!("\n{row}")),
                )
                .map_err(|e| e.to_string())?;
                summary.push_str(&row);
                fs::write(evidence.join("matrix.tsv"), &summary).map_err(|e| e.to_string())?;
            }
        }
    }
    // The required ten-run regression follows the twelve interruption processes.
    // Its uninterrupted traces are reused; no duplicate reference processes.
    let reference = run_repeatability_report()?;
    fs::write(
        evidence.join("reference.txt"),
        reference.display().to_string(),
    )
    .map_err(|e| e.to_string())?;
    for schedule in [2, 5] {
        for repetition in 1..=2 {
            let dir = evidence.join(format!("OI-03-RS-{schedule:02}-rep-{repetition}"));
            let outcome = (|| {
                let original =
                    read(&reference.join(format!("RS-{schedule:02}-rep-1/runtime.log")))?;
                let log = read(&dir.join("runtime.log"))?;
                verify(&original)?;
                compare_interruption(&original, &log, true)
            })();
            let row = match outcome {
                Ok(d) => format!(
                    "PASS\t{}\t{}\t{}\t{}\tnone\n",
                    d.position, d.waypoint, d.length, d.travel
                ),
                Err(e) => {
                    failed = true;
                    format!("FAIL\tNA\tNA\tNA\tNA\t{}\n", e.replace(['\n', '\t'], " "))
                }
            };
            fs::write(dir.join("continuity.tsv"),format!("verdict\tposition_cm\twaypoint_cm\tlength_cm\ttravel_cm\tfirst_divergence\n{row}")).map_err(|e|e.to_string())?;
        }
    }
    fs::write(
        evidence.join("verdict.tsv"),
        format!(
            "gate\tverdict\nInterruption\t{}\n",
            if failed { "FAIL" } else { "PASS" }
        ),
    )
    .map_err(|e| e.to_string())?;
    require(!failed, "interruption matrix failed; inspect matrix.tsv")
}

#[cfg(test)]
mod interruption_tests {
    use super::*;
    #[test]
    fn rejected_outcome_is_only_allowed_continuity_difference() {
        let reference = "RTS_GAMEPLAY_EVENT kind=CommandOutcome step=1 sequence=1 accepted=1\nNAV_ACCEPTANCE_MOVEMENT_SAMPLE step=41 position=1,2,3 waypoint=2 order=1 sequence=1\n";
        let rejected = format!(
            "{reference}RTS_GAMEPLAY_EVENT kind=CommandOutcome step=41 sequence=2 accepted=0\n"
        );
        assert!(compare_interruption(reference, &rejected, true).is_ok());
        assert!(compare_interruption(reference, &rejected, false).is_err());
        for (from, to) in [
            ("waypoint=2", "waypoint=1"),
            ("sequence=1", "sequence=2"),
            ("order=1", "order=4"),
            ("position=1,2,3", "position=1.2,2,3"),
        ] {
            assert!(compare_interruption(reference, &rejected.replace(from, to), true).is_err());
        }
        assert!(compare_interruption(reference, "", true).is_err());
        assert!(verify_interruption("NAV_INTERRUPTION_RESULT result=PASS", "OI-01").is_err());
    }
}
