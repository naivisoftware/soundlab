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
};


class AudioComposition {
public:
    AudioComposition(nap::Entity& root, const std::string& jsonPath);
    
    void play(int player, int partIndex);
    void play(int player, const std::string& partName);
    
    void next();
    void random();

    int getPartCount();
    
private:
    nap::Entity* entity = nullptr;
    std::vector<std::unique_ptr<AudioPlayer>> players;
    nap::JsonComponent* jsonComponent = nullptr;
    
    int mCurrentPartIndex = 0;
};


#endif /* audioentity_hpp */
