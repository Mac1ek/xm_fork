# C++ XM Module Player - TODO List

## High Priority (Core Functionality)

### XM File Loader
- [ ] Implement XM format parser (xmloader.cpp)
  - [ ] Parse XM header (module name, version, tracker)
  - [ ] Load pattern data (notes, effects)
  - [ ] Load instrument data (envelopes, samples)
  - [ ] Load sample data (8-bit, 16-bit, compressed)
  - [ ] Handle XM file format variations (1.02, 1.04)
- [ ] Implement MOD format parser
  - [ ] Parse MOD header (31-instrument format)
  - [ ] Load patterns (M.K., M!K!, FLT4, etc.)
  - [ ] Convert ProTracker samples to XM format
  - [ ] Map MOD effects to XM effects
- [ ] Implement XI instrument file loader
  - [ ] Parse XI header
  - [ ] Load envelope data
  - [ ] Load sample data

### Complete Player Engine (xmplayer.cpp)
- [ ] Implement tick-based sequencer
  - [ ] Main playback loop (playTick)
  - [ ] Row processing (processRow)
  - [ ] Pattern advancement
  - [ ] Song looping detection
- [ ] Implement pattern/row processing
  - [ ] Read notes from current pattern
  - [ ] Handle note on/off
  - [ ] Initialize samples on mixer
  - [ ] Parse effect commands
  - [ ] Handle volume column
- [ ] Implement complete effect system (50+ effects)
  - [ ] **Volume Effects**
    - [ ] Cxx - Set volume
    - [ ] Axx - Volume slide up
    - [ ] Dxx - Volume slide down
    - [ ] EAx - Fine volume slide up
    - [ ] EBx - Fine volume slide down
    - [ ] 7xx - Tremolo
  - [ ] **Pitch Effects**
    - [ ] 1xx - Portamento up
    - [ ] 2xx - Portamento down
    - [ ] 3xx - Tone portamento
    - [ ] 4xx - Vibrato
    - [ ] 0xx - Arpeggio
    - [ ] E1x - Fine portamento up
    - [ ] E2x - Fine portamento down
    - [ ] X1x - Extra fine portamento up
    - [ ] X2x - Extra fine portamento down
    - [ ] E3x - Glissando control
    - [ ] E4x - Vibrato waveform
    - [ ] E5x - Set finetune
  - [ ] **Pattern Flow**
    - [ ] Bxx - Pattern jump
    - [ ] Dxx - Pattern break
    - [ ] E6x - Pattern loop
    - [ ] EEx - Pattern delay
  - [ ] **Timing**
    - [ ] Fxx - Set tempo/BPM
    - [ ] FEx - Fine pattern delay
    - [ ] 9xx - Retrigger note
    - [ ] Rxx - Multi retrigger note
  - [ ] **Sample**
    - [ ] 9xx - Sample offset
    - [ ] ECx - Note cut
    - [ ] EDx - Note delay
    - [ ] Kxx - Key off
    - [ ] Lxx - Set envelope position
  - [ ] **Global**
    - [ ] Gxx - Set global volume
    - [ ] Hxx - Global volume slide
    - [ ] 8xx - Set panning
    - [ ] Pxx - Panning slide
    - [ ] E8x - Set fine panning
    - [ ] 25x - Panning slide (volume column)
  - [ ] **Filter**
    - [ ] Zxx - Set filter cutoff
    - [ ] Qxx - Set filter resonance
- [ ] Implement envelope processing
  - [ ] Volume envelope handler
  - [ ] Panning envelope handler
  - [ ] Sustain point handling
  - [ ] Loop handling
  - [ ] Fade-out calculation
- [ ] Implement frequency/pitch algorithms
  - [ ] Complete pitchToFreq for linear mode
  - [ ] Implement Amiga frequency mode
  - [ ] Create period-to-frequency tables
  - [ ] Implement note-to-pitch conversion
  - [ ] Handle finetune and relative note

### Frequency Tables
- [ ] Generate ProTracker period tables
  - [ ] Create 16 finetune tables
  - [ ] Create hnotetab arrays
  - [ ] Implement cubic interpolation
- [ ] Implement vibrato/tremolo waveforms
  - [ ] Sine wave table (256 entries)
  - [ ] Square wave table
  - [ ] Ramp down table
  - [ ] Random wave table

## Medium Priority (Features)

### Audio Output
- [ ] Implement audio output abstraction
- [ ] Add ALSA driver (Linux)
- [ ] Add PulseAudio driver (Linux)
- [ ] Add PortAudio driver (cross-platform)
- [ ] Add WASAPI driver (Windows)
- [ ] Add CoreAudio driver (macOS)

### Advanced Features
- [ ] Implement MOD-specific features
  - [ ] ProTracker 2.3D effect handling
  - [ ] FunkRepeat (EFx)
  - [ ] Invert loop (EFx + 8)
- [ ] Add XM-specific features
  - [ ] Multi retrigger with volume change
  - [ ] Tremor effect
  - [ ] Extra fine portamento
- [ ] Performance monitoring
  - [ ] CPU usage tracking
  - [ ] Mixing time measurement
  - [ ] Clip detection reporting

### Build System
- [ ] Add installation targets
- [ ] Create pkg-config file
- [ ] Add RPM/DEB packaging
- [ ] Windows installer (NSIS/WiX)
- [ ] macOS bundle

## Low Priority (Polish)

### Testing
- [ ] Unit tests for XM structures
- [ ] Unit tests for mixer
- [ ] Unit tests for effects
- [ ] Integration tests with known modules
- [ ] Regression test suite
- [ ] Benchmark suite

### Documentation
- [ ] Complete API documentation (Doxygen)
- [ ] Add code examples
- [ ] Tutorial documentation
- [ ] Effect reference guide
- [ ] File format documentation

### Examples
- [ ] Simple playback example (done)
- [ ] Audio output example
- [ ] Real-time control example
- [ ] Module info dump example
- [ ] Sample extraction example
- [ ] Pattern editor example

### Optimization
- [ ] SIMD mixer optimizations
  - [ ] SSE2 mixer (x86/x64)
  - [ ] AVX2 mixer (modern x86)
  - [ ] NEON mixer (ARM)
- [ ] Multi-threaded mixing
- [ ] Optimize envelope calculation
- [ ] Cache-friendly data layout

## Completed ✓

- [x] Core data structures (XM, patterns, instruments, samples)
- [x] Mixer interface design
- [x] Integer32 mixer implementation
- [x] Basic fixed-point arithmetic
- [x] Loop handling (forward, ping-pong)
- [x] Stereo panning
- [x] CMake build system
- [x] Simple example program
- [x] README documentation
- [x] Architecture documentation
- [x] .gitignore for build artifacts
- [x] Project structure

## Known Issues

### Critical
- None currently

### Important
- [ ] Envelope interpolation needs validation against FT2
- [ ] Ping-pong loop direction change needs testing
- [ ] Fixed-point overflow handling in mixer

### Minor
- [ ] Example uses stack-allocated sample data (unsafe for real use)
- [ ] No error handling in many functions
- [ ] Missing input validation

## Performance Targets

- **Latency**: < 10ms @ 44.1kHz (achievable)
- **CPU Usage**: < 5% for 32 channels (target)
- **Memory**: < 50MB for large modules (target)
- **Real-time**: Must not skip or stutter (critical)

## Testing Modules

Test with these well-known modules:

### XM Format
- [ ] kb-elysium.xm (classic, many effects)
- [ ] enigma.xm (portamento heavy)
- [ ] spacedeb.xm (instruments)

### MOD Format  
- [ ] mod.axel_f (classic ProTracker)
- [ ] mod.2nd_pm (complex patterns)
- [ ] mod.elysium (long patterns)

## Reference Implementation

Cross-check behavior against:
- FastTracker 2 (original)
- MilkyTracker (modern clone)
- OpenMPT (comprehensive)
- The Real SoundTracker (source)

## Community

### Contribution Areas
- Effect implementation (pick an effect!)
- File format loaders (XM, MOD, XI)
- Audio drivers
- Test cases
- Documentation
- Examples

### Getting Started
1. Pick an item from High Priority
2. Check existing code in app/ directory
3. Implement in C++17 style
4. Write tests
5. Submit PR

## Notes

### Design Decisions
- **No exceptions**: Use return codes for errors
- **Smart pointers**: Prefer std::unique_ptr and std::shared_ptr
- **Const correctness**: Mark const wherever possible
- **Thread safety**: Mutex protection for shared state

### Code Style
- Follow existing style (see .clang-format)
- Document public APIs
- Prefer composition over inheritance
- Keep functions small and focused

### Git Workflow
- Feature branches for new work
- Descriptive commit messages
- Squash before merging
- Keep history clean
