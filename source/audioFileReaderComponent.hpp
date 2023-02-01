#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <memory>
#include "utils.hpp"

#ifndef AUDIOFILEREADERCOMPONENT_H
#define AUDIOFILEREADERCOMPONENT_H

enum TransportState { 
    Stopped,
    Starting,
    Playing,
    Stopping
};

class AudioFileReaderComponent : public juce::Component,
                                  public juce::ChangeListener,
                                  public juce::Timer
{
    juce::TextButton _open_button;
    juce::TextButton _play_button;
    juce::TextButton _stop_button;

    std::unique_ptr<juce::FileChooser> _chooser;

    juce::AudioFormatManager _format_manager;
    std::unique_ptr<juce::AudioFormatReaderSource> _reader_source;
    juce::AudioTransportSource _transport_source;

    TransportState _state;

    juce::Slider _track;
    juce::Label _track_position;

    void _openButtonClicked();
    void _playButtonClicked();
    void _stopButtonClicked();

    void _moveTrackSlider();

    void _changeState (TransportState new_state);

    public:
    AudioFileReaderComponent();

    ~AudioFileReaderComponent() = default;

    void resized() override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void timerCallback() override;

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& buffer_to_fill);

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);

    void releaseResources();

    bool isSourcePresent();
};

#endif