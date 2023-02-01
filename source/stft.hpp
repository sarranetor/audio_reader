// class for filtering a buffer
// filter can be set with different filters to be applied, or only one filter? 
// in the latter two instance of filter would be needed for filtering with two different filters

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>
#include "utils.hpp"

/*
enum fft { 
    fftOrder = 11, 
    fftSize = 1 << fftOrder, // 2^fftOrder
    scopeSize = 512 // number of points in the visual representation
};*/

class STFT 
{ 
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    float fifo [fftSize]; // will contain incoming audio buffer
    float fftData [2 * fftSize]; // will contain the result of FFT calculation
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    float scopeData [scopeSize]; // will contain points to display

    void pushNextSampleIntoFifo(float sample);

    public:
    STFT();

    ~STFT() = default;

    void drawNextFrameOfSpectrum();

    void drawFrame (juce::Graphics& g, juce::Rectangle<int> local_bounds);

    bool isNextBlockReady();

    void setBlockNotReady();

    void fillFifo(const juce::AudioSourceChannelInfo& buffer_to_fill, int channel);
};

inline STFT::STFT() 
 : forwardFFT (fftSize),
   window (fftSize, juce::dsp::WindowingFunction<float>::hann)
{
};

inline void STFT::drawFrame (juce::Graphics& g, juce::Rectangle<int> local_bounds) 
{
    for (int i = 1; i < scopeSize; ++i)
    {
        auto width  = local_bounds.getWidth();
        auto height = local_bounds.getHeight() - 10; 

        g.setColour (juce::Colours::white);
        g.drawLine ({ (float) juce::jmap (i - 1, 0, scopeSize - 1, 0, width),
                              juce::jmap (scopeData[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
                      (float) juce::jmap (i,     0, scopeSize - 1, 0, width),
                              juce::jmap (scopeData[i],     0.0f, 1.0f, (float) height, 0.0f) });
    }
};

inline void STFT::pushNextSampleIntoFifo(float sample) 
{
    // if fifo full get it ready for next buffer
    if (fifoIndex == fftSize)
    {
        if (! nextFFTBlockReady)
        {
            juce::zeromem (fftData, sizeof (fftData));
            memcpy (fftData, fifo, sizeof (fifo));
            nextFFTBlockReady = true;
        }
        fifoIndex = 0;
    }
    fifo[fifoIndex++] = sample;
};

inline void STFT::drawNextFrameOfSpectrum() 
{
    // first apply a windowing function to our data
    window.multiplyWithWindowingTable (fftData, fftSize);       

    // then render our FFT data..
    forwardFFT.performFrequencyOnlyForwardTransform (fftData);  

    auto mindB = -100.0f;
    auto maxdB =    0.0f;

    // scale fft to display
    for (int i = 0; i < scopeSize; ++i)                        
    {
        auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) scopeSize) * 0.8f);
        auto fftDataIndex = juce::jlimit (0, fftSize / 2, (int) (skewedProportionX * (float) fftSize * 0.5f));
        auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (fftData[fftDataIndex])
                                                           - juce::Decibels::gainToDecibels ((float) fftSize)),
                                 mindB, maxdB, 0.0f, 1.0f);

        scopeData[scopeSize -1 - i] = level; // not sure why needs to be filled in reverse ..                     
    }
};

inline bool STFT::isNextBlockReady() 
{
    return nextFFTBlockReady;
};

inline void STFT::fillFifo(const juce::AudioSourceChannelInfo& buffer_to_fill, int channel) 
{
    auto* channel_data = buffer_to_fill.buffer->getReadPointer(channel, buffer_to_fill.startSample);

    for (int i = 0; i<buffer_to_fill.numSamples; i++)
        pushNextSampleIntoFifo(channel_data[i]);
};

inline void STFT::setBlockNotReady()
{
    nextFFTBlockReady = false;
};