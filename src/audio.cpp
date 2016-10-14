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

#include <4dService/SpeakerGrid.h>
#include <4dService/SpatialGranulator.h>

using namespace nap;
using namespace std;


AudioPlayer::AudioPlayer(nap::Entity& root, const std::string& name, nap::JsonComponent& inJsonComponent) : jsonComponent(inJsonComponent)
{
    entity = &root.addEntity(name);
    
    transform = &entity->addComponent<spatial::Transform>("transform");
    transform->scale.setValue(glm::vec3(1, 1, 1));
    patchComponent = &entity->addComponent<PatchComponent>("patch");
    
    // granulator
    granulator = &patchComponent->getPatch().addOperator<spatial::SpatialGranulator>("granulator");
    granulator->channelCount.setValue(8);
    granulator->density.setRange(granulator->density.getMin(), 50);
    granulator->positionSpeed.setValue(0);
    auto& x = granulator->addChild<NumericAttribute<float>>("x");
    auto& z = granulator->addChild<NumericAttribute<float>>("z");
    auto& size = granulator->addChild<NumericAttribute<float>>("size");
    x.setRange(-2, 2);
    x.setValue(0);
    z.setRange(-2, 2);
    z.setValue(0);
    size.setRange(0, 2);
    size.setValue(0);
    std::function<void(const float&)> posChanged = [&](const float& value){
        transform->position.setValue(glm::vec3(x.getValue(), 0, z.getValue()));
    };
    std::function<void(const float&)> sizeChanged = [&](const float& value){
        transform->scale.setValue(glm::vec3(value, value, value));
    };
    x.valueChangedSignal.connect(posChanged);
    z.valueChangedSignal.connect(posChanged);
    size.valueChangedSignal.connect(sizeChanged);
    
    // resonator
    resonator = &patchComponent->getPatch().addOperator<lib::audio::ResonatorUnit>("resonator");
    resonator->channelCount.setValue(8);
    resonator->inputChannelCount.setValue(8);
    
    // audio output
    output = &patchComponent->getPatch().addOperator<lib::audio::OutputUnit>("output");
    output->channelCount.setValue(8);
    output->routing.setValue({ 0, 1, -1, -1, 2, 3, -1, -1 }); // use channel 0, 1, 4 and 5 only
    output->audioInput.connect(granulator->output);
    output->audioInput.connect(resonator->audioOutput);
    resonator->audioInput.connect(granulator->output);
    
    // sequencers
    for (auto i = 0; i < 2; ++i)
    {
        auto& grainSeq = patchComponent->getPatch().addOperator<lib::Sequencer>("grainSequencer" + to_string(i + 1));
        grainSeq.schedulerInput.connect(output->schedulerOutput);
        granulator->cloudInput.connect(grainSeq.output);
        grainSequencers.emplace_back(&grainSeq);
        
        auto& resSeq = patchComponent->getPatch().addOperator<lib::Sequencer>("resonatorSequencer" + to_string(i + 1));
        resSeq.schedulerInput.connect(output->schedulerOutput);
        resonator->input.connect(resSeq.output);
        resonatorSequencers.emplace_back(&resSeq);
        
        // json settings choosers
        auto& grainSeqChooser = entity->addComponent<nap::JsonChooser>();
        grainSeqChooser.setJsonComponent(jsonComponent);
        grainSeqChooser.setTarget(grainSeq);
        grainSeqChooser.optionsJsonPtr.setValue("/granulatorSequences");
        grainParameters.addAttribute(grainSeq.playing);
        grainSeqChooser.choice.setName("sequence");
        grainParameters.addAttribute(grainSeqChooser.choice);
        grainSequenceChoosers.emplace_back(&grainSeqChooser);
        
        auto& grainInputChooser = entity->addComponent<nap::JsonChooser>();
        grainInputChooser.setJsonComponent(jsonComponent);
        grainInputChooser.setTarget(grainSeq.sequences);
        grainInputChooser.optionsJsonPtr.setValue("/granulatorInputs");
        grainInputChooser.choice.setName("input audio");
        grainParameters.addAttribute(grainInputChooser.choice);
        
        auto& resSeqChooser = entity->addComponent<nap::JsonChooser>();
        resSeqChooser.setJsonComponent(jsonComponent);
        resSeqChooser.setTarget(resSeq);
        resSeqChooser.optionsJsonPtr.setValue("/resonatorSequences");
        resonParameters.addAttribute(resSeq.playing);
        resSeqChooser.choice.setName("sequence");
        resonParameters.addAttribute(resSeqChooser.choice);
        resonatorSequenceChoosers.emplace_back(&resSeqChooser);
    }
    
    // add granulator parameters
    grainParameters.addAttribute(granulator->density.proportionAttribute);
    grainParameters.addAttribute(granulator->position.attribute);
    grainParameters.addAttribute(granulator->amplitude.proportionAttribute);
    grainParameters.addAttribute(granulator->amplitudeDev.attribute);
    grainParameters.addAttribute(granulator->duration.proportionAttribute);
    grainParameters.addAttribute(granulator->durationDev.attribute);
    grainParameters.addAttribute(granulator->pitch.attribute);
    grainParameters.addAttribute(granulator->transpose.attribute);
    grainParameters.addAttribute(granulator->positionDev.attribute);
    grainParameters.addAttribute(granulator->irregularity.proportionAttribute);
    grainParameters.addAttribute(granulator->pitchDev.proportionAttribute);
    grainParameters.addAttribute(granulator->panDev.proportionAttribute);
    grainParameters.addAttribute(granulator->shape.attribute);
    grainParameters.addAttribute(granulator->attackDecay.proportionAttribute);
    grainParameters.addAttribute(x);
    grainParameters.addAttribute(z);
    grainParameters.addAttribute(size);
    
    // add resonator parameters
    resonParameters.addAttribute(resonator->amplitude.attribute);
    resonParameters.addAttribute(resonator->attack.attribute);
    resonParameters.addAttribute(resonator->releaseTime.attribute);
    resonParameters.addAttribute(resonator->damping.proportionAttribute);
    resonParameters.addAttribute(resonator->feedback.proportionAttribute);
    resonParameters.addAttribute(resonator->detune.proportionAttribute);
    resonParameters.addAttribute(resonator->polarity.attribute);

    // granulator animators
    createModulator(granulator->density, densityParameters);

    createModulator(granulator->position, positionParameters);
    
    grainParameters.setName("granulator");
    resonParameters.setName("resonator");
    positionParameters.setName("position modulation");
    densityParameters.setName("density modulation");
}


void AudioPlayer::createModulator(lib::ValueControl& control, OFAttributeWrapper& parameters)
{
    auto& animator = patchComponent->getPatch().addOperator<lib::RampSequencer>("animator");
    animator.schedulerInput.connect(output->schedulerOutput);
    control.proportionPlug.connect(animator.output);
    animator.startValue.link(control.proportionAttribute);
    
    animator.sequence.setValue({ 0, 1 });
    animator.times.addAttribute(5000);
    animator.times.addAttribute(5000);
    
    auto& centerValue = animator.addChild<NumericAttribute<float>>("center");
    centerValue.setRange(0., 1.);
    centerValue.setValue(0.5);
    
    auto& range = animator.addChild<NumericAttribute<float>>("range");
    range.setRange(0., 1.);
    range.setValue(1.);
    
    std::function<void(const float&)> calcSequence = [&](const float&){
        animator.sequence.getValueRef()[0] = std::max<float>(centerValue.getValue() - (range.getValue()/2.), 0);
        animator.sequence.getValueRef()[1] = std::min<float>(centerValue.getValue() + (range.getValue()/2.), 1);
    };
    
    centerValue.valueChangedSignal.connect(calcSequence);
    range.valueChangedSignal.connect(calcSequence);
    
    parameters.addAttribute(animator.playing);
    parameters.addAttribute(centerValue);
    parameters.addAttribute(range);
    parameters.addAttribute(animator.speed.proportionAttribute);
    
}


void AudioPlayer::setupGui(ofxPanel& panel)
{
    panel.setName(entity->getName());
    panel.setup();
    panel.add(grainParameters.getGroup());
    panel.add(positionParameters.getGroup());
    panel.add(densityParameters.getGroup());
    panel.add(resonParameters.getGroup());
    panel.minimizeAll();    
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
    
    auto& speakerGrid = entity->addComponent<spatial::SpeakerGrid>();
    
    // add the player
    for (auto i = 0; i < 2; ++i)
        players.emplace_back(make_unique<AudioPlayer>(*entity, "player" + to_string(i + 1), *jsonComponent));
    
    play(0, "init/1");
    play(1, "init/2");
    
//    mCurrentPartIndex = jsonComponent->getNumber<int>("/start", 0);
//    play(0, mCurrentPartIndex);
//    play(1, mCurrentPartIndex);

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
    
    if (it == parts->MemberEnd())
    {
        Logger::warn("Part not found: " + to_string(index));
        return;
    }
    
    auto name = it->name.GetString();
    rapidjson::Value* json = &it->value;
    
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

