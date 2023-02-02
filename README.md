# Title: Audio Reader Project

# Project Description

Audio Reader is a Gui app built with the JUCE framework for Audio development. A WAV file can be loaded, played and we can move around the track thanks to a slider.
Moreover a few DSP blocks allow to apply a gain and change the tonal balance thanks to two shelving filter, one for low frequencies and one for high frequencies.  

## Audio Reader App GUI Description 

![reader_numbered](figs/reader_numbered.png)

1. Button for choosing a WAV file to read and play
2. Once a WAV file is chosen it can be played pressing this button
3. Stop the reproduction of audio and bringing back the audio file to initial position
4. This slider moves according to reproiction time. Moving slider position is possible to move time position on audio track as well. Acrive only when audio is playing.
5. Label to display reproduction time of the track.
6. When the audio is playing FFT is real-time is computed and shown.
7. Slider to change audio vulume applying a gain in Db.
8. Low Shelving filter with cut frequency of 500Hz. It can boost of cut the low end and part of the mid range of the audio signal.
9. High Shelving filter with cut frequency of 2500Hz. It can boost or cut the high end of audio signal. 

# How to Install and Run the Projectt

A ready to use executable is present in: 

    cmake-build-dir/AudioReader_artefacts/Release/

For building the source code Cmake [2] is used. Moreover the JUCE audio framework [1] [3] is a dependency.
The project can be built in the following way:

    cd ../audio_reader
    git clone --recurse-submodules https://github.com/juce-framework/JUCE.git
    git submodule update --init --recursive
    cmake -B cmake-build-dir
    cmake --build cmake-build-dir --config release
    ./cmake-build-dir/AudioReader_artefacts/Release/audio_reader.exe

# References

[1] https://github.com/juce-framework/JUCE
[2] https://github.com/juce-framework/JUCE/README cmake section
[3] https://juce.com/learn/tutorials/