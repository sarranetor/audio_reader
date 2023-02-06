#include "audioReaderProcessing.hpp"

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
    _hs_label.setText("Highs", juce::NotificationType::dontSendNotification);
    _hs_label.attachToComponent(&_high_shelf_gain, false);

    addAndMakeVisible (_low_shelf_gain);
    _low_shelf_gain.setSliderStyle(juce::Slider::Rotary);
	_low_shelf_gain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
	_low_shelf_gain.setTextValueSuffix(" Db");
    _low_shelf_gain.setRange(-_range_Db, _range_Db, 0.05);
    _low_shelf_gain.setValue(0.0);
    // .
    addAndMakeVisible (_ls_label);
    _ls_label.setText("Lows", juce::NotificationType::dontSendNotification);
    _ls_label.attachToComponent(&_low_shelf_gain, false);

    addAndMakeVisible (_peaking_gain);
    _peaking_gain.setSliderStyle(juce::Slider::Rotary);
	_peaking_gain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
	_peaking_gain.setTextValueSuffix(" Db");
    _peaking_gain.setRange(-_range_Db, _range_Db, 0.05);
    _peaking_gain.setValue(0.0);
    // .
    addAndMakeVisible (_peaking_label);
    _peaking_label.setText("Mids", juce::NotificationType::dontSendNotification);
    _peaking_label.attachToComponent(&_peaking_gain, false);

    addAndMakeVisible (_disable_filt);
    _disable_filt.setClickingTogglesState(true);
    _disable_filt.onClick = [this] { _disable_filtering(); };
};

void AudioReaderProcessing::resized() 
{
    auto area = getLocalBounds();

    _gain_label.setBounds (area.removeFromTop (20));
    _track_gain.setBounds (area.removeFromTop (30));

    auto disfilt_area = area.removeFromTop(20);
    disfilt_area.reduce(90, 0);
    _disable_filt.setBounds(disfilt_area);

    _hs_label.setBounds (area.removeFromTop (20));
    auto hs_area = area.removeFromRight (100);
    hs_area.setHeight(100);
    hs_area.setWidth(100);
    _high_shelf_gain.setBounds (hs_area);

    auto p_area = area.removeFromRight (100);
    p_area.setHeight(100);
    _peaking_gain.setBounds (p_area);

    auto ls_area = area.removeFromRight (100);
    ls_area.setHeight(100);
    _low_shelf_gain.setBounds (ls_area);
};

void AudioReaderProcessing::getNextAudioBlock (const juce::AudioSourceChannelInfo& buffer_to_fill) 
{
    //Db to scale factor
    if (!_disable_filt.getToggleState())
    {
        _scale_gain_ls = std::pow(10, _low_shelf_gain.getValue() / 20); 
        _scale_gain_hs = std::pow(10, _high_shelf_gain.getValue() / 20);
        _scale_gain_peaking = std::pow(10, _peaking_gain.getValue() / 20);
    } else if (_disable_filt.getToggleState())
    {
        _scale_gain_ls = 1.0;
        _scale_gain_hs = 1.0;
        _scale_gain_peaking = 1.0;
    }

    // set dsp processors parameters
    _filter_ls.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf (_samplerate, _ls_cut_freq, _Q, _scale_gain_ls);
    _filter_mids.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter (_samplerate, _peaking_freq, _Q, _scale_gain_peaking);
    _filter_hs.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf (_samplerate, _hs_cut_freq, _Q, _scale_gain_hs);
    _fader.setGainDecibels(_track_gain.getValue());

    // process audio
    juce::dsp::AudioBlock<float> block (buffer_to_fill.buffer->getArrayOfWritePointers(), 
                                        buffer_to_fill.buffer->getNumChannels(), buffer_to_fill.buffer->getNumSamples());
    _filter_ls.process(juce::dsp::ProcessContextReplacing<float> (block));
    _filter_hs.process(juce::dsp::ProcessContextReplacing<float> (block));
    _filter_mids.process(juce::dsp::ProcessContextReplacing<float> (block));
    _fader.process(juce::dsp::ProcessContextReplacing<float> (block));
};

void AudioReaderProcessing::prepareToPlay (int samplesPerBlockExpected, double sampleRate) 
{
    _samplerate = static_cast<float>(sampleRate);
    _filter_hs.prepare({sampleRate, static_cast<unsigned int>(samplesPerBlockExpected), _n_channels});
    _filter_ls.prepare({sampleRate, static_cast<unsigned int>(samplesPerBlockExpected), _n_channels});
    _filter_mids.prepare({sampleRate, static_cast<unsigned int>(samplesPerBlockExpected), _n_channels});
    _fader.prepare({sampleRate, static_cast<unsigned int>(samplesPerBlockExpected), _n_channels});
};

void AudioReaderProcessing::releaseResources() 
{
    _filter_hs.reset();
    _filter_ls.reset();
    _filter_mids.reset();
    _fader.reset();
};

void AudioReaderProcessing::setNChannels(unsigned int n_channels) 
{
    _n_channels = n_channels;
};

void AudioReaderProcessing::_disable_filtering()
{
    bool state = _disable_filt.getToggleState();
    if (state == true)
    {
        _low_shelf_gain.setEnabled(false);
        _high_shelf_gain.setEnabled(false);
        _peaking_gain.setEnabled(false);
    } else if (state == false) 
    {
        _low_shelf_gain.setEnabled(true);
        _high_shelf_gain.setEnabled(true);
        _peaking_gain.setEnabled(true);
    }
};