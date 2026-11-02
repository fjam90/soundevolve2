#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <cmath>

// Medidor LUFS aproximado según ITU-R BS.1770 / EBU R128:
// ponderación K (aproximada con biquads de JUCE), momentáneo (400 ms)
// e integrado con compuertas (-70 LUFS absoluta, -10 LU relativa).
class LoudnessMeter
{
public:
    void prepare (double sr)
    {
        auto hp    = juce::dsp::IIR::Coefficients<float>::makeHighPass (sr, 38.0f, 0.5f);
        auto shelf = juce::dsp::IIR::Coefficients<float>::makeHighShelf (sr, 1681.97f, 0.7071f,
                                                                         juce::Decibels::decibelsToGain (4.0f));
        for (auto& c : ch) { c.hp.coefficients = hp; c.shelf.coefficients = shelf; }
        subBlockLen = (int) std::round (sr * 0.1);
        resetAll();
    }

    void requestReset() { resetFlag = true; }
    float getMomentary()  const { return momentary.load(); }
    float getIntegrated() const { return integrated.load(); }

    void process (const juce::AudioBuffer<float>& buf)
    {
        if (resetFlag.exchange (false)) resetAll();

        const int n = buf.getNumSamples();
        const int nch = juce::jmin (2, buf.getNumChannels());

        for (int i = 0; i < n; ++i)
        {
            for (int c = 0; c < nch; ++c)
            {
                float x = buf.getSample (c, i);
                x = ch[(size_t) c].hp.processSample (x);
                x = ch[(size_t) c].shelf.processSample (x);
                subSum += (double) x * (double) x;
            }
            if (++subCount >= subBlockLen) finishSubBlock();
        }
    }

private:
    static constexpr int numBins = 1200; // pasos de 0.1 LU desde -70 LUFS

    struct ChannelFilters { juce::dsp::IIR::Filter<float> hp, shelf; };
    std::array<ChannelFilters, 2> ch;

    int subBlockLen = 4800, subCount = 0;
    double subSum = 0.0;
    std::array<double, 4> ring {};
    int ringPos = 0, ringFilled = 0;

    std::array<uint32_t, numBins> counts {};
    std::array<double, numBins> energy {};

    std::atomic<float> momentary { -100.f }, integrated { -100.f };
    std::atomic<bool> resetFlag { false };

    void resetAll()
    {
        for (auto& c : ch) { c.hp.reset(); c.shelf.reset(); }
        subCount = 0; subSum = 0.0; ringPos = 0; ringFilled = 0;
        ring.fill (0.0); counts.fill (0); energy.fill (0.0);
        momentary = -100.f; integrated = -100.f;
    }

    static double toLufs (double e) { return -0.691 + 10.0 * std::log10 (e); }

    void finishSubBlock()
    {
        ring[(size_t) ringPos] = subSum / (double) subCount;
        ringPos = (ringPos + 1) % 4;
        ringFilled = juce::jmin (4, ringFilled + 1);
        subSum = 0.0; subCount = 0;

        if (ringFilled < 4) return;

        const double e = (ring[0] + ring[1] + ring[2] + ring[3]) * 0.25;
        if (e <= 1.0e-12) { momentary = -100.f; return; }

        const double l = toLufs (e);
        momentary = (float) l;

        if (l > -70.0)
        {
            int idx = juce::jlimit (0, numBins - 1, (int) std::floor ((l + 70.0) * 10.0));
            counts[(size_t) idx]++;
            energy[(size_t) idx] += e;
        }

        double sumE = 0.0; uint64_t cnt = 0;
        for (int b = 0; b < numBins; ++b) { sumE += energy[(size_t) b]; cnt += counts[(size_t) b]; }
        if (cnt == 0) return;

        const double relThresh = toLufs (sumE / (double) cnt) - 10.0;
        const int startBin = juce::jmax (0, (int) std::ceil ((relThresh + 70.0) * 10.0));

        sumE = 0.0; cnt = 0;
        for (int b = startBin; b < numBins; ++b) { sumE += energy[(size_t) b]; cnt += counts[(size_t) b]; }
        if (cnt > 0) integrated = (float) toLufs (sumE / (double) cnt);
    }
};
