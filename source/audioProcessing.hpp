#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#ifndef AUDIOPROCESSING_H
#define AUDIOPROCESSING_H

class AudioReaderProcessing : public juce::Component
{
    float _range_Db {20.0};
    float _hs_cut_freq {2500.0};
    float _ls_cut_freq {500.0};
    float _Q {0.707};
    float _samplerate {44100.0};

    juce::Slider _high_shelf_gain;
	juce::Slider _low_shelf_gain;
    juce::Label _ls_label;
    juce::Label _hs_label;

    juce::Slider _track_gain;
    juce::Label _gain_label;

    unsigned int _n_channels {2};
    juce::dsp::IIR::Filter<float> _filter_ls;
    juce::dsp::IIR::Filter<float> _filter_hs;

    public:
    AudioReaderProcessing();

    ~AudioReaderProcessing() = default;

    void resized() override;

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& buffer_to_fill);

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);

    void releaseResources();

    void setNChannels(unsigned int n_channels);
};

#endif