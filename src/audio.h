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
#include <jsonchooser.h>

#include <Utils/nofattributewrapper.h>

#include "ofxGui.h"

class AudioPlayer {
public:
    AudioPlayer(nap::Entity& root, const std::string& name, nap::JsonComponent& jsonComponent);
    
    nap::Entity* entity = nullptr;
    nap::PatchComponent* patchComponent = nullptr;
    lib::audio::ResonatorUnit* resonator = nullptr;
    lib::audio::Granulator* granulator = nullptr;
    std::vector<lib::Sequencer*> grainSequencers;
    std::vector<lib::Sequencer*> resonatorSequencers;
    std::vector<nap::JsonChooser*> grainSequenceChoosers;
    std::vector<nap::JsonChooser*> resonatorSequenceChoosers;
    
    OFAttributeWrapper grainParameters;
    OFAttributeWrapper resonParameters;
    OFAttributeWrapper positionParameters;
    OFAttributeWrapper densityParameters;
    ofxPanel granulatorPanel;
    ofxPanel resonatorPanel;
};


class AudioComposition {
public:
    AudioComposition(nap::Entity& root, const std::string& jsonPath);
    
    void play(int player, int partIndex);
    void play(int player, const std::string& partName);
    
    ofxPanel& getGranulatorGuiForPlayer(int index) { return players[index]->granulatorPanel; }
    ofxPanel& getResonatorGuiForPlayer(int index) { return players[index]->resonatorPanel; }
    
    int getPlayerCount() { return players.size(); }
    
private:
    nap::Entity* entity = nullptr;
    nap::JsonComponent* jsonComponent = nullptr;
    std::vector<std::unique_ptr<AudioPlayer>> players;
};


#endif /* audioentity_hpp */
