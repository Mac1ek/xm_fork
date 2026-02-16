/*
 * C++ XM Module Player - Mixer Interface
 *
 * Based on The Real SoundTracker mixer module
 * Copyright (C) 1998-2001 Michael Krause
 * C++17 port and adaptation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CPP_PLAYER_MIXER_H
#define CPP_PLAYER_MIXER_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

namespace xmplayer {

// Loop types for sample playback
enum class LoopType : uint32_t {
    None = 0,
    Forward = 1,      // Amiga-style forward loop
    PingPong = 2      // Bidirectional loop
};

// Sample information structure
struct SampleInfo {
    LoopType looptype;
    uint32_t length;       // Length in samples, not bytes
    uint32_t loopstart;    // Loop start offset in samples
    uint32_t loopend;      // Loop end offset (first sample not played)
    int16_t* data;         // Pointer to sample data
    std::mutex* lock;      // Thread-safety lock
    
    SampleInfo() 
        : looptype(LoopType::None)
        , length(0)
        , loopstart(0)
        , loopend(0)
        , data(nullptr)
        , lock(nullptr) {}
};

// Channel status information
struct ChannelStatus {
    SampleInfo* current_sample;
    uint32_t current_position;
    
    ChannelStatus() 
        : current_sample(nullptr)
        , current_position(0) {}
};

// Mixer output formats
enum class MixFormat {
    S16_LE = 1,   // Signed 16-bit little-endian
    S16_BE = 2,   // Signed 16-bit big-endian
    S8 = 3,       // Signed 8-bit
    U16_LE = 4,   // Unsigned 16-bit little-endian
    U16_BE = 5,   // Unsigned 16-bit big-endian
    U8 = 6,       // Unsigned 8-bit
};

constexpr int MIXER_FORMAT_STEREO_FLAG = 16;

// Abstract mixer interface
class Mixer {
public:
    virtual ~Mixer() = default;
    
    // Mixer identification
    virtual const char* getId() const = 0;
    virtual const char* getDescription() const = 0;
    
    // Configuration
    virtual void setNumChannels(int numchannels) = 0;
    virtual bool setMixFormat(int format) = 0;
    virtual bool setStereo(bool enabled) = 0;
    virtual void setMixFrequency(uint16_t frequency) = 0;
    virtual void setAmplification(float amplification) = 0;
    
    // Sample management
    virtual void updateSample(SampleInfo* si) = 0;
    
    // Playback control
    virtual void reset() = 0;
    virtual void startNote(int channel, SampleInfo* si) = 0;
    virtual void stopNote(int channel) = 0;
    
    // Channel parameters
    virtual void setSamplePosition(int channel, uint32_t offset) = 0;
    virtual void setSampleEnd(int channel, uint32_t offset) = 0;
    virtual void setFrequency(int channel, float frequency) = 0;
    virtual void setVolume(int channel, float volume) = 0;
    virtual void setPanning(int channel, float panning) = 0;
    virtual void setChannelCutoff(int channel, float freq) = 0;
    virtual void setChannelResonance(int channel, float reso) = 0;
    
    // Mixing
    virtual void* mix(void* dest, uint32_t count, int16_t* scopebufs[], int scopebuf_offset) = 0;
    
    // Status
    virtual bool getClipFlag() const = 0;
    virtual void dumpStatus(ChannelStatus status[]) const = 0;
    virtual void loadChannelSettings(int channel) = 0;
    
    // Limits
    virtual uint32_t getMaxSampleLength() const = 0;
};

} // namespace xmplayer

#endif // CPP_PLAYER_MIXER_H
