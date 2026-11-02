#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Presets.h"
#include "LoudnessMeter.h"

class MasteringProcessor : public juce::AudioProcessor
{
public:
    MasteringProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Aranguiz Soundevolve"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return (int) mp::presets.size(); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override { return mp::presets[(size_t) index].name; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    LoudnessMeter meter;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void updateEq();

    using Filter = juce::dsp::IIR::Filter<float>;
    using Coeffs = juce::dsp::IIR::Coefficients<float>;
    std::array<juce::dsp::ProcessorDuplicator<Filter, Coeffs>, mp::numEq> eq;
    juce::dsp::Gain<float> inputGain, outputGain, makeup, limiterIn;
    juce::dsp::Compressor<float> comp;
    juce::dsp::Limiter<float> limiter;

    std::array<std::atomic<float>*, mp::numParams> raw {};
    std::array<float, mp::numEq> lastEq;
    double sr = 44100.0;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasteringProcessor)
};
