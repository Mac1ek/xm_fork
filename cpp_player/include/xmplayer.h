/*
 * C++ XM Module Player - Player Engine
 *
 * Based on The Real SoundTracker XM player
 * Copyright (C) 1994-1998 Niklas Beisert
 * Copyright (C) 1998 Tammo Hinrichs
 * Copyright (C) 1998-2001 Michael Krause
 * C++17 port and adaptation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CPP_PLAYER_XMPLAYER_H
#define CPP_PLAYER_XMPLAYER_H

#include <cstdint>
#include <memory>
#include "xm.h"
#include "mixer.h"

namespace xmplayer {

// Player state
enum class PlayMode {
    Stopped = 0,
    PlayingSong = 1,
    PlayingPattern = 2,
    PlayingNote = 3
};

// Channel structure
class Channel {
public:
    // Current state
    int32_t chPitch;           // Current pitch
    int32_t chFinalPitch;      // Final pitch (after effects)
    int chVol;                 // Volume (0-64)
    int chFinalVol;            // Final volume
    int chPan;                 // Panning (0-255)
    int chFinalPan;            // Final panning
    
    STSample* cursamp;         // Current sample
    STSample* nextsamp;        // Next sample
    STInstrument* curinstr;    // Current instrument
    
    // Envelope state
    uint32_t chVolEnvPos;
    uint32_t chPanEnvPos;
    uint16_t chFadeVol;        // Fade volume (0-0x8000)
    bool chSustain;
    
    // Effect memory
    int32_t chPortaToPitch;
    int32_t chPortaToVal;
    uint8_t chPortaUVal;
    uint8_t chPortaDVal;
    uint8_t chFinePortaUVal;
    uint8_t chFinePortaDVal;
    uint8_t chXFinePortaUVal;
    uint8_t chXFinePortaDVal;
    
    uint8_t chVolSlideUVal;
    uint8_t chVolSlideDVal;
    uint8_t chFineVolSlideUVal;
    uint8_t chFineVolSlideDVal;
    
    uint8_t chPanSlideRVal;
    uint8_t chPanSlideLVal;
    
    uint8_t chVibRate;
    uint8_t chVibPos;
    uint8_t chVibType;
    uint8_t chVibDep;
    
    uint8_t chTremRate;
    uint8_t chTremPos;
    uint8_t chTremType;
    uint8_t chTremDep;
    
    uint8_t chArpPos;
    int8_t chArpOffsets[3];
    
    int chGlissando;
    int chCutoff;
    int chResonance;
    
    uint8_t chRetrigCount;
    uint8_t chRetrigMem;
    
    int chTremorCount;
    int chTremorMem;
    int chTremorOff;
    
    uint8_t chVibSweep;
    uint8_t chVibDepth;
    
    int nextnote;
    int nown;
    
    Channel();
    void reset();
};

// XM Player class
class XMPlayer {
public:
    XMPlayer();
    ~XMPlayer();
    
    // Module management
    void setModule(std::shared_ptr<XM> module);
    std::shared_ptr<XM> getModule() const { return xm; }
    
    // Mixer management
    void setMixer(std::shared_ptr<Mixer> mixer);
    std::shared_ptr<Mixer> getMixer() const { return mixer; }
    
    // Playback control
    bool initPlaySong(int songpos, int patpos, bool initall);
    bool initPlayPattern(int pattern, int patpos, bool only1row);
    bool playNote(int channel, int note, int instrument);
    bool playNoteFull(int channel, int note, STSample* sample, 
                      uint32_t offset, uint32_t count);
    void playNoteKeyoff(int channel);
    
    double play();  // Main tick function, returns current time
    void stop();
    
    // Playback state
    int getSongPos() const { return player_songpos; }
    int getPatPos() const { return player_patpos; }
    int getTempo() const { return player_tempo; }
    int getBPM() const { return player_bpm; }
    uint8_t getCurrentTick() const { return curtick; }
    bool hasLooped() const { return player_looped; }
    
    void setSongPos(int songpos);
    void setPattern(int pattern);
    void setTempo(int tempo);
    void setBPM(int bpm);
    
private:
    std::shared_ptr<XM> xm;
    std::shared_ptr<Mixer> mixer;
    
    // Player state
    PlayMode playmode;
    int player_songpos;
    int player_patpos;
    int player_tempo;
    int player_bpm;
    uint8_t curtick;
    bool player_looped;
    double current_time;
    
    // Channels
    std::array<Channel, 32> channels;
    
    // Pattern delay state
    int patdelaytime;
    int patdelaycount;
    
    // Global volume
    int globalvol;
    int globalvolslide;
    
    // Internal functions
    void initModule();
    void playTick();
    void processRow();
    void processEffects();
    void finalChannelOps(int ch);
    
    // Helper functions
    int envelopeHandle(STEnvelope* env, uint32_t* pos, bool sustain);
    void setNotePitch(int ch, int note, int finetune, int relnote);
    float pitchToFreq(int32_t pitch);
    
    // Effect handlers (internal)
    void handleEffect(int ch, uint8_t cmd, uint8_t data);
    void handleVolumeColumn(int ch, uint8_t volcol);
    void handleNoteRetrig(int ch);
    void handleVibrato(int ch);
    void handleTremolo(int ch);
};

} // namespace xmplayer

#endif // CPP_PLAYER_XMPLAYER_H
