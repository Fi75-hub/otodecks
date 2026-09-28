#include "DJAudioPlayer.h"
#include <cmath>

// SimpleEcho class
SimpleEcho::SimpleEcho()
{
    delaySamples = (int)(sampleRate * 0.375); // Default 375ms delay
}

// Prepare the echo effect
void SimpleEcho::prepare(double sr)
{
    sampleRate = sr;
    // Max 1 second delay buffer
    bufferSize = (int)(sampleRate * 1.0);
    delayBufferL.resize(bufferSize, 0.0f);
    delayBufferR.resize(bufferSize, 0.0f);
    writeIndex = 0;
    // Default 375ms delay
    delaySamples = (int)(sampleRate * 0.375);
}

// Set delay time in milliseconds
void SimpleEcho::setDelayTime(float ms)
{
    delaySamples = (int)(sampleRate * ms / 1000.0f);
    if (delaySamples >= bufferSize) delaySamples = bufferSize - 1;
    if (delaySamples < 1) delaySamples = 1;
}

// Set delay time in beats (for BPM sync)
void SimpleEcho::setDelayTimeBeats(double beats, double bpm)
{
    if (bpm <= 0.0) return;
    double ms = (60.0 / bpm) * 1000.0 * beats;
    setDelayTime((float)ms);
}

// Set feedback level
void SimpleEcho::setFeedback(float fb)
{
    feedback = jlimit(0.0f, 0.9f, fb);
}

// Set wet level
void SimpleEcho::setWetLevel(float wet)
{
    wetLevel = jlimit(0.0f, 1.0f, wet);
}

// Reset the echo effect
void SimpleEcho::reset()
{
    std::fill(delayBufferL.begin(), delayBufferL.end(), 0.0f);
    std::fill(delayBufferR.begin(), delayBufferR.end(), 0.0f);
    writeIndex = 0;
}

// Process stereo audio with echo effect
void SimpleEcho::processStereo(float& left, float& right)
{
    if (wetLevel <= 0.0f || bufferSize == 0) return;
    
    // Calculate read position
    int readIndex = writeIndex - delaySamples;
    if (readIndex < 0) readIndex += bufferSize;
    
    // Read delayed samples
    float delayedL = delayBufferL[readIndex];
    float delayedR = delayBufferR[readIndex];
    
    // Apply Low-Pass Filter (Simple One-Pole) to the feedback path
    // This makes the echoes sound progressively duller and more musical
    float filteredL = lastL + lpfAlpha * (delayedL - lastL);
    float filteredR = lastR + lpfAlpha * (delayedR - lastR);
    lastL = filteredL;
    lastR = filteredR;
    
    // Write to delay buffer (input + filtered feedback)
    delayBufferL[writeIndex] = left + filteredL * feedback;
    delayBufferR[writeIndex] = right + filteredR * feedback;
    
    // Advance write position
    writeIndex++;
    if (writeIndex >= bufferSize) writeIndex = 0;
    
    // Mix dry and wet
    left = left * (1.0f - wetLevel * 0.5f) + delayedL * wetLevel;
    right = right * (1.0f - wetLevel * 0.5f) + delayedR * wetLevel;
}

DJAudioPlayer::DJAudioPlayer(AudioFormatManager& _formatManager) 
: formatManager(_formatManager)
{

}

DJAudioPlayer::~DJAudioPlayer()
{

}

// Prepare the audio player
void DJAudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate) 
{
    currentSampleRate = sampleRate;
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    
    // Setup JUCE DSP ProcessSpec
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (uint32)samplesPerBlockExpected;
    spec.numChannels = 2;

    // Initialise EQ chains
    for (int ch = 0; ch < 2; ++ch)
    {
        eqChain[ch].prepare(spec);
        djFilter[ch].prepare(spec);
    }
    
    // Initialize reverb
    reverb.setSampleRate(sampleRate);
    
    // Initialize echo
    echo.prepare(sampleRate);
    
    // Initialize EQ filters
    updateFilters();
}

// Get the next audio block
void DJAudioPlayer::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    // Get audio from the resampled source
    resampleSource.getNextAudioBlock(bufferToFill);
    
    // Apply crossfader gain
    if (crossfaderGain < 1.0)
    {
        bufferToFill.buffer->applyGain(bufferToFill.startSample, bufferToFill.numSamples, (float)crossfaderGain);
    }

    // Handle looping
    if (isLooping && loopEnd > loopStart)
    {
        double currentPos = transportSource.getCurrentPosition();
        if (currentPos >= loopEnd)
        {
            transportSource.setPosition(loopStart);
        }
    }
    
    auto* buffer = bufferToFill.buffer;
    int numSamples = bufferToFill.numSamples;
    int startSample = bufferToFill.startSample;
    
    // Wrap the buffer for DSP processing
    juce::dsp::AudioBlock<float> block(*buffer);
    auto subBlock = block.getSubBlock((size_t)startSample, (size_t)numSamples);
    juce::dsp::ProcessContextReplacing<float> context(subBlock);

    // Apply 3-band EQ using JUCE DSP chain
    if (lowEQdB != 0.0 || midEQdB != 0.0 || highEQdB != 0.0)
    {
        for (int channel = 0; channel < (int)subBlock.getNumChannels() && channel < 2; ++channel)
        {
            auto channelBlock = subBlock.getSingleChannelBlock(channel);
            juce::dsp::ProcessContextReplacing<float> channelContext(channelBlock);
            eqChain[channel].process(channelContext);
        }
    }
    
    // Apply DJ Filter (Combined LPF/HPF) using JUCE DSP
    if (filterAmount < 0.49 || filterAmount > 0.51)
    {
        for (int channel = 0; channel < (int)subBlock.getNumChannels() && channel < 2; ++channel)
        {
            auto channelBlock = subBlock.getSingleChannelBlock(channel);
            juce::dsp::ProcessContextReplacing<float> channelContext(channelBlock);
            djFilter[channel].process(channelContext);
        }
    }
    
    // Apply reverb if enabled (JUCE Reverb)
    if (reverbLevel > 0.0 && buffer->getNumChannels() >= 2)
    {
        reverb.processStereo(buffer->getWritePointer(0, startSample), 
                             buffer->getWritePointer(1, startSample), 
                             numSamples);
    }
    
    // Apply echo if enabled
    if (echoLevel > 0.0 && buffer->getNumChannels() >= 2)
    {
        float* leftChannel = buffer->getWritePointer(0, startSample);
        float* rightChannel = buffer->getWritePointer(1, startSample);
        
        for (int i = 0; i < numSamples; ++i)
        {
            echo.processStereo(leftChannel[i], rightChannel[i]);
        }
    }
    
    // Calculate RMS levels for visualization
    if (buffer->getNumChannels() > 0)
    {
        rmsLevelL = buffer->getRMSLevel(0, startSample, numSamples);
        if (buffer->getNumChannels() > 1)
            rmsLevelR = buffer->getRMSLevel(1, startSample, numSamples);
        else
            rmsLevelR = rmsLevelL.load();
    }
    else
    {
        rmsLevelL = 0.0f;
        rmsLevelR = 0.0f;
    }
}

// Release resources
void DJAudioPlayer::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
}

void DJAudioPlayer::loadURL(URL audioURL)
{
    auto* reader = formatManager.createReaderFor(audioURL.createInputStream(false));
    if (reader != nullptr)
    {       
        std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true)); 
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);             
        readerSource.reset(newSource.release());          
        
        // Clear echo buffer for the new track
        echo.reset();
        
        // Analyze BPM for the new track
        analyzeTrackForBPM();
    }
}

// Set gain
void DJAudioPlayer::setGain(double gain)
{
    if (gain < 0 || gain > 1.0)
    {
        DBG("DJAudioPlayer::setGain gain should be between 0 and 1");
    }
    else {
        transportSource.setGain(gain);
    }
}

// Set speed
void DJAudioPlayer::setSpeed(double ratio)
{
    if (ratio < 0 || ratio > 100.0)
    {
        DBG("DJAudioPlayer::setSpeed ratio should be between 0 and 100");
    }
    else {
        resampleSource.setResamplingRatio(ratio);
        currentSpeed = ratio;
    }
}

// Set position
void DJAudioPlayer::setPosition(double posInSecs)
{
    transportSource.setPosition(posInSecs);
}

// Set position relative
void DJAudioPlayer::setPositionRelative(double pos)
{
    if (pos < 0 || pos > 1.0)
    {
        DBG("DJAudioPlayer::setPositionRelative pos should be between 0 and 1");
    }
    else {
        double posInSecs = transportSource.getLengthInSeconds() * pos;
        setPosition(posInSecs);
    }
}

// 3-band EQ setters

// Low EQ setter
void DJAudioPlayer::setLowEQ(double level)
{
    lowEQdB = (level - 0.5) * 48.0;
    updateFilters();
}

// Mid EQ setter
void DJAudioPlayer::setMidEQ(double level)
{
    midEQdB = (level - 0.5) * 48.0;
    updateFilters();
}

// High EQ setter
void DJAudioPlayer::setHighEQ(double level)
{
    highEQdB = (level - 0.5) * 48.0;
    updateFilters();
}

// Reverb setter
void DJAudioPlayer::setReverb(double level)
{
    reverbLevel = level;
    
    float scaledLevel = (float)level * 0.75f;
    
    reverbParameters.roomSize = scaledLevel * 0.9f;
    reverbParameters.damping = 0.5f;
    reverbParameters.wetLevel = scaledLevel;
    reverbParameters.dryLevel = 1.0f - scaledLevel * 0.5f;
    reverbParameters.width = 1.0f;
    
    reverb.setParameters(reverbParameters);
}

// Echo setter
void DJAudioPlayer::setEcho(double level)
{
    echoLevel = level;
    echo.setWetLevel((float)level);
    
    // Use BPM-synced delay if BPM is available, otherwise default to 375ms
    double bpm = getCurrentBPM();
    if (bpm > 0)
        echo.setDelayTimeBeats(0.5, bpm); // 0.5 beats = 1/2 beat echo
    else
        echo.setDelayTime(375.0f); 
}

// Loop setters

// Set loop points
void DJAudioPlayer::setLoopPoints(double startSeconds, double endSeconds)
{
    loopStart = startSeconds;
    loopEnd = endSeconds;
}

// Set looping
void DJAudioPlayer::setLooping(bool shouldLoop)
{
    isLooping = shouldLoop;
}

bool DJAudioPlayer::getLooping() const
{
    return isLooping;
}

std::pair<double, double> DJAudioPlayer::getLoopPoints() const
{
    return {loopStart, loopEnd};
}

// Filter setter
void DJAudioPlayer::setFilter(double amount)
{
    filterAmount = amount;
    
    for (int ch = 0; ch < 2; ++ch)
    {
        if (amount < 0.49) // Low Pass
        {
            // Range: 500Hz to ~18kHz
            double frequency = 500.0 * std::pow(40.0, amount * 2.0); 
            djFilter[ch].coefficients = FilterCoefficients::makeLowPass(currentSampleRate, frequency, 0.707);
        }
        else if (amount > 0.51) // High Pass
        {
            double norm = (amount - 0.5) * 2.0;
            // Range: 20Hz to 1200Hz
            double frequency = 20.0 * std::pow(60.0, norm);
            djFilter[ch].coefficients = FilterCoefficients::makeHighPass(currentSampleRate, frequency, 0.707);
        }
        else
        {
            djFilter[ch].reset(); // Disable
        }
    }
}

// Echo feedback setter
void DJAudioPlayer::setEchoFeedback(double feedback)
{
    echo.setFeedback((float)feedback);
}


void DJAudioPlayer::updateFilters()
{
    auto lowCoeffs = FilterCoefficients::makeLowShelf(currentSampleRate, 250.0f, 1.3f, (float)std::pow(10.0, lowEQdB / 20.0));
    auto midCoeffs = FilterCoefficients::makePeakFilter(currentSampleRate, 1000.0f, 1.3f, (float)std::pow(10.0, midEQdB / 20.0));
    auto highCoeffs = FilterCoefficients::makeHighShelf(currentSampleRate, 4000.0f, 1.3f, (float)std::pow(10.0, highEQdB / 20.0));

    for (int ch = 0; ch < 2; ++ch)
    {
        eqChain[ch].get<LowBandIndex>().coefficients = lowCoeffs;
        eqChain[ch].get<MidBandIndex>().coefficients = midCoeffs;
        eqChain[ch].get<HighBandIndex>().coefficients = highCoeffs;
    }
}

// Start
void DJAudioPlayer::start()
{
    transportSource.start();
}

// Stop
void DJAudioPlayer::stop()
{
    transportSource.stop();
}

// Is playing
bool DJAudioPlayer::isPlaying() const
{
    return transportSource.isPlaying();
}

// Get position relative
double DJAudioPlayer::getPositionRelative()
{
    return transportSource.getCurrentPosition() / transportSource.getLengthInSeconds();
}

// Crossfader gain control
void DJAudioPlayer::setCrossfaderGain(double gain)
{
    crossfaderGain = jlimit(0.0, 1.0, gain);
    // Apply crossfader gain on top of existing volume
    // The actual volume is set by setGain, crossfader modifies the output
}

// BPM detection methods
double DJAudioPlayer::getDetectedBPM() const
{
    return detectedBPM;
}

// Get current BPM
double DJAudioPlayer::getCurrentBPM() const
{
    if (detectedBPM <= 0.0) return 0.0;
    return detectedBPM * currentSpeed;
}

// Get current speed
double DJAudioPlayer::getCurrentSpeed() const
{
    return currentSpeed;
}

// Get track length in seconds
double DJAudioPlayer::getTrackLengthInSeconds() const
{
    return transportSource.getLengthInSeconds();
}

// Analyze track for BPM
void DJAudioPlayer::analyzeTrackForBPM()
{
    if (readerSource == nullptr)
    {
        detectedBPM = 0.0;
        return;
    }
    
    // Get the audio reader
    auto* reader = readerSource->getAudioFormatReader();
    if (reader == nullptr)
    {
        detectedBPM = 0.0;
        return;
    }
    
    // BPM detection using energy-based beat detection
    // We analyze a portion of the track (first 30 seconds max)
    double sampleRate = reader->sampleRate;
    int64 numSamples = reader->lengthInSamples;
    int64 samplesToAnalyze = jmin(numSamples, (int64)(sampleRate * 30.0));
    
    if (samplesToAnalyze < sampleRate * 2) // Need at least 2 seconds
    {
        detectedBPM = 0.0;
        return;
    }
    
    // Read audio data
    AudioBuffer<float> buffer(1, (int)samplesToAnalyze);
    reader->read(&buffer, 0, (int)samplesToAnalyze, 0, true, false);
    
    // Calculate energy in windows (for beat detection)
    int windowSize = (int)(sampleRate * 0.02); // 20ms windows
    int hopSize = windowSize / 2;
    int numWindows = ((int)samplesToAnalyze - windowSize) / hopSize;
    
    if (numWindows < 10)
    {
        detectedBPM = 0.0;
        return;
    }
    
    std::vector<float> energy(numWindows);
    const float* samples = buffer.getReadPointer(0);
    
    // Calculate energy for each window
    for (int w = 0; w < numWindows; ++w)
    {
        int start = w * hopSize;
        float sum = 0.0f;
        for (int i = 0; i < windowSize; ++i)
        {
            float sample = samples[start + i];
            sum += sample * sample;
        }
        energy[w] = sum / windowSize;
    }
    
    // Calculate average energy
    float avgEnergy = 0.0f;
    for (int w = 0; w < numWindows; ++w)
        avgEnergy += energy[w];
    avgEnergy /= numWindows;
    
    // Detect beats (energy peaks above threshold)
    std::vector<int> beatPositions;
    float threshold = avgEnergy * 1.5f;
    int minBeatDistance = (int)(sampleRate * 0.25 / hopSize); // Min 0.25s between beats (240 BPM max)
    
    int lastBeat = -minBeatDistance;
    for (int w = 1; w < numWindows - 1; ++w)
    {
        // Peak detection: current > previous and current > next and above threshold
        if (energy[w] > energy[w-1] && energy[w] > energy[w+1] && 
            energy[w] > threshold && (w - lastBeat) >= minBeatDistance)
        {
            beatPositions.push_back(w);
            lastBeat = w;
        }
    }
    
    // Calculate BPM from beat intervals
    if (beatPositions.size() < 4)
    {
        detectedBPM = 120.0; // Default fallback
        return;
    }
    
    // Calculate average interval between beats
    double totalInterval = 0.0;
    for (size_t i = 1; i < beatPositions.size(); ++i)
    {
        int interval = beatPositions[i] - beatPositions[i-1];
        totalInterval += interval;
    }
    double avgInterval = totalInterval / (beatPositions.size() - 1);
    
    // Convert to BPM
    double secondsPerBeat = (avgInterval * hopSize) / sampleRate;
    double bpm = 60.0 / secondsPerBeat;
    
    // Normalize BPM to reasonable range (60-180)
    while (bpm < 60.0) bpm *= 2.0;
    while (bpm > 180.0) bpm /= 2.0;
    
    detectedBPM = bpm;
    DBG("Detected BPM: " << detectedBPM);
    
    // Generate beat grid for the whole track
    // Assume first beat found is consistent
    if (!beatPositions.empty())
    {
        beatTimes.clear();
        double firstBeatTime = (beatPositions[0] * hopSize) / sampleRate;
        double secondsPerBeat = 60.0 / detectedBPM;
        double trackLength = transportSource.getLengthInSeconds();
        
        // Populate beat grid
        for (double t = firstBeatTime; t < trackLength; t += secondsPerBeat)
        {
            beatTimes.push_back(t);
        }
    }
}

// Get RMS level
float DJAudioPlayer::getRMSLevel(int channel) const
{
    if (channel == 0) return rmsLevelL;
    if (channel == 1) return rmsLevelR;
    return 0.0f;
}

// Get nearest beat
double DJAudioPlayer::getNearestBeat(double positionSeconds)
{
    if (beatTimes.empty()) return positionSeconds; // No grid, return input
    
    double minDiff = 1e9;
    double nearest = positionSeconds;
    
    // Find closest beat
    for (double beat : beatTimes)
    {
        double diff = std::abs(beat - positionSeconds);
        if (diff < minDiff)
        {
            minDiff = diff;
            nearest = beat;
        }
        else if (beat > positionSeconds)
        {
            // Sorted, so once diff increases, we've passed the closest
            break;
        }
    }
    
    return nearest;
}