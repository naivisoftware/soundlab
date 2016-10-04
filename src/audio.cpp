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
    output.audioInput.connect(granulator->output);
//    resonator->audioInput.connect(granulator->output);
    
    for (auto i = 0; i < 2; ++i)
    {
        auto& grainSeq = patchComponent->getPatch().addOperator<lib::Sequencer>("grainSequencer" + to_string(i + 1));
        grainSeq.playing.setValue(true);
        grainSeq.schedulerInput.connect(output.schedulerOutput);
        granulator->cloudInput.connect(grainSeq.output);
        
        auto& resSeq = patchComponent->getPatch().addOperator<lib::Sequencer>("resonatorSequencer" + to_string(i + 1));
        resSeq.playing.setValue(true);
        resSeq.schedulerInput.connect(output.schedulerOutput);
        resonator->input.connect(resSeq.output);
    }
    
    
    granulator->density.setProportion(0.4);
    granulator->duration.setValue(100);
    granulator->irregularity.setProportion(0.5);
    granulator->positionDev.setProportion(0.25);
    granulator->positionSpeed.setProportion(0.4);
    granulator->amplitude.setProportion(1.);
    granulator->selectedStream.setValue(5);
   granulator->playCloudParams({ { "pitch", 1. }, { "position", 0.5 }, { "amplitude", 1 }, { "attack", 1000 }, { "decay", 1000 }});
    
    
    
// 	auto& animator = patchComponent.getPatch().addOperator<RampSequencer>("animator");
// 	animator.sequence.setValue({ 0.2f, 0.5f, 0.1f, 0.6f });
// 	animator.durations.setValue({ 3000, 500, 6000 });
// 	animator.looping.setValue(true);
// 	animator.schedulerInput.connect(output.schedulerOutput);
// 	granulator->density.proportionPlug.connect(animator.output);

//   granulator->playCloudParams({ { "pitch", 1. }, { "position", 0.5 }, { "amplitude", 1 }, { "attack", 1000 }, { "decay", 1000 }});
    
//    FloatArray pitches1 = { 1.0f, 2/3.f, 1/2.f, 1/3.f, 1/6.f, 5/6.f, 7/6.f };
//    FloatArray pitches2 = mult({ 2/3.f, 1/2.f, 1/3.f, 1/6.f, 5/6.f, 1.f }, 1.5f);
//    
//    patchComponent->getPatch().addOperator<Sequencer>("animator");
//    animator1.sequences.addAttribute<FloatArray>("pitch", pitches1);
//    animator1.sequences.addAttribute<FloatArray>("position", { 0.2f } );
//    animator1.sequences.addAttribute<float>("attack", 500);
//    animator1.sequences.addAttribute<float>("decay", 500);
//    animator1.sequences.addAttribute<float>("duration", 2000);
//    animator1.times.setValue({ 2000, 1000, 1000, 2000, 1000 });
//    animator1.looping.setValue(true);
//    animator1.schedulerInput.connect(output.schedulerOutput);
//    granulator->cloudInput.connect(animator1.output);
    
//    patchComponent->getPatch().addOperator<Sequencer>("animator2");
//    animator2.sequences.addAttribute<FloatArray>("pitch", pitches2);
//    animator2.sequences.addAttribute<FloatArray>("position", { 0.2f } );
//    animator2.sequences.addAttribute<float>("attack", 500);
//    animator2.sequences.addAttribute<float>("decay", 500);
//    animator2.sequences.addAttribute<float>("duration", 3000);
//    animator2.times.setValue({ 3000, 1500, 3000, 1500, 3000 });
//    animator2.looping.setValue(true);
//    animator2.schedulerInput.connect(output.schedulerOutput);
//    granulator->cloudInput.connect(animator2.output);
    
//    resonator->playParams({ { "frequency", 440 } });
//    resonator->playParams({ { "frequency", 660 } });
    
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
    for (auto i = 0; i < 2; ++i)
        players.emplace_back(make_unique<AudioPlayer>(*entity, "player" + to_string(i + 1)));
    
//    play(0, "part1");
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



