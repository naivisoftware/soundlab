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
#include <Lib/Audio/Unit/OutputUnit.h>
#include <Lib/Utility/Data/Sequencer.h>
#include <Lib/Utility/Data/RampSequencer.h>

#include <4dService/Transform.h>

#include <jsoncomponent.h>
#include <jsonchooser.h>

#include <Utils/nofattributewrapper.h>

#include "ofxGui.h"

class AudioPlayer {
public:
    AudioPlayer(nap::Entity& root, const std::string& name, nap::JsonComponent& jsonComponent);
    
    void createModulator(lib::ValueControl& control, OFAttributeWrapper& parameters);
    void setupGui(ofxPanel& panel);
    
    nap::Entity* entity = nullptr;
    spatial::Transform* transform;
    nap::PatchComponent* patchComponent = nullptr;
    lib::audio::ResonatorUnit* resonator = nullptr;
    lib::audio::Granulator* granulator = nullptr;
    lib::audio::OutputUnit* output = nullptr;
    std::vector<lib::Sequencer*> grainSequencers;
    std::vector<lib::Sequencer*> resonatorSequencers;
    std::vector<nap::JsonChooser*> grainSequenceChoosers;
    std::vector<nap::JsonChooser*> resonatorSequenceChoosers;
    nap::JsonComponent& jsonComponent;
    
    OFAttributeWrapper grainParameters;
    OFAttributeWrapper resonParameters;
    OFAttributeWrapper positionParameters;
    OFAttributeWrapper densityParameters;
    OFAttributeWrapper globalParameters;
    
    std::vector<float> tonalities = { 3/4.f, 8/10.f, 7/8.f, 1, 9.f/8.f, 6.f/5.f, 4.f/3.f };
};


class AudioComposition {
public:
    AudioComposition(nap::Entity& root, const std::string& jsonPath);
    
    void play(int player, int partIndex);
    void play(int player, const std::string& partName);
    
    void setupGuiForPlayer(ofxPanel& panel, int player) { players[player]->setupGui(panel); }
    nap::Signal<lib::TimeValue, const lib::audio::GrainParameters&>& getGrainSignalForPlayer(int player) { return players[player]->granulator->grainSignal; }
    int getPlayerCount() { return players.size(); }
    
private:
    nap::Entity* entity = nullptr;
    nap::JsonComponent* jsonComponent = nullptr;
    std::vector<std::unique_ptr<AudioPlayer>> players;
};


#endif /* audioentity_hpp */
