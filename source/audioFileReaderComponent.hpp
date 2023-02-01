#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <memory>

/* TransportState is an emum for keeping track of the state of the player */
enum TransportState { 
    Stopped,
    Starting,
    Playing,
    Stopping
};

/* 
    AudioFileReaderComponent is a GUI compoonent for selecting and audio file, playing or stopping the audio,
    keeping track of playing time and sliding through the track.
*/
class AudioFileReaderComponent : public juce::Component,
                                  public juce::ChangeListener,
                                  public juce::Timer
{
    juce::TextButton _open_button; // button for opening a WAV file
    juce::TextButton _play_button;
    juce::TextButton _stop_button;

    std::unique_ptr<juce::FileChooser> _chooser;

    juce::AudioFormatManager _format_manager;
    std::unique_ptr<juce::AudioFormatReaderSource> _reader_source;
    juce::AudioTransportSource _transport_source;
    // state variable of the player
    TransportState _state;

    // slider for track and change audio track time
    juce::Slider _track;
    // label to diaplay playing time in seconds
    juce::Label _track_position;

    /* choose WAV file to read and play */
    void _openButtonClicked();

    /* start to play loaded WAV file */
    void _playButtonClicked();

    /* stop audio */
    void _stopButtonClicked();

    /* change playing position of track according to new _track slider value */
    void _moveTrackSlider();
    
    /* change internal variable state of class according to new state */
    void _changeState (TransportState new_state);

    public:

    /* AudioFileReaderComponent constructor */
    AudioFileReaderComponent();

    /* AudioFileReaderComponent destructor */
    ~AudioFileReaderComponent() = default;

    /* place internal compoment on GUI */
    void resized() override;

    /* listener called when _transport_source changes playing state (playing, stopping) */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    /* method called every 30 times per second */
    void timerCallback() override;

    /* process buffer_to_fill */
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& buffer_to_fill);

    /* set necessary value before palying */
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);

    /* release resouces when stop playing*/
    void releaseResources();
 
    /* return true is a WAV file is ready to be played. return false otherwise. */
    bool isSourcePresent();
};