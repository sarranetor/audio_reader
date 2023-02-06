#include "MainComponent.hpp"

MainComponent::MainComponent()
{
    // make components visible
    addAndMakeVisible (_reader);
    addAndMakeVisible (_dsp_processor);
    // App title
    addAndMakeVisible (_title);
    _title.setEnabled (true);
    _title.setText ("L.S. Audio Effects", juce::NotificationType::dontSendNotification);
    _title.setFont (juce::Font (16.0f, juce::Font::italic));
    _title.setColour (juce::Label::textColourId, juce::Colours::azure);
    _title.setJustificationType (juce::Justification::centred);

    // time listener set interval
    startTimerHz (30);
    // GUI size
    setSize (600, 400);
    // 2 audio input and 2 andio output
    setAudioChannels (2, 2);
};

void MainComponent::paint (juce::Graphics& g)
{
    // Our component is opaque, so we must completely fill the background with a solid colour
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    _stft.drawFrame(g, getLocalBounds()); //draw FFT
};

void MainComponent::resized()
{
    auto area = getLocalBounds();

    auto title_rect = area.removeFromBottom(210);
    title_rect.setHeight(40);
    _title.setBounds (title_rect);

    auto reader_rect = area.removeFromLeft(300);
    reader_rect.setHeight(200);
    _reader.setBounds (reader_rect);

    auto dsp_rect = area.removeFromLeft(300);
    dsp_rect.setHeight(200);
    _dsp_processor.setBounds (dsp_rect);
};

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& buffer_to_fill) 
{ 
    if (!_reader.isSourcePresent()) // if there is no file ready to be played 
    {
        buffer_to_fill.clearActiveBufferRegion();
        return;
    }

    // fill buffer_to_fill buffer from WAV file 
    _reader.getNextAudioBlock (buffer_to_fill);

    // filter audio buffer
    _dsp_processor.getNextAudioBlock (buffer_to_fill);

    // fill stft fifo for displaying FFT on GUI
    _stft.fillFifo(buffer_to_fill, 0);
};

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate) 
{
    _reader.prepareToPlay (samplesPerBlockExpected, sampleRate);
    _dsp_processor.prepareToPlay (samplesPerBlockExpected, sampleRate);
};

void MainComponent::releaseResources()
{
    _reader.releaseResources();
    _dsp_processor.releaseResources();
};

void MainComponent::timerCallback() 
{
    if (_stft.isNextBlockReady()) // FFT ready to be plot
    {
        _stft.drawNextFrameOfSpectrum();
        _stft.setBlockNotReady();
        // repaing will tell the App to call the paint method as soon as possible
        repaint(); 
    }
};