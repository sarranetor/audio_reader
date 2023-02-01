#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>

/* enum containing the order of FFT to be computed and number of points to be displayed */
enum fft { 
    fftOrder = 11, 
    fftSize = 1 << fftOrder, // 2^fftOrder
    scopeSize = 512 // number of points in the visual representation
};

/* 
    STFT is a class for computing filling a fifo array with incomung audio,
    computing the FFT when fifo is full and drawing the result on GUI.
*/
class STFT 
{ 
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    // will contain incoming audio 
    float fifo [fftSize]; 
    // will store audio when fifo is full and will store FFT after computation.
    // needs to be 2 * size_fft according to dsp::FFT class doc
    float fftData [2 * fftSize]; 
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    // will contain points to display on GUI
    float scopeData [scopeSize]; 

    /* push sample into fifo array */
    void pushNextSampleIntoFifo(float sample);

    public:

    /* STFT constructor*/
    STFT();

    /* STFT destructor*/
    ~STFT() = default;

    /* Compute FFT and fill scopeData to be displayed */
    void drawNextFrameOfSpectrum();

    /* Draw FFT on GUI graphics from the bottom */
    void drawFrame (juce::Graphics& g, juce::Rectangle<int> local_bounds);

    /* return true if FFT block ready to be computed and there is an available frame to draw */
    bool isNextBlockReady();

    /* set false when a new audio block needs to be acquired */
    void setBlockNotReady();

    /* push channel of buffer_to_fill nto fifo array */
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
    // perform FFT
    forwardFFT.performFrequencyOnlyForwardTransform (fftData);  

    float mindB = -100.0;
    float maxdB =    0.0;
    float expantion_factor = 0.4;

    // scale fft to display and store FFT magnitude in scopeData array
    for (int i = 0; i < scopeSize; ++i)                        
    {
        auto skewedProportionX = 1.0f - std::exp (std::log (1.0 - (float) i / (float) scopeSize) * expantion_factor);
        auto fftDataIndex = juce::jlimit (0, fftSize / 2, (int) (skewedProportionX * (float) fftSize * 0.5f));

        auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (fftData[fftDataIndex])
                                                           - juce::Decibels::gainToDecibels ((float) fftSize)),
                                 mindB, maxdB, 0.0f, 1.0f);

        scopeData[scopeSize - i -1] = level; // not sure why need to be stored in reverse                    
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