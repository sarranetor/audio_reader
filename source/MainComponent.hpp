#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_dsp/juce_dsp.h>
#include "utils.hpp"
#include "audioFileReaderComponent.hpp"
#include "audioProcessing.hpp"
#include "stft.hpp"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public juce::AudioAppComponent,
                        public juce::Timer
{
public:
    MainComponent();

    ~MainComponent() override
    {
        shutdownAudio();
    }

    void paint (juce::Graphics& g) override;

    void resized() override;

    // no processing just foreward it for playback
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& buffer_to_fill) override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    void releaseResources() override;

    void timerCallback() override;
  
    // draw spectrum
    void drawFrame (juce::Graphics& g);

    // audio processing ..
    void pushNextSampleIntoFifo(float sample);
    void drawNextFrameOfSpectrum();

private:
    // fft
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    float fifo [fftSize]; // will contain incoming audio buffer
    float fftData [2 * fftSize]; // will contain the result of FFT calculation
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    float scopeData [scopeSize]; // will contain points to display

    AudioFileReaderComponent _reader;

    AudioReaderProcessing _dsp_processor;

    STFT _stft;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
