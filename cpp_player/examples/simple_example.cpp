/*
 * C++ XM Module Player - Simple Example
 *
 * Demonstrates basic usage of the XM player library
 */

#include <iostream>
#include <memory>
#include <cmath>
#include "xm.h"
#include "xmplayer.h"
#include "integer32.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace xmplayer;

int main(int /* argc */, char** /* argv */) {
    std::cout << "C++ XM Module Player - Simple Example" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;
    
    // Create mixer
    auto mixer = std::make_shared<Integer32Mixer>();
    std::cout << "Mixer: " << mixer->getDescription() << std::endl;
    
    // Configure mixer
    mixer->setNumChannels(8);
    mixer->setMixFrequency(44100);
    mixer->setStereo(true);
    mixer->setAmplification(1.0f);
    mixer->setMixFormat(static_cast<int>(MixFormat::S16_LE) | MIXER_FORMAT_STEREO_FLAG);
    
    // Create player
    auto player = std::make_unique<XMPlayer>();
    player->setMixer(mixer);
    
    std::cout << "Player created successfully!" << std::endl;
    std::cout << std::endl;
    
    // Create a simple test module
    auto xm = XM::create();
    xm->name = "Test Module";
    xm->num_channels = 8;
    xm->tempo = 6;
    xm->bpm = 125;
    xm->song_length = 1;
    xm->restart_position = 0;
    
    // Allocate one pattern
    xm->patterns[0].allocate(64);
    
    // Add some test notes (C-4, E-4, G-4 - C major chord)
    auto* note1 = xm->patterns[0].getNote(0, 0);
    if (note1) {
        note1->note = 48;  // C-4
        note1->instrument = 1;
        note1->volume = 0;
        note1->fxtype = 0;
        note1->fxparam = 0;
    }
    
    auto* note2 = xm->patterns[0].getNote(1, 0);
    if (note2) {
        note2->note = 52;  // E-4
        note2->instrument = 1;
    }
    
    auto* note3 = xm->patterns[0].getNote(2, 0);
    if (note3) {
        note3->note = 55;  // G-4
        note3->instrument = 1;
    }
    
    // Set up a simple instrument with a sine wave sample
    STInstrument& inst = xm->instruments[0];
    inst.name = "Sine Wave";
    
    // Create a simple sine wave sample
    constexpr int sample_length = 256;
    std::vector<int16_t> sine_data(sample_length);
    for (int i = 0; i < sample_length; i++) {
        sine_data[i] = static_cast<int16_t>(32767.0 * sin(2.0 * M_PI * i / sample_length));
    }
    
    STSample& sample = inst.samples[0];
    sample.name = "Sine";
    sample.sample.length = sample_length;
    sample.sample.looptype = LoopType::Forward;
    sample.sample.loopstart = 0;
    sample.sample.loopend = sample_length;
    sample.sample.data = sine_data.data();
    sample.volume = 64;
    sample.panning = 128;
    sample.relnote = 0;
    sample.finetune = 0;
    
    // Set instrument to use this sample for all notes
    for (int i = 0; i < 96; i++) {
        inst.samplemap[i] = 0;
    }
    
    // Load module into player (convert unique_ptr to shared_ptr)
    auto xm_shared = std::shared_ptr<XM>(std::move(xm));
    player->setModule(xm_shared);
    
    std::cout << "Module loaded:" << std::endl;
    std::cout << "  Name: " << xm_shared->name << std::endl;
    std::cout << "  Channels: " << xm_shared->num_channels << std::endl;
    std::cout << "  Tempo: " << xm_shared->tempo << std::endl;
    std::cout << "  BPM: " << xm_shared->bpm << std::endl;
    std::cout << std::endl;
    
    std::cout << "Note: This is a basic example demonstrating the library structure." << std::endl;
    std::cout << "Full XM playback requires implementation of:" << std::endl;
    std::cout << "  - XM file loader" << std::endl;
    std::cout << "  - Complete player engine with all effects" << std::endl;
    std::cout << "  - Audio output driver" << std::endl;
    std::cout << std::endl;
    
    return 0;
}
