#pragma once

#include <JuceHeader.h>

// Simple delay-based echo effect
class SimpleEcho
{
public:
    SimpleEcho();
    void prepare(double sampleRate);
    void setDelayTime(float ms); // Delay time in milliseconds
    void setDelayTimeBeats(double beats, double bpm); // BPM-synced delay
    void setFeedback(float fb); // 0.0 to 0.9
    void setWetLevel(float wet); // 0.0 to 1.0
    void processStereo(float& left, float& right);
    void reset();
    
private:
    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;
    int bufferSize = 0;
    int writeIndex = 0;
    int delaySamples = 0;
    float feedback = 0.5f;
    float wetLevel = 0.0f;
    double sampleRate = 44100.0;
    
    // Feedback LPF state
    float lastL = 0.0f, lastR = 0.0f;
    float lpfAlpha = 0.7f; // Fixed smooth LPF for feedback
};

// DJ Audio Player class
class DJAudioPlayer : public AudioSource {
  public:

    DJAudioPlayer(AudioFormatManager& _formatManager);
    ~DJAudioPlayer(); // Destructor

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void loadURL(URL audioURL);
    void setGain(double gain);
    void setSpeed(double ratio);
    void setPosition(double posInSecs);
    void setPositionRelative(double pos);
    
    // 3-band EQ controls
    void setLowEQ(double level);
    void setMidEQ(double level);
    void setHighEQ(double level);
    
    // Reverb control (0.0 to 1.0)
    void setReverb(double level);
    
    // Echo control (0.0 to 1.0)
    void setEcho(double level);
    void setEchoFeedback(double feedback); // 0.0 to 1.0
    
    // DJ Filter Control
    void setFilter(double amount); // 0.0 = LPF, 0.5 = Off, 1.0 = HPF

    // Looping controls
    void setLoopPoints(double startSeconds, double endSeconds);
    void setLooping(bool shouldLoop);
    bool getLooping() const;
    std::pair<double, double> getLoopPoints() const;

    void start();
    void stop();
    bool isPlaying() const;

    double getPositionRelative();
    
    // Crossfader gain control (0.0 to 1.0)
    void setCrossfaderGain(double gain);
    
    // BPM detection and speed control
    double getDetectedBPM() const;
    double getCurrentBPM() const;
    double getCurrentSpeed() const;
    double getTrackLengthInSeconds() const;
    void analyzeTrackForBPM();
    
    // Beat Grid
    std::vector<double> beatTimes;
    double getNearestBeat(double positionSeconds);


    // Audio level for visualization
    float getRMSLevel(int channel) const;

private:
    AudioFormatManager& formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource; 
    ResamplingAudioSource resampleSource{&transportSource, false, 2};
    
    // Crossfader gain
    double crossfaderGain = 1.0;
    
    // Audio levels for visualization
    std::atomic<float> rmsLevelL { 0.0f };
    std::atomic<float> rmsLevelR { 0.0f };

    
    // BPM detection
    double detectedBPM = 0.0;
    double currentSpeed = 1.0;
    
    // EQ processing
    double currentSampleRate = 44100.0;
    
    // Processor chain for EQ: LowShelf -> Peaking (Mid) -> HighShelf
    using FilterType = juce::dsp::IIR::Filter<float>;
    using FilterCoefficients = juce::dsp::IIR::Coefficients<float>;
    
    juce::dsp::ProcessorChain<FilterType, FilterType, FilterType> eqChain[2];
    
    enum {
        LowBandIndex = 0,
        MidBandIndex = 1,
        HighBandIndex = 2
    };

    double lowEQdB = 0.0;
    double midEQdB = 0.0;
    double highEQdB = 0.0;
    
    // DJ Combo Filter
    juce::dsp::IIR::Filter<float> djFilter[2];
    double filterAmount = 0.5; // Starts neutral
    
    // Reverb processing
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParameters;
    double reverbLevel = 0.0;
    
    // Echo processing
    SimpleEcho echo;
    double echoLevel = 0.0;

    // Looping
    double loopStart = 0.0;
    double loopEnd = 0.0;
    bool isLooping = false;
    
    void updateFilters();
};