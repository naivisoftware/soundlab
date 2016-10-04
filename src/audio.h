//
//  audioentity.hpp
//  Soundlab
//
//  Created by Stijn van Beek on 10/1/16.
//
//

#ifndef audioentity_hpp
#define audioentity_hpp

#include <stdio.h>

#include <nap/entity.h>
#include <nap/patchcomponent.h>

#include <Lib/Audio/Unit/Granular/Granulator.h>
#include <Lib/Audio/Unit/Resonator/ResonatorUnit.h>
#include <Lib/Utility/Data/Sequencer.h>
#include <Lib/Utility/Data/RampSequencer.h>
#include <jsoncomponent.h>


class AudioPlayer {
public:
    AudioPlayer(nap::Entity& root, const std::string& name);
    
    nap::Entity* entity = nullptr;
    nap::PatchComponent* patchComponent = nullptr;
    lib::audio::Granulator* granulator = nullptr;
    lib::audio::ResonatorUnit* resonator = nullptr;
    std::vector<lib::Sequencer*> grainSequencers;
    std::vector<lib::RampSequencer*> grainAnimators;
    std::vector<lib::Sequencer*> resonatorSequencers;
    std::vector<lib::RampSequencer*> resonatorAnimators;
};


class AudioComposition {
public:
    AudioComposition(nap::Entity& root, const std::string& jsonPath);
    
    void play(int player, const std::string& partName);
    
    nap::Entity* entity = nullptr;
    std::vector<std::unique_ptr<AudioPlayer>> players;
    
    nap::JsonComponent* jsonComponent = nullptr;
    
    nap::Entity* audioFiles = nullptr;
};


#endif /* audioentity_hpp */
