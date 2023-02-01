#include "MainComponent.hpp"

MainComponent::MainComponent()
    : forwardFFT (fftSize),
    window (fftSize, juce::dsp::WindowingFunction<float>::hann)
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
    // g.setColour (juce::Colours::white);
    // g.drawText ("Hello User!", getLocalBounds(), juce::Justification::centred, true);
    //drawFrame (g);
    _stft.drawFrame(g, getLocalBounds());
};

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
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

    /*if (buffer_to_fill.buffer->getNumChannels() > 0)
    {
        auto* channel0_data = buffer_to_fill.buffer->getReadPointer(0, buffer_to_fill.startSample);

        for (int i = 0; i<buffer_to_fill.numSamples; i++)
            pushNextSampleIntoFifo(channel0_data[i]);
    }*/

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

void MainComponent::pushNextSampleIntoFifo(float sample)
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

void MainComponent::drawNextFrameOfSpectrum()
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

void MainComponent::drawFrame (juce::Graphics& g)
{
    for (int i = 1; i < scopeSize; ++i)
    {
        auto width  = getLocalBounds().getWidth();
        auto height = getLocalBounds().getHeight() - 10; 

        g.setColour (juce::Colours::white);
        g.drawLine ({ (float) juce::jmap (i - 1, 0, scopeSize - 1, 0, width),
                              juce::jmap (scopeData[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
                      (float) juce::jmap (i,     0, scopeSize - 1, 0, width),
                              juce::jmap (scopeData[i],     0.0f, 1.0f, (float) height, 0.0f) });
    }
};

void MainComponent::timerCallback() 
{
    /*if (nextFFTBlockReady)
    {
        drawNextFrameOfSpectrum();
        nextFFTBlockReady = false;
        repaint();
    }*/

    if (_stft.isNextBlockReady())
    {
        _stft.drawNextFrameOfSpectrum();
        _stft.setBlockNotReady();
        repaint();
    }
};