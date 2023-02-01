#include "MainComponent.hpp"

MainComponent::MainComponent()
    : state (Stopped),
    forwardFFT (fftSize),
    window (fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    addAndMakeVisible (open_button);
    open_button.setButtonText ("Open...");
    open_button.onClick = [this] { openButtonClicked(); };

    addAndMakeVisible (play_button);
    play_button.setButtonText ("Play");
    play_button.onClick = [this] { playButtonClicked(); };
    play_button.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    play_button.setEnabled(false);

    addAndMakeVisible (stop_button);
    stop_button.setButtonText ("Stop");
    stop_button.onClick = [this] { stopButtonClicked(); };
    stop_button.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    stop_button.setEnabled(false);

    // to do add mp3 ..
    format_manager.registerBasicFormats(); // initialize manager for at least WAV and AIFF formats

    addAndMakeVisible (track);
    track.setSliderStyle(juce::Slider::LinearHorizontal);
    track.setEnabled(false);
    track.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    track.setRange(0, 1, 0.01);
    track.onDragEnd = [this] { moveTrackSlider(); };

    addAndMakeVisible (track_position);
    track_position.setEnabled(true);
    track_position.setColour (juce::Label::backgroundColourId, juce::Colours::black);
    track_position.setColour (juce::Label::textColourId, juce::Colours::white);
    track_position.setJustificationType (juce::Justification::centred);

    transport_source.addChangeListener (this); // add as a listener so we can respond to changes in transpost_source state
    setAudioChannels (2, 2);

    // sliders knob setting
    addAndMakeVisible (high_shelf_gain);
    high_shelf_gain.setSliderStyle(juce::Slider::Rotary);
	high_shelf_gain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
	high_shelf_gain.setTextValueSuffix(" Db");
    high_shelf_gain.setRange(-_range_Db, _range_Db, 0.05);
    high_shelf_gain.setValue(0.0);

    addAndMakeVisible (hs_text);
    hs_text.setText("Brightness");

    addAndMakeVisible (low_shelf_gain);
    low_shelf_gain.setSliderStyle(juce::Slider::Rotary);
	low_shelf_gain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
	low_shelf_gain.setTextValueSuffix(" Db");
    //low_shelf_gain.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::red);
    low_shelf_gain.setRange(-_range_Db, _range_Db, 0.05);
    low_shelf_gain.setValue(0.0);

    addAndMakeVisible (ls_text);
    ls_text.setText("Fullness");

    // filter init
    // ..

    addAndMakeVisible (_reader);

    addAndMakeVisible (_dsp_processor);

    startTimerHz (30);
    setSize (600, 400);
};

void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::Font (16.0f));
    // g.setColour (juce::Colours::white);
    // g.drawText ("Hello User!", getLocalBounds(), juce::Justification::centred, true);
    drawFrame (g);
};

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    //open_button.setBounds (10, 10, getWidth() - 20, 30);
    //play_button.setBounds (10, 50, getWidth() - 20, 30);
    //stop_button.setBounds (10, 90, getWidth() - 20, 30);

    //track.setBounds(10, 180, 360, 20);
    //track_position.setBounds(155, 150, 50, 30);
    auto reader_rect = area.removeFromLeft(300);
    reader_rect.setHeight(250);
    _reader.setBounds (reader_rect);

    // int size = 100;
    //low_shelf_gain.setBounds(400, 160, size, size);
    //high_shelf_gain.setBounds(500, 160, size, size);

    //ls_text.setBounds(380, 155, 57, 18);
    //hs_text.setBounds(470, 155, 71, 18);

    auto dsp_rect = area.removeFromLeft(300);
    dsp_rect.setHeight(250);
    _dsp_processor.setBounds (dsp_rect);
};

// mi sa che si puo fare anche quando i bottoni vengono premuti .. comunque ..
void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source) 
{
    if (source == &transport_source) { 
        if (transport_source.isPlaying()) { 
            changeState(Playing);
        }
        else { 
            changeState(Stopped);
        }
    }
};

void MainComponent::changeState (TransportState new_state) 
{ 
    if (state != new_state)
        {
            state = new_state;

            switch (state)
            {
                case Stopped:
                    stop_button.setEnabled (false);
                    play_button.setEnabled (true);
                    track.setEnabled(false);
                    transport_source.setPosition (0.0);
                    break;

                case Starting:
                    play_button.setEnabled (false);
                    transport_source.start();
                    break;

                case Playing:
                    stop_button.setEnabled (true);
                    track.setEnabled(true);
                    break;

                case Stopping:
                    transport_source.stop();
                    break;
            }
        }
};

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& buffer_to_fill) 
{ 
    //if (reader_source.get() == nullptr)
    if (!_reader.isSourcePresent())
    {
        buffer_to_fill.clearActiveBufferRegion();
        return;
    }

    transport_source.getNextAudioBlock (buffer_to_fill);

    _reader.getNextAudioBlock (buffer_to_fill);

    // filter
    float scale_gain_ls = std::pow(10, low_shelf_gain.getValue()/20); //Db to scale factor
    float scale_gain_hs = std::pow(10, high_shelf_gain.getValue()/20);
    filter_ls.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf (_samplerate, 500.0, 0.707, scale_gain_ls);
    filter_hs.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf (_samplerate, 2500.0, 0.707, scale_gain_hs);
    //juce::dsp::ProcessContextReplacing<double> context (buffer_to_fill.buffer);
    // juce::dsp::AudioBlock<float> block (*buffer_to_fill.buffer, (size_t) buffer_to_fill.startSample);
    juce::dsp::AudioBlock<float> block (buffer_to_fill.buffer->getArrayOfWritePointers(), buffer_to_fill.buffer->getNumChannels(), buffer_to_fill.buffer->getNumSamples());
    // block.clear();
    filter_ls.process(juce::dsp::ProcessContextReplacing<float> (block)); // it has two channels make sure to filter both
    filter_hs.process(juce::dsp::ProcessContextReplacing<float> (block));

    // filter_ls.processSamples(buffer_to_fill.buffer->getWritePointer(0), buffer_to_fill.numSamples);
    // filter_ls.processSamples(buffer_to_fill.buffer->getWritePointer(1), buffer_to_fill.numSamples);

    // filter_hs.processSamples(buffer_to_fill.buffer->getWritePointer(0), buffer_to_fill.numSamples);
    // filter_hs.processSamples(buffer_to_fill.buffer->getWritePointer(1), buffer_to_fill.numSamples);

    // juce::AudioBuffer  buff (Type *const *dataToReferTo, int numChannelsToUse, int startSample, int numSamples)
    // AudioSourceChannelInfo (AudioBuffer< float > &bufferToUse)

    // *buffer_to_fill.buffer->getWritePointer(0, buffer_to_fill.startSample) = 1.0;
    // *buffer_to_fill.buffer->getWritePointer(1, buffer_to_fill.startSample) = 1.0;

    // buffer_to_fill.buffer->clear();

    // transport_source.getNextAudioBlock (buffer_to_fill); //BUG: it plays the originl audio not the filtered for some reason.

    // avoid clipping

    if (buffer_to_fill.buffer->getNumChannels() > 0)
    {
        auto* channel0_data = buffer_to_fill.buffer->getReadPointer(0, buffer_to_fill.startSample);

        for (int i = 0; i<buffer_to_fill.numSamples; i++)
            pushNextSampleIntoFifo(channel0_data[i]);
    }
};

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate) 
{
    transport_source.prepareToPlay (samplesPerBlockExpected, sampleRate);
    _samplerate = static_cast<float>(sampleRate);
    filter_hs.prepare({ sampleRate, static_cast<unsigned int>(samplesPerBlockExpected), 2 });
    filter_ls.prepare({ sampleRate, static_cast<unsigned int>(samplesPerBlockExpected), 2 });
    //filter_ls.setCoefficients (juce::IIRCoefficients::makeLowShelf (_samplerate, 2000.0, 0.707, 10));
    //filter_hs.setCoefficients (juce::IIRCoefficients::makeHighShelf (_samplerate, 2000.0, 0.707, 1));
    //filter_hs.reset();
    //filter_ls.reset();

    _reader.prepareToPlay (samplesPerBlockExpected, sampleRate);
};

void MainComponent::releaseResources()
{
    transport_source.releaseResources();
    filter_hs.reset();
    filter_ls.reset();

    _reader.releaseResources();
};

void MainComponent::openButtonClicked()
{
    chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
                                                    juce::File {}, 
                                                    "*.wav");
    
    auto chooser_flag = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync (chooser_flag, 
                          [this] (const juce::FileChooser& fc) { 
                            auto file = fc.getResult();

                            if (file != juce::File{})
                            {
                                auto* reader = format_manager.createReaderFor (file);

                                if (reader != nullptr)
                                {
                                    auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
                                    transport_source.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
                                    play_button.setEnabled (true);
                                    reader_source.reset (newSource.release());
                                }
                            }
                        });
};

void MainComponent::playButtonClicked()
{
    changeState (Starting);
};

void MainComponent::stopButtonClicked()
{
    changeState (Stopping);
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
    if (nextFFTBlockReady)
    {
        drawNextFrameOfSpectrum();
        nextFFTBlockReady = false;
        repaint();
    }

    if (track.isEnabled() & !track.isMouseOverOrDragging())
    {
        int seconds = transport_source.getCurrentPosition();
        track.setValue( seconds / transport_source.getLengthInSeconds());
        track_position.setText(juce::String(int(seconds/60)) + "m " + juce::String(seconds % 60) + "s" , juce::NotificationType::sendNotification);
    }
};

void MainComponent::moveTrackSlider() 
{
    transport_source.setPosition(track.getValue() * transport_source.getLengthInSeconds());
    filter_ls.reset();
    filter_hs.reset();
};