#include "audioProcessing.hpp"

AudioReaderProcessing::AudioReaderProcessing() 
{
    addAndMakeVisible (_track_gain);
    _track_gain.setSliderStyle(juce::Slider::LinearHorizontal);
    _track_gain.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    _track_gain.setTextValueSuffix(" Db");
    _track_gain.setRange(-_range_Db, _range_Db, 0.05);
    _track_gain.setValue(0.0);
    // ..
    addAndMakeVisible (_gain_label);
    _gain_label.setText ("Volume", juce::NotificationType::dontSendNotification);
    _gain_label.setJustificationType (juce::Justification::centred);
    _gain_label.attachToComponent(&_track_gain, false);

    addAndMakeVisible (_high_shelf_gain);
    _high_shelf_gain.setSliderStyle(juce::Slider::Rotary);
	_high_shelf_gain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
	_high_shelf_gain.setTextValueSuffix(" Db");
    _high_shelf_gain.setRange(-_range_Db, _range_Db, 0.05);
    _high_shelf_gain.setValue(0.0);
    // ..
    addAndMakeVisible (_hs_label);
    _hs_label.setText("Brightness Filter", juce::NotificationType::dontSendNotification);
    _hs_label.attachToComponent(&_high_shelf_gain, false);

    addAndMakeVisible (_low_shelf_gain);
    _low_shelf_gain.setSliderStyle(juce::Slider::Rotary);
	_low_shelf_gain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
	_low_shelf_gain.setTextValueSuffix(" Db");
    _low_shelf_gain.setRange(-_range_Db, _range_Db, 0.05);
    _low_shelf_gain.setValue(0.0);
    // .
    addAndMakeVisible (_ls_label);
    _ls_label.setText("Fullness Filter", juce::NotificationType::dontSendNotification);
    _ls_label.attachToComponent(&_low_shelf_gain, false);
};

void AudioReaderProcessing::resized() 
{
    auto area = getLocalBounds();

    _gain_label.setBounds (area.removeFromTop (20));
    _track_gain.setBounds (area.removeFromTop (30));

    _hs_label.setBounds (area.removeFromTop (20));
    auto hs_area = area.removeFromRight (150);
    hs_area.setHeight(100);
    hs_area.setWidth(100);
    _high_shelf_gain.setBounds (hs_area);

    auto ls_area = area.removeFromRight (100);
    ls_area.setHeight(100);
    _low_shelf_gain.setBounds (ls_area);
};

void AudioReaderProcessing::getNextAudioBlock (const juce::AudioSourceChannelInfo& buffer_to_fill) 
{
    //Db to scale factor
    float scale_gain_ls = std::pow(10, _low_shelf_gain.getValue() / 20); 
    float scale_gain_hs = std::pow(10, _high_shelf_gain.getValue() / 20);

    // ..
    _filter_ls.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf (_samplerate, _ls_cut_freq, _Q, scale_gain_ls);
    _filter_hs.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf (_samplerate, _hs_cut_freq, _Q, scale_gain_hs);

    // ..
    juce::dsp::AudioBlock<float> block (buffer_to_fill.buffer->getArrayOfWritePointers(), 
                                        buffer_to_fill.buffer->getNumChannels(), buffer_to_fill.buffer->getNumSamples());
    _filter_ls.process(juce::dsp::ProcessContextReplacing<float> (block));
    _filter_hs.process(juce::dsp::ProcessContextReplacing<float> (block));
};

void AudioReaderProcessing::prepareToPlay (int samplesPerBlockExpected, double sampleRate) 
{
    _samplerate = static_cast<float>(sampleRate);
    _filter_hs.prepare({sampleRate, static_cast<unsigned int>(samplesPerBlockExpected), _n_channels});
    _filter_ls.prepare({sampleRate, static_cast<unsigned int>(samplesPerBlockExpected), _n_channels});
};

void AudioReaderProcessing::releaseResources() 
{
    _filter_hs.reset();
    _filter_ls.reset();
};

void AudioReaderProcessing::setNChannels(unsigned int n_channels) 
{
    _n_channels = n_channels;
};