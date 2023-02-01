#include "MainComponent.hpp"

MainComponent::MainComponent()
{
    addAndMakeVisible (_reader);
    addAndMakeVisible (_dsp_processor);

    startTimerHz (30);
    setSize (600, 400);
    setAudioChannels (2, 2);
};

void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::Font (16.0f));
    _stft.drawFrame(g, getLocalBounds());
};

void MainComponent::resized()
{
    auto area = getLocalBounds();

    auto reader_rect = area.removeFromLeft(300);
    reader_rect.setHeight(250);
    _reader.setBounds (reader_rect);

    auto dsp_rect = area.removeFromLeft(300);
    dsp_rect.setHeight(250);
    _dsp_processor.setBounds (dsp_rect);
};

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& buffer_to_fill) 
{ 
    if (!_reader.isSourcePresent())
    {
        buffer_to_fill.clearActiveBufferRegion();
        return;
    }

    _reader.getNextAudioBlock (buffer_to_fill);
    _dsp_processor.getNextAudioBlock (buffer_to_fill);

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
    if (_stft.isNextBlockReady())
    {
        _stft.drawNextFrameOfSpectrum();
        _stft.setBlockNotReady();
        repaint();
    }
};