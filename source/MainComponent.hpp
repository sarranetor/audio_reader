#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "audioFileReaderComponent.hpp"
#include "audioReaderProcessing.hpp"
#include "stft.hpp"

/*
    MainComponent lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public juce::AudioAppComponent,
                        public juce::Timer
{
public:
    /* MainComponent constructor */
    MainComponent();
 
    /* MainComponent destructor */
    ~MainComponent() override
    {
        shutdownAudio();
    }

    /* draw graphics on GUI */
    void paint (juce::Graphics& g) override;

    /* 
       This is called when the MainComponent is resized.
       If you add any child components, this is where you should update their positions.
    */
    void resized() override;

    /* fill buffer_to_fill with desidered audio output */
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& buffer_to_fill) override;

    /* set necessary value before palying */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    /* release resouces when stop playing */
    void releaseResources() override;

    /* method called every 30 times per second to draw FFT */
    void timerCallback() override;

private:

    // component for opening, reading and pyaing an audio file
    AudioFileReaderComponent _reader;
    // component to filter and apply gain to played audio
    AudioReaderProcessing _dsp_processor;
    // compute FFT from filled buffer and draw graphics
    STFT _stft;
    // App title
    juce::Label _title;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
