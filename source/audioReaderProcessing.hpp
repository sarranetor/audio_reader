#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

/*
    AudioReaderProcessing GUI component capable of processing the audio according to GUI controllers.
    processing chain: low shelving filter (for low end), high shelving filter (for high end), fader for controlling track volume.
*/
class AudioReaderProcessing : public juce::Component
{
    float _range_Db {20.0}; // Db range of controllers
    float _hs_cut_freq {2000.0};
    float _ls_cut_freq {400.0};
    float _peaking_freq {700.0};
    float _Q {0.707};
    float _samplerate {44100.0};

    // filters gain control 
    juce::Slider _high_shelf_gain;
	juce::Slider _low_shelf_gain;
    juce::Slider _peaking_gain;
    juce::Label _ls_label;
    juce::Label _hs_label;
    juce::Label _peaking_label;
    // volume slder gain control 
    juce::Slider _track_gain;
    juce::Label _gain_label;
    // activate or deactivate filreting
    juce::ToggleButton _disable_filt {"Disable Filtering"};

    // filters gain factor (not in Db)
    float _scale_gain_ls {1.0};
    float _scale_gain_hs {1.0};
    float _scale_gain_peaking {1.0};

    unsigned int _n_channels {2}; // two channels are processed by default
    // biquad IIR filters
    juce::dsp::IIR::Filter<float> _filter_ls;
    juce::dsp::IIR::Filter<float> _filter_mids;
    juce::dsp::IIR::Filter<float> _filter_hs;
    juce::dsp::Gain<float> _fader;

    /* Disable gain knobs for setting filters gain depending on _disable_filt button state */
    void _disable_filtering();

    public:

    /* AudioReaderProcessing constructor */
    AudioReaderProcessing();

    /* AudioReaderProcessing destructor */
    ~AudioReaderProcessing() = default;

    /* place internal compoment on GUI */
    void resized() override;

    /* process buffer_to_fill */
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& buffer_to_fill);

    /* set necessary value before palying */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);

    /* release resouces when stop playing*/
    void releaseResources();
    
    /* set number of channels to be processed */
    void setNChannels(unsigned int n_channels);
};