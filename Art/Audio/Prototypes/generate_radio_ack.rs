//! Render a tiny, original POLITIKA radio-acknowledgement sound prototype.
//! Usage: rustc generate_radio_ack.rs -O -o generate_radio_ack && ./generate_radio_ack [output.wav]
//! No third-party crates or borrowed samples are used.

use std::{env, fs::File, io::{self, Write}};

const RATE: u32 = 48_000;

fn write_u16(out: &mut impl Write, v: u16) -> io::Result<()> { out.write_all(&v.to_le_bytes()) }
fn write_u32(out: &mut impl Write, v: u32) -> io::Result<()> { out.write_all(&v.to_le_bytes()) }

fn main() -> io::Result<()> {
    let path = env::args().nth(1).unwrap_or_else(|| "politika-radio-ack.wav".into());
    let count = (RATE as f32 * 0.42) as usize;
    let mut samples = Vec::with_capacity(count);
    let mut seed = 0x1984_1017_u32;
    for i in 0..count {
        let t = i as f32 / RATE as f32;
        // Brief filtered-noise-like squelch, followed by two clear, non-musical tones.
        seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
        let noise = ((seed as i32 as f32) / i32::MAX as f32) * 0.10;
        let (tone, env) = if (0.075..0.155).contains(&t) {
            ((2.0 * std::f32::consts::PI * 980.0 * t).sin(), ((t - 0.075) / 0.008).min(1.0) * ((0.155 - t) / 0.018).min(1.0))
        } else if (0.190..0.315).contains(&t) {
            ((2.0 * std::f32::consts::PI * 740.0 * t).sin(), ((t - 0.190) / 0.010).min(1.0) * ((0.315 - t) / 0.025).min(1.0))
        } else { (0.0, 0.0) };
        let hiss_env = if t < 0.050 { (t / 0.006).min(1.0) * ((0.050 - t) / 0.018).min(1.0) } else { 0.0 };
        // Gentle deterministic flutter keeps the cue from sounding like a bare UI sine beep.
        let flutter = 0.92 + 0.08 * (2.0 * std::f32::consts::PI * 17.0 * t).sin();
        let value = (noise * hiss_env + tone * env * 0.27 * flutter).clamp(-0.9, 0.9);
        samples.push((value * i16::MAX as f32) as i16);
    }

    let mut f = File::create(path)?;
    f.write_all(b"RIFF")?; write_u32(&mut f, 36 + (samples.len() * 2) as u32)?;
    f.write_all(b"WAVEfmt ")?; write_u32(&mut f, 16)?; write_u16(&mut f, 1)?;
    write_u16(&mut f, 1)?; write_u32(&mut f, RATE)?; write_u32(&mut f, RATE * 2)?;
    write_u16(&mut f, 2)?; write_u16(&mut f, 16)?; f.write_all(b"data")?;
    write_u32(&mut f, (samples.len() * 2) as u32)?;
    for sample in samples { f.write_all(&sample.to_le_bytes())?; }
    Ok(())
}
