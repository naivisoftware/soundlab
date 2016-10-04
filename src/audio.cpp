//
//  audioentity.cpp
//  Soundlab
//
//  Created by Stijn van Beek on 10/1/16.
//
//

#include <audio.h>

#include "ofMain.h"

#include <nap/entity.h>
#include <Lib/Audio/Utility/AudioFile/AudioFileComponent.h>
#include <Lib/Audio/Unit/Granular/Granulator.h>
#include <Lib/Audio/Unit/Resonator/ResonatorUnit.h>
#include <Lib/Audio/Unit/OutputUnit.h>
#include <Lib/Utility/Data/Sequencer.h>
#include <Lib/Utility/Data/RampSequencer.h>

using namespace nap;
using namespace std;


AudioPlayer::AudioPlayer(nap::Entity& root, const std::string& name)
{
    entity = &root.addEntity(name);
    
    patchComponent = &entity->addComponent<PatchComponent>("patch");
    
    granulator = &patchComponent->getPatch().addOperator<lib::audio::Granulator>("granulator");
    granulator->channelCount.setValue(2);
    
    resonator = &patchComponent->getPatch().addOperator<lib::audio::ResonatorUnit>("resonator");
    resonator->channelCount.setValue(2);
    resonator->inputChannelCount.setValue(2);
    
    auto& output = patchComponent->getPatch().addOperator<lib::audio::OutputUnit>("output");
    output.channelCount.setValue(2);
    output.audioInput.connect(resonator->audioOutput);
    resonator->audioInput.connect(granulator->output);
    
    for (auto i = 0; i < 1; ++i)
    {
        auto& grainSeq = patchComponent->getPatch().addOperator<lib::Sequencer>("grainSequencer" + to_string(i + 1));
        grainSeq.schedulerInput.connect(output.schedulerOutput);
        granulator->cloudInput.connect(grainSeq.output);
        
        auto& resSeq = patchComponent->getPatch().addOperator<lib::Sequencer>("resonatorSequencer" + to_string(i + 1));
        resSeq.schedulerInput.connect(output.schedulerOutput);
        resonator->input.connect(resSeq.output);
    }
        
}


AudioComposition::AudioComposition(nap::Entity& root, const std::string& jsonPath)
{
    entity = &root.addEntity("audio");
    
    jsonComponent = &entity->addComponent<JsonComponent>("json");
    jsonComponent->jsonPath.setValue(jsonPath);
    
    if (!jsonComponent->isLoaded())
    {
        Logger::warn("Audio json not loaded: " + jsonPath);
        return;
    }
    
    // add the audio files
    std::vector<std::string> audioFileNames = jsonComponent->getStringArray("/audioFiles");
    audioFiles = &root.addEntity("audioFiles");
    for (auto& audioFileName : audioFileNames)
    {
        auto& audioFile = audioFiles->addComponent<lib::audio::AudioFileComponent>();
        audioFile.fileName.setValue(ofFile(audioFileName).getAbsolutePath());
    }
    
    // add the players
    for (auto i = 0; i < 1; ++i)
        players.emplace_back(make_unique<AudioPlayer>(*entity, "player" + to_string(i + 1)));
    
    play(0, "part1");
}


void AudioComposition::play(int player, const std::string& partName)
{
    rapidjson::Value* json = jsonComponent->getValue("/" + partName);
    if (!json)
    {
        Logger::warn("Part not found: " + partName);
        return;
    }
    
    if (player >= players.size())
    {
        Logger::warn("Player index out of range: " + to_string(player));
        return;
    }
    
    jsonComponent->mapToAttributes(*json, players[player]->patchComponent->getPatch());
}



