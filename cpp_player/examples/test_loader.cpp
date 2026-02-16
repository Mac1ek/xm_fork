/*
 * Simple test for XM loader
 */

#include "xmloader.h"
#include <iostream>
#include <memory>

using namespace xmplayer;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <xm_or_mod_file>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    LoadStatus status;

    std::cout << "Loading file: " << filename << std::endl;
    
    auto xm = XMLoader::load(filename, &status);
    
    if (!xm) {
        std::cout << "Failed to load file. Status: " << static_cast<int>(status) << std::endl;
        return 1;
    }

    std::cout << "Successfully loaded XM module!" << std::endl;
    std::cout << "  Name: " << xm->name << std::endl;
    std::cout << "  Channels: " << xm->num_channels << std::endl;
    std::cout << "  Song length: " << xm->song_length << std::endl;
    std::cout << "  Tempo: " << xm->tempo << std::endl;
    std::cout << "  BPM: " << xm->bpm << std::endl;
    std::cout << "  Flags: 0x" << std::hex << xm->flags << std::dec << std::endl;

    // Count patterns with data
    int pattern_count = 0;
    for (int i = 0; i < 256; i++) {
        if (xm->patterns[i].length > 0) {
            pattern_count++;
        }
    }
    std::cout << "  Patterns: " << pattern_count << std::endl;

    // Count instruments with samples
    int instr_count = 0;
    for (int i = 0; i < 128; i++) {
        bool has_samples = false;
        for (int j = 0; j < 16; j++) {
            if (xm->instruments[i].samples[j].sample.length > 0) {
                has_samples = true;
                break;
            }
        }
        if (has_samples) {
            instr_count++;
        }
    }
    std::cout << "  Instruments: " << instr_count << std::endl;

    return 0;
}
