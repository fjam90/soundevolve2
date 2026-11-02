#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout MasteringProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (int i = 0; i < mp::numParams; ++i)
    {
        const auto& r = mp::ranges[(size_t) i];
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { mp::paramIds[(size_t) i], 1 },
            mp::paramNames[(size_t) i],
            juce::NormalisableRange<float> (r[0], r[1], r[2]),
            mp::presets[0].v[(size_t) i]));
    }
    return layout;
}

MasteringProcessor::MasteringProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    for (int i = 0; i < mp::numParams; ++i)
        raw[(size_t) i] = apvts.getRawParameterValue (mp::paramIds[(size_t) i]);
    lastEq.fill (std::numeric_limits<float>::quiet_NaN());
}

bool MasteringProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void MasteringProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };

    for (auto& e : eq) e.prepare (spec);
    comp.prepare (spec);
    limiter.prepare (spec);
    for (auto* g : { &inputGain, &outputGain, &makeup, &limiterIn })
    {
        g->prepare (spec);
        g->setRampDurationSeconds (0.02);
    }

    lastEq.fill (std::numeric_limits<float>::quiet_NaN());
    meter.prepare (sampleRate);
}

// EQ gráfico de 10 bandas (filtros peak, Q ~ 1.41 = ancho de octava)
void MasteringProcessor::updateEq()
{
    for (int b = 0; b < mp::numEq; ++b)
    {
        const float g = raw[(size_t) b]->load();
        if (g != lastEq[(size_t) b])
        {
            const float f = juce::jmin (mp::eqFreqs[(size_t) b], (float) (sr * 0.45));
            *eq[(size_t) b].state = *Coeffs::makePeakFilter (sr, f, 1.414f, juce::Decibels::decibelsToGain (g));
            lastEq[(size_t) b] = g;
        }
    }
}

void MasteringProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto get = [this] (int i) { return raw[(size_t) i]->load(); };

    updateEq();
    inputGain.setGainDecibels (get (mp::pInput));
    comp.setThreshold (get (mp::pThresh)); comp.setRatio (get (mp::pRatio));
    comp.setAttack (get (mp::pAttack));    comp.setRelease (get (mp::pRelease));
    makeup.setGainDecibels (get (mp::pMakeup));
    limiterIn.setGainDecibels (get (mp::pLimGain));
    limiter.setThreshold (get (mp::pCeiling)); limiter.setRelease (100.0f);
    outputGain.setGainDecibels (get (mp::pOutput));

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);

    // 0) Input -> 1) EQ gráfico -> 2) Compresor + makeup
    inputGain.process (ctx);
    for (auto& e : eq) e.process (ctx);
    comp.process (ctx);
    makeup.process (ctx);

    const int n = buffer.getNumSamples();
    auto* L = buffer.getWritePointer (0);
    auto* R = buffer.getWritePointer (1);

    // 3) Saturación (tanh normalizada, se omite si drive ~ 0)
    const float driveDb = get (mp::pDrive);
    if (driveDb > 0.01f)
    {
        const float g = juce::Decibels::decibelsToGain (driveDb);
        for (int i = 0; i < n; ++i)
        {
            L[i] = std::tanh (g * L[i]) / g;
            R[i] = std::tanh (g * R[i]) / g;
        }
    }

    // 4) Ancho estéreo (Mid/Side)
    const float width = get (mp::pWidth) * 0.01f;
    if (std::abs (width - 1.0f) > 0.001f)
    {
        for (int i = 0; i < n; ++i)
        {
            const float m = 0.5f * (L[i] + R[i]);
            const float s = 0.5f * (L[i] - R[i]) * width;
            L[i] = m + s;
            R[i] = m - s;
        }
    }

    // 5) Ganancia al limitador + limitador (ceiling) -> 6) Output (solo atenúa: -24..0 dB)
    limiterIn.process (ctx);
    limiter.process (ctx);
    outputGain.process (ctx);

    // 7) Medidor LUFS sobre la salida final
    meter.process (buffer);
}

void MasteringProcessor::setCurrentProgram (int index)
{
    index = juce::jlimit (0, (int) mp::presets.size() - 1, index);
    currentProgram = index;
    for (int i = 0; i < mp::numParams; ++i)
        if (auto* p = apvts.getParameter (mp::paramIds[(size_t) i]))
            p->setValueNotifyingHost (p->convertTo0to1 (mp::presets[(size_t) index].v[(size_t) i]));
}

void MasteringProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, dest);
}

void MasteringProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            currentProgram = juce::jlimit (0, (int) mp::presets.size() - 1, (int) tree.getProperty ("program", 0));
            apvts.replaceState (tree);
        }
}

juce::AudioProcessorEditor* MasteringProcessor::createEditor() { return new MasteringEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MasteringProcessor(); }
