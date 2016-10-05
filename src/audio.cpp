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
#include <Lib/Utility/Functions/MathFunctions.h>

using namespace nap;
using namespace std;


AudioPlayer::AudioPlayer(nap::Entity& root, const std::string& name)
{
    entity = &root.addEntity(name);
    
    patchComponent = &entity->addComponent<PatchComponent>("patch");
    
    // granulator
    auto& granulator = patchComponent->getPatch().addOperator<lib::audio::Granulator>("granulator");
    granulator.channelCount.setValue(2);
    
    // resonator
    auto& resonator = patchComponent->getPatch().addOperator<lib::audio::ResonatorUnit>("resonator");
    resonator.channelCount.setValue(2);
    resonator.inputChannelCount.setValue(2);
    
    // audio output
    auto& output = patchComponent->getPatch().addOperator<lib::audio::OutputUnit>("output");
    output.channelCount.setValue(2);
    output.audioInput.connect(granulator.output);
    output.audioInput.connect(resonator.audioOutput);
    resonator.audioInput.connect(granulator.output);
    
    // sequencers
    for (auto i = 0; i < 2; ++i)
    {
        auto& grainSeq = patchComponent->getPatch().addOperator<lib::Sequencer>("grainSequencer" + to_string(i + 1));
        grainSeq.schedulerInput.connect(output.schedulerOutput);
        granulator.cloudInput.connect(grainSeq.output);
        
        auto& resSeq = patchComponent->getPatch().addOperator<lib::Sequencer>("resonatorSequencer" + to_string(i + 1));
        resSeq.schedulerInput.connect(output.schedulerOutput);
        resonator.input.connect(resSeq.output);
    }
    
    // granulator animators
    auto& densityAnimator = patchComponent->getPatch().addOperator<lib::RampSequencer>("densityAnimator");
    densityAnimator.schedulerInput.connect(output.schedulerOutput);
    granulator.density.proportionPlug.connect(densityAnimator.output);
    
    auto& positionAnimator = patchComponent->getPatch().addOperator<lib::RampSequencer>("positionAnimator");
    positionAnimator.schedulerInput.connect(output.schedulerOutput);
    granulator.position.proportionPlug.connect(positionAnimator.output);
    
    auto& grainSizeAnimator = patchComponent->getPatch().addOperator<lib::RampSequencer>("grainSizeAnimator");
    grainSizeAnimator.schedulerInput.connect(output.schedulerOutput);
    granulator.position.proportionPlug.connect(grainSizeAnimator.output);
    
    // resonator animator
    auto& resonatorAnimator = patchComponent->getPatch().addOperator<lib::RampSequencer>("resonanceAnimator");
    resonatorAnimator.schedulerInput.connect(output.schedulerOutput);
    resonator.feedback.proportionPlug.connect(resonatorAnimator.output);
        
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
    auto& audioFiles = root.addEntity("audioFiles");
    for (auto& audioFileName : audioFileNames)
    {
        auto& audioFile = audioFiles.addComponent<lib::audio::AudioFileComponent>();
        audioFile.fileName.setValue(ofFile(audioFileName).getAbsolutePath());
    }
    
    // add the player
    for (auto i = 0; i < 2; ++i)
        players.emplace_back(make_unique<AudioPlayer>(*entity, "player" + to_string(i + 1)));
    
    play(0, "init/1");
    play(1, "init/2");
    mCurrentPartIndex = jsonComponent->getNumber<int>("/start", 0);
    play(0, mCurrentPartIndex);
    play(1, mCurrentPartIndex);
}


void AudioComposition::play(int player, int index)
{
    rapidjson::Value* parts = jsonComponent->getValue("/parts");
    int i = 0;
    auto it = parts->MemberBegin();
    while (i != index && it != parts->MemberEnd())
    {
        it++;
        i++;
    }
    
    auto name = it->name.GetString();
    rapidjson::Value* json = &it->value;
    if (!json)
    {
        Logger::warn("Part not found: " + to_string(index));
        return;
    }
    
    if (player >= players.size())
    {
        Logger::warn("Player index out of range: " + to_string(player));
        return;
    }
    
    Logger::debug(std::string("Playing audio part: ") + name + " on " + to_string(player));
    jsonComponent->mapToAttributes(*json, players[player]->patchComponent->getPatch());
    
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
    
    Logger::debug("Playing audio part: " + partName + " on " + to_string(player));
    jsonComponent->mapToAttributes(*json, players[player]->patchComponent->getPatch());
}


int AudioComposition::getPartCount()
{
    rapidjson::Value* json = jsonComponent->getValue("/parts");
    
    if (!json || !json->IsObject())
    {
        Logger::warn("No parts found");
        return;
    }
    
    return json->MemberCount();
}


void AudioComposition::next()
{
    mCurrentPartIndex = (mCurrentPartIndex + 1) % getPartCount();
    play(lib::dblRand() * players.size(), mCurrentPartIndex);
}


void AudioComposition::random()
{
    mCurrentPartIndex = lib::dblRand() * getPartCount();
    play(lib::dblRand() * players.size(), mCurrentPartIndex);
}



