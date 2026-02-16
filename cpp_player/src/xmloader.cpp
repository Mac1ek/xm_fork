/*
 * C++ XM Module Player - XM File Loader
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

#include "xmloader.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cmath>

namespace xmplayer {

// Period table for MOD note conversion
static const uint16_t npertab[60] = {
    1712,1616,1524,1440,1356,1280,1208,1140,1076,1016, 960, 906,
     856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
     428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
     214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
     107, 101,  95,  90,  85,  80,  75,  71,  67,  63,  60,  56
};

// Endian conversion helpers
static inline uint32_t getLE32(const uint8_t* src) {
    return (src[0] << 0) | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);
}

static inline uint16_t getLE16(const uint8_t* src) {
    return (src[0] << 0) | (src[1] << 8);
}

static inline uint16_t getBE16(const uint8_t* src) {
    return (src[0] << 8) | (src[1] << 0);
}

static inline void putLE32(uint8_t* dest, uint32_t val) {
    dest[0] = val & 0xff;
    dest[1] = (val >> 8) & 0xff;
    dest[2] = (val >> 16) & 0xff;
    dest[3] = (val >> 24) & 0xff;
}

static inline void putLE16(uint8_t* dest, uint16_t val) {
    dest[0] = val & 0xff;
    dest[1] = (val >> 8) & 0xff;
}

static void le16ArrayToHostOrder([[maybe_unused]] int16_t* data, [[maybe_unused]] int count) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    for (int i = 0; i < count; i++) {
        uint16_t val = data[i];
        data[i] = ((val & 0xff) << 8) | ((val >> 8) & 0xff);
    }
#endif
}

// Load a single XM note from stream
static bool loadXMNote(XMNote* note, std::ifstream& f) {
    note->note = 0;
    note->instrument = 0;
    note->volume = 0;
    note->fxtype = 0;
    note->fxparam = 0;

    uint8_t c;
    if (!f.read(reinterpret_cast<char*>(&c), 1)) {
        return false;
    }

    if (c & 0x80) {
        // Compressed format
        if (c & 0x01) {
            if (!f.read(reinterpret_cast<char*>(&note->note), 1)) return false;
        }
        if (c & 0x02) {
            if (!f.read(reinterpret_cast<char*>(&note->instrument), 1)) return false;
        }
        if (c & 0x04) {
            if (!f.read(reinterpret_cast<char*>(&note->volume), 1)) return false;
        }
        if (c & 0x08) {
            if (!f.read(reinterpret_cast<char*>(&note->fxtype), 1)) return false;
        }
        if (c & 0x10) {
            if (!f.read(reinterpret_cast<char*>(&note->fxparam), 1)) return false;
        }
    } else {
        // Uncompressed format
        uint8_t d[4];
        if (!f.read(reinterpret_cast<char*>(d), 4)) {
            return false;
        }
        note->note = c;
        note->instrument = d[0];
        note->volume = d[1];
        note->fxtype = d[2];
        note->fxparam = d[3];
    }

    return true;
}

// Load XM pattern
static bool loadXMPattern(XMPattern* pat, int num_channels, std::ifstream& f) {
    uint8_t ph[9];
    if (!f.read(reinterpret_cast<char*>(ph), 9)) {
        return false;
    }

    uint16_t len = getLE16(ph + 5);
    if (len > 256) {
        return false;
    }

    uint32_t hdr_len = getLE32(ph);
    if (hdr_len > 9) {
        f.seekg(hdr_len - 9, std::ios::cur);
    }

    auto position = f.tellg();
    
    // Allocate pattern
    pat->allocate(len > 0 ? len : 1);

    uint16_t datasize = getLE16(ph + 7);
    if (datasize == 0) {
        return true;
    }

    // Read channel data
    for (int j = 0; j < len; j++) {
        for (int i = 0; i < num_channels; i++) {
            if (!loadXMNote(&pat->channels[i][j], f)) {
                return false;
            }
        }
    }

    // Seek to next pattern for error-proof positioning
    f.seekg(position + static_cast<std::streamoff>(datasize), std::ios::beg);
    return true;
}

// Check and fix envelope data
static void checkEnvelope(STEnvelope* env) {
    if (env->num_points == 0 || env->num_points > 12) {
        env->num_points = 1;
    }

    for (int i = 0; i < env->num_points; i++) {
        if (env->points[i].val > 64) {
            env->points[i].val = 32;
        }
    }

    env->points[0].pos = 0;
}

// Load XM samples for an instrument
static bool loadXMSamples(std::array<STSample, 16>& samples, int num_samples, std::ifstream& f) {
    if (num_samples > 16) {
        return false;
    }

    // Store loop types for later use
    std::array<uint8_t, 16> looptypes;

    // Read sample headers
    for (int i = 0; i < num_samples; i++) {
        uint8_t sh[40];
        if (!f.read(reinterpret_cast<char*>(sh), 40)) {
            return false;
        }

        STSample& s = samples[i];
        s.sample.length = getLE32(sh + 0);
        s.sample.loopstart = getLE32(sh + 4);
        s.sample.loopend = getLE32(sh + 8); // This is loop length, not end
        s.volume = sh[12];
        s.finetune = sh[13];
        looptypes[i] = sh[14];
        s.panning = sh[15];
        s.relnote = sh[16];
        
        // Copy sample name
        std::memcpy(&s.name[0], sh + 18, 22);
        s.name[22] = '\0';
    }

    // Read sample data
    for (int i = 0; i < num_samples; i++) {
        STSample& s = samples[i];
        
        if (s.sample.length == 0) {
            continue;
        }

        uint8_t looptype_byte = looptypes[i];
        s.treat_as_8bit = !(looptype_byte & 0x10);
        s.sample.looptype = static_cast<LoopType>(looptype_byte & 3);

        if (!s.treat_as_8bit) {
            // 16-bit sample
            s.sample.length >>= 1;
            s.sample.loopstart >>= 1;
            s.sample.loopend >>= 1;

            std::vector<int16_t> temp(s.sample.length);
            if (!f.read(reinterpret_cast<char*>(temp.data()), s.sample.length * 2)) {
                return false;
            }

            le16ArrayToHostOrder(temp.data(), s.sample.length);

            // Delta decode
            s.sample.data = new int16_t[s.sample.length];
            int16_t p = 0;
            for (uint32_t j = 0; j < s.sample.length; j++) {
                p += temp[j];
                s.sample.data[j] = p;
            }
        } else {
            // 8-bit sample
            std::vector<int8_t> temp(s.sample.length);
            if (!f.read(reinterpret_cast<char*>(temp.data()), s.sample.length)) {
                return false;
            }

            // Delta decode and convert to 16-bit
            s.sample.data = new int16_t[s.sample.length];
            int16_t p = 0;
            for (uint32_t j = 0; j < s.sample.length; j++) {
                p += static_cast<int16_t>(temp[j]) << 8;
                s.sample.data[j] = p;
            }
        }

        // Fix loop end (convert from length to end position)
        if (s.sample.loopend == 0) {
            s.sample.looptype = LoopType::None;
        } else {
            s.sample.loopend += s.sample.loopstart;
        }

        if (s.sample.looptype == LoopType::None) {
            s.sample.loopstart = 0;
            s.sample.loopend = 1;
        }

        // Initialize mutex
        s.sample.lock = new std::mutex();
    }

    return true;
}

// Load XM instrument
static bool loadXMInstrument(STInstrument* instr, std::ifstream& f) {
    uint8_t a[29];
    if (!f.read(reinterpret_cast<char*>(a), 29)) {
        return false;
    }

    uint32_t iheader_size = getLE32(a);
    
    // Copy instrument name
    std::memcpy(&instr->name[0], a + 4, 22);
    instr->name[22] = '\0';

    if (iheader_size <= 29) {
        return true;
    }

    uint16_t num_samples = getLE16(a + 27);
    if (num_samples > 16) {
        return false;
    }

    if (num_samples == 0) {
        // Skip rest of header
        f.seekg(iheader_size - 29, std::ios::cur);
        return true;
    }

    // Read sample header size
    uint8_t shsize[4];
    if (!f.read(reinterpret_cast<char*>(shsize), 4)) {
        return false;
    }
    
    if (getLE32(shsize) != 40) {
        return false;
    }

    // Read sample map
    if (!f.read(reinterpret_cast<char*>(instr->samplemap.data()), 96)) {
        return false;
    }

    // Read volume envelope
    if (!f.read(reinterpret_cast<char*>(instr->vol_env.points.data()), 48)) {
        return false;
    }
    le16ArrayToHostOrder(reinterpret_cast<int16_t*>(instr->vol_env.points.data()), 24);

    // Read pan envelope
    if (!f.read(reinterpret_cast<char*>(instr->pan_env.points.data()), 48)) {
        return false;
    }
    le16ArrayToHostOrder(reinterpret_cast<int16_t*>(instr->pan_env.points.data()), 24);

    // Read envelope and vibrato parameters
    uint8_t b[16];
    if (!f.read(reinterpret_cast<char*>(b), 16)) {
        return false;
    }

    instr->vol_env.num_points = b[0];
    instr->vol_env.sustain_point = b[2];
    instr->vol_env.loop_start = b[3];
    instr->vol_env.loop_end = b[4];
    instr->vol_env.flags = b[8];
    
    instr->pan_env.num_points = b[1];
    instr->pan_env.sustain_point = b[5];
    instr->pan_env.loop_start = b[6];
    instr->pan_env.loop_end = b[7];
    instr->pan_env.flags = b[9];

    checkEnvelope(&instr->vol_env);
    checkEnvelope(&instr->pan_env);

    instr->vibtype = b[10];
    if (instr->vibtype >= 4) {
        instr->vibtype = 0;
    }
    instr->vibrate = b[13];
    instr->vibdepth = b[12];
    instr->vibsweep = b[11];
    instr->volfade = getLE16(b + 14);

    // Skip remainder of header if needed
    if (iheader_size > 241) {
        f.seekg(iheader_size - 241, std::ios::cur);
    }

    // Load samples
    return loadXMSamples(instr->samples, num_samples, f);
}

// Load MOD note
static bool loadMODNote(XMNote* dest, std::ifstream& f) {
    uint8_t c[4];
    if (!f.read(reinterpret_cast<char*>(c), 4)) {
        return false;
    }

    int period = ((c[0] & 0x0f) << 8) | c[1];
    int note = 0;

    if (period) {
        for (note = 0; note < 60; note++) {
            if (period >= npertab[note]) {
                break;
            }
        }
        note++;
        if (note == 61) {
            note = 0;
        }
    }

    dest->note = note ? note + 24 : 0;
    dest->instrument = (c[0] & 0xf0) | (c[2] >> 4);
    dest->volume = 0;
    dest->fxtype = c[2] & 0x0f;
    dest->fxparam = c[3];

    return true;
}

// Load MOD pattern
static bool loadMODPattern(XMPattern* pat, int num_channels, std::ifstream& f) {
    int len = 64;
    
    pat->allocate(len);

    // Read channel data
    for (int j = 0; j < len; j++) {
        for (int i = 0; i < num_channels; i++) {
            if (!loadMODNote(&pat->channels[i][j], f)) {
                return false;
            }
        }
    }

    return true;
}

std::unique_ptr<XM> XMLoader::load(const std::string& filename, LoadStatus* status) {
    // Try to detect file type by opening and reading header
    std::ifstream f(filename, std::ios::binary);
    if (!f.is_open()) {
        if (status) *status = LoadStatus::FileNotFound;
        return nullptr;
    }

    uint8_t header[20];
    f.read(reinterpret_cast<char*>(header), 17);
    f.close();

    // Check if it's an XM file
    if (std::memcmp(header, "Extended Module: ", 17) == 0) {
        return loadXM(filename, status);
    } else {
        // Try MOD format
        return loadMOD(filename, status);
    }
}

std::unique_ptr<XM> XMLoader::loadXM(const std::string& filename, LoadStatus* status) {
    std::ifstream f(filename, std::ios::binary);
    if (!f.is_open()) {
        if (status) *status = LoadStatus::FileNotFound;
        return nullptr;
    }

    uint8_t xh[80];
    if (!f.read(reinterpret_cast<char*>(xh), 80)) {
        if (status) *status = LoadStatus::InvalidFormat;
        return nullptr;
    }

    // Verify XM header
    if (std::memcmp(xh, "Extended Module: ", 17) != 0 || xh[37] != 0x1a) {
        if (status) *status = LoadStatus::InvalidFormat;
        return nullptr;
    }

    // Check version
    if (getLE16(xh + 58) != 0x0104) {
        // Warning: non-standard version, but continue anyway
    }

    auto xm = std::make_unique<XM>();

    // Load song name
    std::memcpy(&xm->name[0], xh + 17, 20);
    xm->name[20] = '\0';

    xm->song_length = getLE16(xh + 64);
    xm->restart_position = getLE16(xh + 66);

    if (xm->restart_position >= xm->song_length) {
        xm->restart_position = xm->song_length - 1;
    }

    xm->num_channels = getLE16(xh + 68);
    if (xm->num_channels > 32 || xm->num_channels < 1) {
        if (status) *status = LoadStatus::InvalidFormat;
        return nullptr;
    }

    int num_patterns = getLE16(xh + 70);
    int num_instruments = getLE16(xh + 72);
    
    if (getLE16(xh + 74) != 1) {
        xm->flags |= XM_FLAGS_AMIGA_FREQ;
    }

    xm->tempo = getLE16(xh + 76);
    xm->bpm = getLE16(xh + 78);

    // Read pattern order table
    if (!f.read(reinterpret_cast<char*>(xm->pattern_order_table.data()), 256)) {
        if (status) *status = LoadStatus::CorruptedFile;
        return nullptr;
    }

    // Load patterns
    for (int i = 0; i < num_patterns; i++) {
        if (!loadXMPattern(&xm->patterns[i], xm->num_channels, f)) {
            if (status) *status = LoadStatus::CorruptedFile;
            return nullptr;
        }
    }

    // Initialize remaining patterns
    for (int i = num_patterns; i < 256; i++) {
        xm->patterns[i].allocate(64);
    }

    // Load instruments
    for (int i = 0; i < num_instruments; i++) {
        if (!loadXMInstrument(&xm->instruments[i], f)) {
            if (status) *status = LoadStatus::CorruptedFile;
            return nullptr;
        }
    }

    // Make sure channel count is even
    if (xm->num_channels & 1) {
        xm->num_channels++;
    }

    if (status) *status = LoadStatus::Success;
    return xm;
}

std::unique_ptr<XM> XMLoader::loadMOD(const std::string& filename, LoadStatus* status) {
    std::ifstream f(filename, std::ios::binary);
    if (!f.is_open()) {
        if (status) *status = LoadStatus::FileNotFound;
        return nullptr;
    }

    auto xm = std::make_unique<XM>();

    // Read module name
    char name[21];
    if (!f.read(name, 20)) {
        if (status) *status = LoadStatus::InvalidFormat;
        return nullptr;
    }
    name[20] = '\0';
    xm->name = name;

    // Read sample info for 31 instruments
    uint8_t sh[31][8];
    for (int i = 0; i < 31; i++) {
        char buf[23];
        if (!f.read(buf, 22)) {
            if (status) *status = LoadStatus::CorruptedFile;
            return nullptr;
        }
        buf[22] = '\0';
        xm->instruments[i].name = buf;
        
        if (!f.read(reinterpret_cast<char*>(sh[i]), 8)) {
            if (status) *status = LoadStatus::CorruptedFile;
            return nullptr;
        }
    }

    // Read song length
    uint8_t mh[4];
    if (!f.read(reinterpret_cast<char*>(mh), 2)) {
        if (status) *status = LoadStatus::CorruptedFile;
        return nullptr;
    }
    xm->song_length = mh[0];

    // Read pattern order table
    if (!f.read(reinterpret_cast<char*>(xm->pattern_order_table.data()), 128)) {
        if (status) *status = LoadStatus::CorruptedFile;
        return nullptr;
    }

    // Read module signature
    if (!f.read(reinterpret_cast<char*>(mh), 4)) {
        if (status) *status = LoadStatus::CorruptedFile;
        return nullptr;
    }

    // Determine number of channels based on signature
    if (!std::memcmp("M.K.", mh, 4) || !std::memcmp("M&K!", mh, 4) || !std::memcmp("M!K!", mh, 4)) {
        xm->num_channels = 4;
    } else if (!std::memcmp("FLT4", mh, 4)) {
        xm->num_channels = 4;
    } else if (!std::memcmp("CHN", mh + 1, 3)) {
        xm->num_channels = mh[0] - 0x30;
    } else if (!std::memcmp("CH", mh + 2, 2)) {
        xm->num_channels = (mh[0] - 0x30) * 10 + (mh[1] - 0x30);
    } else {
        if (status) *status = LoadStatus::InvalidFormat;
        return nullptr;
    }

    // Find highest pattern number
    int max_pattern = 0;
    for (int i = 0; i < 128; i++) {
        if (xm->pattern_order_table[i] > max_pattern) {
            max_pattern = xm->pattern_order_table[i];
        }
    }

    xm->tempo = 6;
    xm->bpm = 125;
    xm->flags = XM_FLAGS_IS_MOD | XM_FLAGS_AMIGA_FREQ;

    // Load patterns
    for (int i = 0; i <= max_pattern; i++) {
        if (!loadMODPattern(&xm->patterns[i], xm->num_channels, f)) {
            if (status) *status = LoadStatus::CorruptedFile;
            return nullptr;
        }
    }

    // Initialize remaining patterns
    for (int i = max_pattern + 1; i < 256; i++) {
        xm->patterns[i].allocate(64);
    }

    // Load sample data
    for (int i = 0; i < 31; i++) {
        STSample& s = xm->instruments[i].samples[0];

        s.sample.length = getBE16(sh[i] + 0) << 1;

        if (s.sample.length > 0) {
            s.finetune = (sh[i][2] & 0x0f) << 4;
            s.volume = sh[i][3];
            s.sample.loopstart = getBE16(sh[i] + 4) << 1;
            s.sample.loopend = s.sample.loopstart + (getBE16(sh[i] + 6) << 1);
            s.treat_as_8bit = true;
            s.panning = 128;

            if (getBE16(sh[i] + 6) > 1) {
                s.sample.looptype = LoopType::Forward;
            }
            
            if (s.sample.loopend > s.sample.length) {
                s.sample.loopend = s.sample.length;
            }
            
            if (s.sample.loopstart == s.sample.loopend) {
                s.sample.loopstart = 0;
                s.sample.loopend = 1;
                s.sample.looptype = LoopType::None;
            } else if (s.sample.loopstart > s.sample.loopend) {
                s.sample.loopstart = 0;
                s.sample.loopend = 1;
                s.sample.looptype = LoopType::None;
            }

            // Allocate and read sample data
            std::vector<int8_t> temp(s.sample.length);
            if (!f.read(reinterpret_cast<char*>(temp.data()), s.sample.length)) {
                if (status) *status = LoadStatus::CorruptedFile;
                return nullptr;
            }

            // Convert 8-bit to 16-bit
            s.sample.data = new int16_t[s.sample.length];
            for (uint32_t j = 0; j < s.sample.length; j++) {
                s.sample.data[j] = static_cast<int16_t>(temp[j]) << 8;
            }

            // Initialize mutex
            s.sample.lock = new std::mutex();
        }
    }

    if (status) *status = LoadStatus::Success;
    return xm;
}

bool XMLoader::loadXI([[maybe_unused]] STInstrument* instr, [[maybe_unused]] const std::string& filename) {
    // XI loading not implemented yet
    return false;
}

bool XMLoader::save([[maybe_unused]] XM* xm, [[maybe_unused]] const std::string& filename, [[maybe_unused]] bool song_only) {
    // XM saving not implemented yet
    return false;
}

bool XMLoader::saveXI([[maybe_unused]] STInstrument* instr, [[maybe_unused]] const std::string& filename) {
    // XI saving not implemented yet
    return false;
}

} // namespace xmplayer
