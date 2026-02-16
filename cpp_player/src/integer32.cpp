/*
 * C++ XM Module Player - Integer32 Mixer Implementation
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

#include "integer32.h"
#include <cstring>
#include <algorithm>
#include <cmath>

namespace xmplayer {

Integer32Mixer::Integer32Mixer()
    : num_channels(8)
    , stereo(true)
    , mixformat(static_cast<int>(MixFormat::S16_LE))
    , mixfreq(44100)
    , ampfactor(1.0f)
    , clipflag(false)
{
    reset();
}

void Integer32Mixer::setNumChannels(int numchannels) {
    if (numchannels >= 1 && numchannels <= 32) {
        num_channels = numchannels;
    }
}

bool Integer32Mixer::setMixFormat(int format) {
    // Only 16-bit formats supported for now
    if ((format & ~MIXER_FORMAT_STEREO_FLAG) == static_cast<int>(MixFormat::S16_LE) ||
        (format & ~MIXER_FORMAT_STEREO_FLAG) == static_cast<int>(MixFormat::S16_BE)) {
        mixformat = format;
        return true;
    }
    return false;
}

bool Integer32Mixer::setStereo(bool enabled) {
    stereo = enabled;
    return true;
}

void Integer32Mixer::setMixFrequency(uint16_t frequency) {
    mixfreq = frequency;
}

void Integer32Mixer::setAmplification(float amplification) {
    ampfactor = amplification;
}

void Integer32Mixer::updateSample(SampleInfo* si) {
    std::lock_guard<std::mutex> lock(mixer_mutex);
    
    for (int i = 0; i < 32; i++) {
        ChannelState& c = channels[i];
        if (c.sample != si || !c.active) {
            continue;
        }
        
        // Check if critical data changed - if so, stop the channel
        if (c.sample->data != si->data ||
            (c.sample->length << ACCURACY_BITS) != (si->length << ACCURACY_BITS) ||
            static_cast<int>(c.sample->looptype) != static_cast<int>(si->looptype)) {
            c.active = false;
        } else {
            // Update loop parameters
            c.sample->loopstart = si->loopstart;
            c.sample->loopend = si->loopend;
            c.sample->looptype = si->looptype;
        }
    }
}

void Integer32Mixer::reset() {
    std::lock_guard<std::mutex> lock(mixer_mutex);
    for (auto& ch : channels) {
        ch = ChannelState();
    }
}

void Integer32Mixer::startNote(int channel, SampleInfo* si) {
    if (channel < 0 || channel >= 32 || !si) return;
    
    std::lock_guard<std::mutex> lock(mixer_mutex);
    ChannelState& c = channels[channel];
    
    c.sample = si;
    c.position = 0;
    c.speed = ACCURACY;  // 1.0 in fixed-point
    c.end = (si->length << ACCURACY_BITS);
    c.direction = 1;
    c.active = true;
}

void Integer32Mixer::stopNote(int channel) {
    if (channel < 0 || channel >= 32) return;
    
    std::lock_guard<std::mutex> lock(mixer_mutex);
    channels[channel].active = false;
}

void Integer32Mixer::setSamplePosition(int channel, uint32_t offset) {
    if (channel < 0 || channel >= 32) return;
    
    std::lock_guard<std::mutex> lock(mixer_mutex);
    ChannelState& c = channels[channel];
    
    uint32_t maxpos = (c.sample ? c.sample->length : 0);
    if (offset < maxpos) {
        c.position = offset << ACCURACY_BITS;
        c.direction = 1;
    } else {
        c.active = false;
    }
}

void Integer32Mixer::setSampleEnd(int channel, uint32_t offset) {
    if (channel < 0 || channel >= 32) return;
    
    std::lock_guard<std::mutex> lock(mixer_mutex);
    ChannelState& c = channels[channel];
    
    if (c.sample && offset < c.sample->length) {
        c.end = offset << ACCURACY_BITS;
    }
}

void Integer32Mixer::setFrequency(int channel, float frequency) {
    if (channel < 0 || channel >= 32 || mixfreq == 0) return;
    
    std::lock_guard<std::mutex> lock(mixer_mutex);
    ChannelState& c = channels[channel];
    
    // Clamp frequency to prevent overflow
    if (frequency > (0x7fffffff >> ACCURACY_BITS)) {
        frequency = (0x7fffffff >> ACCURACY_BITS);
    }
    
    // Convert frequency to fixed-point speed
    c.speed = static_cast<uint32_t>(frequency * ACCURACY / mixfreq);
    if (c.speed == 0) {
        c.speed = 1;
    }
}

void Integer32Mixer::setVolume(int channel, float volume) {
    if (channel < 0 || channel >= 32) return;
    
    std::lock_guard<std::mutex> lock(mixer_mutex);
    ChannelState& c = channels[channel];
    
    // Volume is stored as 0-256 for mixing
    int32_t vol = static_cast<int32_t>(volume * 256.0f);
    c.volumeleft = vol;
    c.volumeright = vol;
}

void Integer32Mixer::setPanning(int channel, float panning) {
    if (channel < 0 || channel >= 32) return;
    
    std::lock_guard<std::mutex> lock(mixer_mutex);
    ChannelState& c = channels[channel];
    
    if (stereo) {
        // Calculate stereo volumes from panning (-1.0 to +1.0)
        // panning = -1.0: left=100%, right=0%
        // panning =  0.0: left=50%, right=50%
        // panning = +1.0: left=0%, right=100%
        int32_t base_vol = (c.volumeleft + c.volumeright) / 2;
        c.volumeleft = static_cast<int32_t>(base_vol * (1.0f - (panning + 1.0f) * 0.5f));
        c.volumeright = static_cast<int32_t>(base_vol * ((panning + 1.0f) * 0.5f));
    }
}

void Integer32Mixer::setChannelCutoff(int /* channel */, float /* freq */) {
    // Filter not implemented in basic mixer
}

void Integer32Mixer::setChannelResonance(int /* channel */, float /* reso */) {
    // Filter not implemented in basic mixer
}

void Integer32Mixer::handleLoop(ChannelState& c) {
    if (!c.sample || c.sample->looptype == LoopType::None) {
        return;
    }
    
    uint32_t loopstart = c.sample->loopstart << ACCURACY_BITS;
    uint32_t loopend = c.sample->loopend << ACCURACY_BITS;
    uint32_t looplen = loopend - loopstart;
    
    if (looplen == 0) {
        c.active = false;
        return;
    }
    
    if (c.sample->looptype == LoopType::Forward) {
        // Amiga-style forward loop
        while (c.position >= loopend) {
            c.position -= looplen;
        }
    } else if (c.sample->looptype == LoopType::PingPong) {
        // Ping-pong loop
        while (c.direction == 1 && c.position >= loopend) {
            c.position = loopend - (c.position - loopend);
            c.direction = -1;
        }
        while (c.direction == -1 && c.position < loopstart) {
            c.position = loopstart + (loopstart - c.position);
            c.direction = 1;
        }
    }
}

void* Integer32Mixer::mix(void* dest, uint32_t count, int16_t* scopebufs[], int scopebuf_offset) {
    std::lock_guard<std::mutex> lock(mixer_mutex);
    
    int16_t* output = static_cast<int16_t*>(dest);
    clipflag = false;
    
    // Temporary mix buffer
    std::vector<int32_t> mixbuf((stereo ? 2 : 1) * count, 0);
    
    // Mix all active channels
    for (int i = 0; i < num_channels; i++) {
        ChannelState& c = channels[i];
        
        if (!c.active || !c.sample) {
            // Clear scope buffer if present
            if (scopebufs && scopebufs[i]) {
                memset(scopebufs[i] + scopebuf_offset, 0, count * sizeof(int16_t));
            }
            continue;
        }
        
        // Lock sample if mutex present
        std::unique_lock<std::mutex> sample_lock;
        if (c.sample->lock) {
            sample_lock = std::unique_lock<std::mutex>(*c.sample->lock);
        }
        
        uint32_t remaining = count;
        int32_t* mixptr = mixbuf.data();
        int16_t* scopeptr = (scopebufs && scopebufs[i]) ? (scopebufs[i] + scopebuf_offset) : nullptr;
        
        while (remaining > 0 && c.active) {
            // Calculate samples we can do before hitting loop/end
            uint32_t todo;
            uint32_t end_pos;
            
            if (c.sample->looptype != LoopType::None && c.end == (c.sample->length << ACCURACY_BITS)) {
                // Looping sample
                uint32_t loopstart = c.sample->loopstart << ACCURACY_BITS;
                uint32_t loopend = c.sample->loopend << ACCURACY_BITS;
                
                if (c.direction == 1) {
                    end_pos = loopend;
                } else {
                    end_pos = loopstart;
                }
            } else {
                // Non-looping or forced end
                end_pos = c.end;
            }
            
            if (c.speed == 0) {
                c.active = false;
                break;
            }
            
            if (c.direction == 1) {
                if (c.position >= end_pos) {
                    handleLoop(c);
                    if (!c.active) break;
                }
                todo = (end_pos - c.position) / c.speed;
            } else {
                if (c.position <= end_pos) {
                    handleLoop(c);
                    if (!c.active) break;
                }
                todo = (c.position - end_pos) / c.speed;
            }
            
            if (todo == 0) {
                handleLoop(c);
                if (!c.active) break;
                continue;
            }
            
            if (todo > remaining) {
                todo = remaining;
            }
            
            // Mix samples
            for (uint32_t j = 0; j < todo; j++) {
                uint32_t pos = c.position >> ACCURACY_BITS;
                
                if (pos >= c.sample->length) {
                    c.active = false;
                    break;
                }
                
                int16_t sample_val = c.sample->data[pos];
                
                if (stereo) {
                    *mixptr++ += (sample_val * c.volumeleft) >> 8;
                    *mixptr++ += (sample_val * c.volumeright) >> 8;
                } else {
                    *mixptr++ += (sample_val * c.volumeleft) >> 8;
                }
                
                if (scopeptr) {
                    *scopeptr++ = sample_val;
                }
                
                c.position += c.speed * c.direction;
            }
            
            remaining -= todo;
            
            // Handle loop boundaries
            if (c.active) {
                handleLoop(c);
            }
        }
        
        // Fill remaining scope buffer with zeros
        if (scopeptr && remaining > 0) {
            memset(scopeptr, 0, remaining * sizeof(int16_t));
        }
    }
    
    // Convert mix buffer to output with amplification and clipping
    int amp = static_cast<int>(8.0f * ampfactor);
    for (uint32_t i = 0; i < count * (stereo ? 2 : 1); i++) {
        int32_t val = (mixbuf[i] * amp) >> 3;
        
        if (val > 32767) {
            val = 32767;
            clipflag = true;
        } else if (val < -32768) {
            val = -32768;
            clipflag = true;
        }
        
        output[i] = static_cast<int16_t>(val);
    }
    
    return output + count * (stereo ? 2 : 1);
}

void Integer32Mixer::dumpStatus(ChannelStatus status[]) const {
    std::lock_guard<std::mutex> lock(mixer_mutex);
    
    for (int i = 0; i < 32; i++) {
        const ChannelState& c = channels[i];
        status[i].current_sample = c.sample;
        status[i].current_position = c.position >> ACCURACY_BITS;
    }
}

void Integer32Mixer::loadChannelSettings(int /* channel */) {
    // Not implemented in basic mixer
}

} // namespace xmplayer
