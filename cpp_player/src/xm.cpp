/*
 * C++ XM Module Player - XM Module Implementation
 *
 * Based on The Real SoundTracker
 * Copyright (C) 1998-2001 Michael Krause
 * C++17 port and adaptation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "xm.h"
#include <cmath>

namespace xmplayer {

void freqNoteToRelnoteFinetune(float frequency, unsigned note, 
                                int8_t* relnote, int8_t* finetune) {
    // Calculate relative note and finetune from frequency
    // Based on the formula: freq = 8363 * 2^((note-48)/12)
    
    if (frequency <= 0.0f) {
        *relnote = 0;
        *finetune = 0;
        return;
    }
    
    // Calculate the note offset from C-4 (note 48)
    float noteOffset = 12.0f * log2f(frequency / 8363.0f);
    float targetNote = 48.0f + noteOffset;
    
    // Current note value
    float currentNote = static_cast<float>(note);
    
    // Calculate relative note (in semitones)
    float relnotef = targetNote - currentNote;
    
    // Split into whole semitones and fractional part
    *relnote = static_cast<int8_t>(roundf(relnotef));
    
    // Finetune is the fractional part scaled to -128..+127
    float fractional = relnotef - *relnote;
    *finetune = static_cast<int8_t>(fractional * 128.0f);
}

} // namespace xmplayer
