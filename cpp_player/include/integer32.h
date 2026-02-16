/*
 * C++ XM Module Player - Integer32 Mixer
 *
 * Based on The Real SoundTracker integer32 mixer
 * Copyright (C) 1998-2001 Michael Krause
 * C++17 port and adaptation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CPP_PLAYER_INTEGER32_H
#define CPP_PLAYER_INTEGER32_H

#include <array>
#include <mutex>
#include "mixer.h"

namespace xmplayer {

// Fixed-point accuracy (12 bits)
constexpr int ACCURACY_BITS = 12;
constexpr int ACCURACY = (1 << ACCURACY_BITS);

// Integer32 mixer - 32 channel stereo mixer with fixed-point arithmetic
class Integer32Mixer : public Mixer {
public:
    Integer32Mixer();
    virtual ~Integer32Mixer() = default;
    
    // Mixer identification
    const char* getId() const override { return "integer32"; }
    const char* getDescription() const override { 
        return "32-channel stereo mixer (fixed-point)"; 
    }
    
    // Configuration
    void setNumChannels(int numchannels) override;
    bool setMixFormat(int format) override;
    bool setStereo(bool enabled) override;
    void setMixFrequency(uint16_t frequency) override;
    void setAmplification(float amplification) override;
    
    // Sample management
    void updateSample(SampleInfo* si) override;
    
    // Playback control
    void reset() override;
    void startNote(int channel, SampleInfo* si) override;
    void stopNote(int channel) override;
    
    // Channel parameters
    void setSamplePosition(int channel, uint32_t offset) override;
    void setSampleEnd(int channel, uint32_t offset) override;
    void setFrequency(int channel, float frequency) override;
    void setVolume(int channel, float volume) override;
    void setPanning(int channel, float panning) override;
    void setChannelCutoff(int channel, float freq) override;
    void setChannelResonance(int channel, float reso) override;
    
    // Mixing
    void* mix(void* dest, uint32_t count, int16_t* scopebufs[], int scopebuf_offset) override;
    
    // Status
    bool getClipFlag() const override { return clipflag; }
    void dumpStatus(ChannelStatus status[]) const override;
    void loadChannelSettings(int channel) override;
    
    // Limits
    uint32_t getMaxSampleLength() const override { return 0x7FFFFFFF; }
    
private:
    struct ChannelState {
        SampleInfo* sample;
        uint32_t position;        // Fixed-point position
        uint32_t speed;           // Fixed-point speed
        uint32_t end;             // Sample end position
        int32_t volumeleft;       // Volume (0-256)
        int32_t volumeright;      // Volume (0-256)
        int direction;            // 1 = forward, -1 = backward (ping-pong)
        bool active;
        
        ChannelState() 
            : sample(nullptr)
            , position(0)
            , speed(0)
            , end(0)
            , volumeleft(0)
            , volumeright(0)
            , direction(1)
            , active(false) {}
    };
    
    std::array<ChannelState, 32> channels;
    int num_channels;
    bool stereo;
    int mixformat;
    uint16_t mixfreq;
    float ampfactor;
    bool clipflag;
    
    mutable std::mutex mixer_mutex;
    
    // Internal mixing functions
    void mixChannel(int ch, int16_t* destleft, int16_t* destright, uint32_t count);
    void handleLoop(ChannelState& ch);
};

} // namespace xmplayer

#endif // CPP_PLAYER_INTEGER32_H
