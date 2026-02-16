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

#ifndef CPP_PLAYER_XMLOADER_H
#define CPP_PLAYER_XMLOADER_H

#include <string>
#include <memory>
#include "xm.h"

namespace xmplayer {

// Load status codes
enum class LoadStatus {
    Success = 0,
    FileNotFound = 1,
    InvalidFormat = 2,
    CorruptedFile = 3,
    UnsupportedVersion = 4,
    OutOfMemory = 5,
    UnknownError = 6
};

// XM Module Loader
class XMLoader {
public:
    // Load XM or MOD file
    static std::unique_ptr<XM> load(const std::string& filename, LoadStatus* status = nullptr);
    
    // Load XM file specifically
    static std::unique_ptr<XM> loadXM(const std::string& filename, LoadStatus* status = nullptr);
    
    // Load MOD file specifically
    static std::unique_ptr<XM> loadMOD(const std::string& filename, LoadStatus* status = nullptr);
    
    // Load XI instrument file
    static bool loadXI(STInstrument* instr, const std::string& filename);
    
    // Save XM file
    static bool save(XM* xm, const std::string& filename, bool song_only = false);
    
    // Save XI instrument file
    static bool saveXI(STInstrument* instr, const std::string& filename);
    
private:
    XMLoader() = default;
};

} // namespace xmplayer

#endif // CPP_PLAYER_XMLOADER_H
