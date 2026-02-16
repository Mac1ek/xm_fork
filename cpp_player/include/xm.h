/*
 * C++ XM Module Player - XM Module Structures
 *
 * Based on The Real SoundTracker XM support routines
 * Copyright (C) 1998-2001 Michael Krause
 * C++17 port and adaptation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CPP_PLAYER_XM_H
#define CPP_PLAYER_XM_H

#include <cstdint>
#include <string>
#include <array>
#include <memory>
#include "mixer.h"

namespace xmplayer {

// Pattern note definitions
constexpr int XM_PATTERN_NOTE_MIN = 0;
constexpr int XM_PATTERN_NOTE_MAX = 95;
constexpr int XM_PATTERN_NOTE_OFF = 97;

// Volume column definitions
constexpr int XM_NOTE_VOLUME_MIN = 0x10;
constexpr int XM_NOTE_VOLUME_MAX = 0x50;

// Pitch definitions
constexpr int PITCH_NOTE = (16 * 4 * 4);
constexpr int PITCH_OCTAVE = (12 * PITCH_NOTE);

// XM pattern note (5 bytes)
struct XMNote {
    uint8_t note;
    uint8_t instrument;
    uint8_t volume;
    uint8_t fxtype;
    uint8_t fxparam;
    
    XMNote() 
        : note(0), instrument(0), volume(0), fxtype(0), fxparam(0) {}
};

// XM pattern structure
struct XMPattern {
    int length;
    int alloc_length;
    std::array<std::unique_ptr<XMNote[]>, 32> channels;
    
    XMPattern() : length(0), alloc_length(0) {}
    
    // Allocate pattern memory
    void allocate(int len) {
        length = len;
        alloc_length = len;
        for (int i = 0; i < 32; i++) {
            channels[i] = std::make_unique<XMNote[]>(len);
        }
    }
    
    // Get note at position
    XMNote* getNote(int channel, int row) {
        if (channel < 0 || channel >= 32 || row < 0 || row >= length) {
            return nullptr;
        }
        return &channels[channel][row];
    }
};

// Sample structure
class STSample {
public:
    SampleInfo sample;
    
    std::string name;      // Max 22 chars
    uint8_t volume;        // Default volume (0..64)
    int8_t finetune;       // Finetune (-128 ... 127)
    uint8_t panning;       // Panning (0 ... 255)
    int8_t relnote;        // Relative note
    bool treat_as_8bit;
    
    STSample() 
        : volume(64)
        , finetune(0)
        , panning(128)
        , relnote(0)
        , treat_as_8bit(false) {}
};

// Envelope flags
constexpr int EF_ON = 1;
constexpr int EF_SUSTAIN = 2;
constexpr int EF_LOOP = 4;

constexpr int ST_MAX_ENVELOPE_POINTS = 12;

// Envelope point
struct STEnvelopePoint {
    uint16_t pos;
    uint16_t val;
    
    STEnvelopePoint() : pos(0), val(0) {}
};

// Envelope structure
class STEnvelope {
public:
    std::array<STEnvelopePoint, ST_MAX_ENVELOPE_POINTS> points;
    uint8_t num_points;
    uint8_t sustain_point;
    uint8_t loop_start;
    uint8_t loop_end;
    uint8_t flags;
    
    STEnvelope() 
        : num_points(0)
        , sustain_point(0)
        , loop_start(0)
        , loop_end(0)
        , flags(0) {}
    
    // Get envelope length
    int getLength() const {
        if (num_points == 0) return 0;
        return points[num_points - 1].pos;
    }
};

// Instrument structure
class STInstrument {
public:
    std::string name;      // Max 22 chars
    
    STEnvelope vol_env;
    STEnvelope pan_env;
    
    uint8_t vibtype;
    uint16_t vibrate;
    uint16_t vibdepth;
    uint16_t vibsweep;
    
    uint16_t volfade;
    
    std::array<int8_t, 96> samplemap;
    std::array<STSample, 16> samples;
    
    STInstrument() 
        : vibtype(0)
        , vibrate(0)
        , vibdepth(0)
        , vibsweep(0)
        , volfade(0) {
        // Initialize samplemap
        for (int i = 0; i < 96; i++) {
            samplemap[i] = 0;
        }
    }
};

// XM module flags
constexpr int XM_FLAGS_AMIGA_FREQ = 1;
constexpr int XM_FLAGS_IS_MOD = 2;

// Main XM module structure
class XM {
public:
    std::string name;      // Max 20 chars
    bool modified;
    
    int flags;
    int num_channels;
    int tempo;
    int bpm;
    
    int song_length;
    int restart_position;
    std::array<uint8_t, 256> pattern_order_table;
    
    std::array<XMPattern, 256> patterns;
    std::array<STInstrument, 128> instruments;
    
    XM() 
        : modified(false)
        , flags(0)
        , num_channels(8)
        , tempo(6)
        , bpm(125)
        , song_length(1)
        , restart_position(0) {
        // Initialize pattern order table
        for (int i = 0; i < 256; i++) {
            pattern_order_table[i] = 0;
        }
    }
    
    // Allocate a new empty module
    static std::unique_ptr<XM> create() {
        return std::make_unique<XM>();
    }
};

// Utility function to convert frequency to relnote/finetune
void freqNoteToRelnoteFinetune(float frequency, unsigned note, 
                                int8_t* relnote, int8_t* finetune);

} // namespace xmplayer

#endif // CPP_PLAYER_XM_H
