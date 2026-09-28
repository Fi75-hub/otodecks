#include "DJAudioPlayer.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

void writeTone(const File& file)
{
    WavAudioFormat format;
    std::unique_ptr<OutputStream> stream = file.createOutputStream();
    require(stream != nullptr, "Could not create test audio");
    auto writer = format.createWriterFor(stream, AudioFormatWriterOptions{}
        .withSampleRate(44100.0).withNumChannels(2).withBitsPerSample(16));
    require(writer != nullptr, "Could not create WAV writer");
    AudioBuffer<float> tone(2, 44100);
    for (int sample = 0; sample < tone.getNumSamples(); ++sample)
    {
        const auto value = 0.2f * std::sin(2.0f * MathConstants<float>::pi * 440.0f * sample / 44100.0f);
        tone.setSample(0, sample, value);
        tone.setSample(1, sample, value);
    }
    require(writer->writeFromAudioSampleBuffer(tone, 0, tone.getNumSamples()), "Could not write test WAV");
}

void testEchoReset()
{
    SimpleEcho echo;
    echo.prepare(1000.0);
    echo.setDelayTime(2.0f);
    echo.setWetLevel(1.0f);
    float left = 1.0f, right = 1.0f;
    echo.processStereo(left, right);
    for (int i = 0; i < 8; ++i)
    {
        left = right = 0.0f;
        echo.processStereo(left, right);
    }
    echo.reset();
    for (int i = 0; i < 100; ++i)
    {
        left = right = 0.0f;
        echo.processStereo(left, right);
        require(left == 0.0f && right == 0.0f, "Echo reset retained audio from the previous track");
    }
}

void testPlayer(const File& toneFile)
{
    AudioFormatManager formats;
    formats.registerBasicFormats();
    DJAudioPlayer player(formats);
    require(player.getPositionRelative() == 0.0, "An empty deck should have a finite zero position");
    player.prepareToPlay(512, 44100.0);
    require(player.loadURL(URL(toneFile)), "Valid WAV did not load");
    require(std::abs(player.getTrackLengthInSeconds() - 1.0) < 0.001, "WAV duration is incorrect");
    require(!player.loadURL(URL(toneFile.getSiblingFile("missing.wav"))), "Missing audio was accepted");
    require(std::abs(player.getTrackLengthInSeconds() - 1.0) < 0.001, "Failed load replaced the current track");

    player.setSpeed(0.0);
    player.setSpeed(-1.0);
    player.setSpeed(std::numeric_limits<double>::quiet_NaN());
    require(player.getCurrentSpeed() == 1.0, "Invalid playback speed changed the resampling ratio");
    player.setSpeed(1.5);
    require(player.getCurrentSpeed() == 1.5, "Valid playback speed was rejected");
    player.setSpeed(1.0);

    player.setPositionRelative(0.25);
    require(std::abs(player.getPositionRelative() - 0.25) < 0.001, "Seeking to a cue failed");
    player.setPositionRelative(0.0);
    player.start();
    AudioBuffer<float> output(2, 512);
    AudioSourceChannelInfo block(&output, 0, 512);
    for (int i = 0; i < 4; ++i)
        player.getNextAudioBlock(block);
    require(output.getMagnitude(0, 512) > 0.01f, "Playback produced no audio samples");
    require(player.getRMSLevel(0) > 0.0f, "Audio level meter did not update");

    player.setReverb(0.8);
    player.setEcho(0.8);
    for (int i = 0; i < 45; ++i)
        player.getNextAudioBlock(block);
    player.setCrossfaderGain(0.0);
    player.getNextAudioBlock(block);
    require(output.getMagnitude(0, 512) == 0.0f, "Effect tails bypassed the muted crossfader");

    player.beatTimes = { 0.0, 0.5 };
    player.setLoopPoints(0.1, 0.8);
    player.setLooping(true);
    require(player.loadURL(URL(toneFile)), "Reloading a WAV failed");
    require(player.beatTimes.empty(), "A short new track retained the previous beat grid");
    require(!player.getLooping(), "Loading a track retained the previous loop state");
    require(player.getLoopPoints().second == 0.0, "Loading a track retained the previous loop range");
    player.releaseResources();
}
}

int main()
{
    ScopedJuceInitialiser_GUI juce;
    const auto toneFile = File::getSpecialLocation(File::tempDirectory)
        .getNonexistentChildFile("otodecks-audio-test", ".wav", false);
    try
    {
        writeTone(toneFile);
        testEchoReset();
        testPlayer(toneFile);
        toneFile.deleteFile();
        std::cout << "Audio regression checks passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        toneFile.deleteFile();
        std::cerr << error.what() << '\n';
        return 1;
    }
}
