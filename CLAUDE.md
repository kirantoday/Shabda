# CLAUDE.md —  Veena Physical Modeling Plugin

## Project Overview

We are building a physically-modeled ** Veena** virtual instrument plugin using **C++ and JUCE**. This is the first instrument in a planned platform for Eastern/Indian instruments using physical modeling synthesis (similar in philosophy to Audio Modeling's SWAM, but for Indian instruments).

Future instruments on the same engine: Dilruba, Sitar, Oud, Saz, Tanpura, Sarangi, Bansuri, Erhu, Tabla, and others.

## About the Developer

- C++/VC++ background (early career), currently primarily Node.js/TypeScript
- Using Claude Code (Max) as pair programmer
- Primary dev OS: Windows (Visual Studio 2022, CMake)
- Target platforms: Windows, Mac, iOS (later)
- Wants production-quality architecture from day one — the engine will be reused across 10+ instruments
- Learning DSP concepts as we build — explain the math and reasoning behind design choices

## About the Instrument:  Veena

The  Veena is a large South Indian plucked string instrument with:

- **4 main strings** (typically tuned Sa Pa sa Pa or similar) played on a long fretted neck with 24 fixed frets
- **3 side drone/tala strings** (thalam strings) plucked openly for rhythmic punctuation
- **A large resonator body (kudam)** made from jackfruit wood with a flat bridge
- **A secondary gourd resonator** at the top of the neck
- **Deep pitch bending capability (meend/gamaka)** — the player pushes the string laterally across the fret to bend pitch, sometimes up to a 4th or 5th — this is THE defining expressive feature of the veena
- **Bright, sustaining tone** with prominent attack transient
- **Sympathetic-like resonance** from open strings ringing when fretted notes are played

### Key Musical Behaviors to Model

- **Gamaka/Meend**: Extreme pitch bending by lateral string deflection. Smooth, continuous, can be slow or fast. This is not optional decoration — it IS veena music.
- **Pluck attack**: Varies with nail angle, speed, and position. Bright near bridge, mellow near neck.
- **Thalam strings**: Short rhythmic plucks on open side strings for tala punctuation.
- **Open string resonance**: Unfretted strings ring sympathetically when harmonically related notes are played.
- **Janta (double pluck)**: Rapid repeated pluck ornament.
- **Kampita**: Oscillating pitch ornament (a form of vibrato but specifically Indian in character).
- **Decay character**: Long sustain, gradually losing brightness — the kudam body sustains well.

---

## Architecture

### Shared Engine Design

The DSP engine has ZERO dependency on JUCE. Only `/plugin` and `/ui` layers depend on JUCE. This allows engine reuse across instruments and potentially other host frameworks.

```
/VeenaPlugin
  /engine                    ← shared DSP core (reusable across all instruments)
    /waveguide               ← string waveguide models
      DelayLine.h/cpp        ← circular buffer with fractional delay interpolation
      PluckedString.h/cpp    ← Karplus-Strong extended / digital waveguide string
      StringModel.h/cpp      ← abstract base for string types (plucked, bowed later)
    /exciter
      PluckExciter.h/cpp     ← filtered noise burst / impulse generation
      BowExciter.h/cpp       ← (future: bow-string friction model for dilruba/sarangi)
    /resonator
      BodyResonator.h/cpp    ← modal filter bank for body resonance
      ConvolutionBody.h/cpp  ← (future: IR convolution alternative)
      ModalFilter.h/cpp      ← single biquad resonant filter
    /sympathetic
      SympatheticBank.h/cpp  ← bank of tuned resonators excited by main string energy
    /expression
      ParameterSmoother.h/cpp ← one-pole exponential smoothing, no zipper noise
      PitchBendEngine.h/cpp   ← high-resolution pitch modulation with curve shaping
      MidiMapper.h/cpp        ← MIDI CC / MPE → engine parameter routing
    /common
      DSPConstants.h          ← sample rate, pi, tuning constants
      Filters.h/cpp           ← biquad, one-pole, comb filter implementations
      Interpolation.h/cpp     ← linear, allpass, cubic interpolation
      TuningSystem.h/cpp      ← equal temperament + Indian shruti/swara tuning tables
  /instruments
    /veena
      VeenaVoice.h/cpp        ← assembles engine components into a veena instrument
      VeenaConfig.h            ← string tunings, body filter presets, behavior params
      VeenaPresets.h/cpp       ← raga-specific tuning and expression presets
  /ui
    MainEditor.h/cpp           ← JUCE UI panel
    StringVisualizer.h/cpp     ← visual feedback of string vibration
    KnobStyles.h/cpp           ← custom look and feel (later)
  /plugin
    PluginProcessor.h/cpp      ← JUCE AudioProcessor, hosts VeenaVoice
    PluginEditor.h/cpp         ← JUCE AudioProcessorEditor, hosts MainEditor
  CMakeLists.txt
  CLAUDE.md                    ← this file
```

---

## Build Phases

### Phase 1: Foundation (current)

#### Step 1: Project Setup
- [x] Set up JUCE project with CMake build system
- [x] Configure for VST3 + Standalone on Windows
- [x] Create folder structure
- [x] Verify compiles and outputs silence in standalone mode

#### Step 2: Core Plucked String Waveguide
- [x] Implement circular buffer delay line with fractional delay (allpass interpolation preferred for quality)
- [x] Karplus-Strong extended: delay line + lowpass loop filter
- [x] Accurate pitch tuning via delay length calculation from MIDI note number
- [x] Excitation: parameterizable filtered noise burst (pluck impulse)
- [x] Parameters: pitch, pluck position, pluck strength, damping, brightness
- [x] Monophonic first — one clean plucked string

#### Step 3: Pitch Bend / Meend Engine
- [x] Smooth continuous pitch modulation by varying delay line length in real time
- [x] Pitch bend range: at least ±7 semitones (veena gamaka can reach ±5 or more)
- [x] 14-bit MIDI pitch bend support (high resolution)
- [x] Curve shaping: linear, exponential, S-curve options
- [x] Zero zipper noise — fractional delay interpolation must handle continuous modulation cleanly

#### Step 4: Body Resonance Module
- [x] Bank of parametric biquad resonant filters (modal resonator approach)
- [x] Default preset: approximate  Veena kudam resonance (warm, woody, low-mid prominence)
- [x] Configurable filter frequencies, Q, and gain per mode
- [x] Swappable for future instruments (different body = different filter bank preset)

#### Step 5: Sympathetic / Open String Resonance
- [x] 3-4 simplified string resonators for open drone strings
- [x] Excited by energy transfer from main string when frequencies align
- [x] Comb filters or simplified waveguides tuned to standard veena drone tuning
- [x] Subtle but audible — adds the "halo" / bloom of veena tone

#### Step 6: Expression Mapping
- [x] MIDI Note On velocity → pluck strength + brightness
- [x] Pitch Bend → meend engine (high res, wide range)
- [x] CC1 (Mod Wheel) → vibrato depth (kampita)
- [x] CC11 (Expression) → overall volume/energy
- [x] Aftertouch → brightness or damping modulation
- [x] All parameters smoothed via one-pole exponential filter

#### Step 7: Basic JUCE UI
- [x] Sliders: pluck position, damping, brightness, body resonance mix, sympathetic volume
- [x] Pitch bend range selector
- [x] Tuning preset selector (Sa = C3, Sa = D3, etc.)
- [x] Simple level meter or string vibration indicator
- [x] Functional, not pretty — aesthetics come later

### Phase 2: Expressivity & Polish (in progress)
- [x] Automatic legato (smooth pitch glide between notes without replucking)
- [x] Meend curve profiles (Linear, Exponential, S-Curve, Late) with GlideEngine
- [x] Polyphonic mode (2 voices for double-stop techniques, voice stealing)
- [x] Humanization (pitch/brightness/timing jitter per pluck)
- [x] Thalam (side drone) strings with independent trigger (Z/X/C keys)
- [ ] Multi-voice: 4 main strings playable independently or via keyswitch
- [x] Raga-aware sympathetic tuning presets (7 ragas with UI dropdown)
- [ ] IR convolution body resonance option (hybrid with modal filters)

### Phase 3: Productization (future)
- [ ] Professional UI with Indian aesthetic
- [ ] VST3 / AU / AUv3 (iOS) / Standalone
- [ ] Preset library for film scoring / Carnatic classical / fusion
- [ ] MIDI mapping templates for keyboards, MPE controllers, breath controllers
- [ ] Installer / license system
- [ ] User manual

---

## Technical Constraints

- **Sample rates**: support 44100 and 48000 Hz (96000 optional later)
- **Buffer size**: must work at 128 samples (low latency performance)
- **No heap allocation in processBlock** — preallocate everything in prepareToPlay
- **CPU target**: single string under 5% on modern hardware
- **Precision**: float32 for DSP (sufficient for this application)
- **C++ standard**: C++17 minimum
- **RAII everywhere**, no raw new/delete in DSP code
- **Engine code must not #include any JUCE headers** — pure C++ only
- **SIMD**: use where straightforward, but don't over-optimize prematurely

## Coding Standards

- Descriptive naming, no abbreviations in public APIs
- `camelCase` for methods and variables, `PascalCase` for classes
- Comments explaining DSP math and design choices, not just "what" but "why"
- Mark parameters needing perceptual tuning with `// TODO(TUNE): description`
- Mark future instrument extension points with `// EXTENSION: description`
- Each DSP class should have a `reset()` method and a `prepare(sampleRate, maxBlockSize)` method
- All DSP processing methods take buffer pointers and sample counts — no JUCE types

## Naming Conventions

- Temporary / diagnostic files: prefix with `_tmp_`
- Test files: prefix with `test_`
- Engine source files: no prefix, descriptive name matching class name

## Design Decisions Log

Record significant architectural or DSP decisions here as they are made:

| Date | Decision | Rationale |
|------|----------|-----------|
| — | Allpass interpolation for fractional delay | Better pitch accuracy than linear at minimal CPU cost |
| — | Modal filter bank over IR convolution for v1 body | Lower latency, easier to parameterize per instrument, IR added later as hybrid option |
| — | Engine decoupled from JUCE | Enables reuse across instruments and potential non-JUCE targets (iOS AudioUnit, etc.) |
| — | Monophonic first | Get one string sounding right before adding complexity |
| — | Tanpura-style sympathetic resonance approach | Simpler than full waveguide per sympathetic string, sufficient for v1 |

## How Claude Code Should Work With Me

1. **Start each session by reading this file** to understand project state
2. **Proceed step by step** — verify each step compiles and works before moving on
3. **Explain DSP concepts** as you implement them — I want to understand the math
4. **When making design decisions** (e.g., interpolation type, filter topology), explain tradeoffs and recommend one
5. **Flag parameters needing ear-tuning** with `// TODO(TUNE):` comments
6. **After each step, tell me how to test**: what to play, what to listen for, what should sound wrong vs right
7. **Update the checkboxes in this file** as steps are completed
8. **Do not skip ahead** — foundation quality matters more than feature count

## Future Instruments (same engine)

These will reuse `/engine` with instrument-specific configs in `/instruments`:

| Instrument | Family | New DSP Needed |
|-----------|--------|----------------|
| Tanpura | Plucked drone | Jawari bridge nonlinearity, 4-string cycling pattern |
| Dilruba | Bowed + sympathetic | BowExciter, bow-string friction model, extended sympathetic bank |
| Sitar | Plucked + sympathetic | Jawari bridge, wider sympathetic bank (13 strings), different body |
| Sarangi | Bowed + sympathetic | Skin body model, gut string params, fretless pitch, 36+ sympathetic strings |
| Oud | Plucked fretless | Fretless pitch model, risha (pick) attack, different body resonance |
| Saz/Bağlama | Plucked | Long neck model, specific body resonance, Turkish tuning systems |
| Bansuri | Wind | Bore waveguide (new engine branch), breath noise, embouchure |
| Tabla | Percussion | Modal synthesis (new engine branch), composite membrane model |
| Erhu | Bowed | BowExciter reuse, skin resonator, 2-string only, no frets |

---

## References

- Julius O. Smith — *Physical Audio Signal Processing* (online, Stanford/CCRMA)
- Perry Cook & Gary Scavone — STK (Synthesis Toolkit in C++)
- Faust `pm.lib` physical modeling library (reference implementations)
- Will Pirkle — *Designing Audio Effect Plugins in C++* (JUCE-oriented)
- Audio Modeling SWAM — public product descriptions and design philosophy
- JUCE documentation and tutorials: https://juce.com/learn/