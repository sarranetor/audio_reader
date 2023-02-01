#include "audioFileReaderComponent.hpp"

AudioFileReaderComponent::AudioFileReaderComponent()
    : _state (Stopped)
{
    addAndMakeVisible (_open_button);
    _open_button.setButtonText ("Open...");
    _open_button.onClick = [this] { _openButtonClicked(); };

    addAndMakeVisible (_play_button);
    _play_button.setButtonText ("Play");
    _play_button.onClick = [this] { _playButtonClicked(); };
    _play_button.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    _play_button.setEnabled(false);

    addAndMakeVisible (_stop_button);
    _stop_button.setButtonText ("Stop");
    _stop_button.onClick = [this] { _stopButtonClicked(); };
    _stop_button.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    _stop_button.setEnabled(false);

    // to do add mp3 ..
    _format_manager.registerBasicFormats(); // initialize manager for at least WAV and AIFF formats

    addAndMakeVisible (_track);
    _track.setSliderStyle(juce::Slider::LinearHorizontal);
    _track.setEnabled(false);
    _track.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    _track.setRange(0, 1, 0.01);
    _track.onDragEnd = [this] { _moveTrackSlider(); };

    addAndMakeVisible (_track_position);
    _track_position.setEnabled(true);
    _track_position.setColour (juce::Label::backgroundColourId, juce::Colours::black);
    _track_position.setColour (juce::Label::textColourId, juce::Colours::white);
    _track_position.setJustificationType (juce::Justification::centred);

    _transport_source.addChangeListener (this); // add as a listener so we can respond to changes in transpost_source state

    startTimerHz (30);
};

void AudioFileReaderComponent::resized() 
{
    auto area = getLocalBounds();

    int buttons_height = 30;
    _open_button.setBounds(area.removeFromTop (buttons_height));
    _play_button.setBounds(area.removeFromTop (buttons_height));
    _stop_button.setBounds(area.removeFromTop (buttons_height));

    _track.setBounds(area.removeFromTop (buttons_height));
    _track_position.setBounds(area.removeFromTop (buttons_height));

};

void AudioFileReaderComponent::changeListenerCallback(juce::ChangeBroadcaster* source) 
{
    if (source == &_transport_source) { 
        if (_transport_source.isPlaying()) { 
            _changeState(Playing);
        }
        else { 
            _changeState(Stopped);
        }
    }
};

void AudioFileReaderComponent::_changeState (TransportState new_state)
{
    if (_state != new_state)
        {
            _state = new_state;

            switch (_state)
            {
                case Stopped:
                    _stop_button.setEnabled (false);
                    _play_button.setEnabled (true);
                    _track.setEnabled(false);
                    _transport_source.setPosition (0.0);
                    break;

                case Starting:
                    _play_button.setEnabled (false);
                    _transport_source.start();
                    break;

                case Playing:
                    _stop_button.setEnabled (true);
                    _track.setEnabled(true);
                    break;

                case Stopping:
                    _transport_source.stop();
                    break;
            }
        }
};

void AudioFileReaderComponent::_openButtonClicked() 
{
    _chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
                                                    juce::File {}, 
                                                    "*.wav");
    
    auto chooser_flag = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    _chooser->launchAsync (chooser_flag, 
                          [this] (const juce::FileChooser& fc) { 
                            auto file = fc.getResult();

                            if (file != juce::File{})
                            {
                                auto* reader = _format_manager.createReaderFor (file);

                                if (reader != nullptr)
                                {
                                    auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
                                    _transport_source.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
                                    _play_button.setEnabled (true);
                                    _reader_source.reset (newSource.release());
                                }
                            }
                        });
};

void AudioFileReaderComponent::_playButtonClicked() 
{
    _changeState (Starting);
};

void AudioFileReaderComponent::_stopButtonClicked() 
{
    _changeState (Stopping);
};

void AudioFileReaderComponent::_moveTrackSlider() 
{
    _transport_source.setPosition(_track.getValue() * _transport_source.getLengthInSeconds());
};

void AudioFileReaderComponent::timerCallback() 
{
    if (_track.isEnabled() & !_track.isMouseOverOrDragging())
    {
        int seconds = _transport_source.getCurrentPosition();
        _track.setValue( seconds / _transport_source.getLengthInSeconds());
        _track_position.setText(juce::String(int(seconds/60)) + "m " + juce::String(seconds % 60) + "s" , juce::NotificationType::sendNotification);
    }
};

void AudioFileReaderComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& buffer_to_fill)
{
    _transport_source.getNextAudioBlock (buffer_to_fill);
};

void AudioFileReaderComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    _transport_source.prepareToPlay (samplesPerBlockExpected, sampleRate);
};

void AudioFileReaderComponent::releaseResources()
{
    _transport_source.releaseResources();
};

bool AudioFileReaderComponent::isSourcePresent()
{
    return (_reader_source.get() != nullptr);
};