#pragma once
#include <JuceHeader.h>

class WaveformDisplay : public Component, 
                        public ChangeListener
{
public:
    WaveformDisplay(AudioFormatManager& formatManagerToUse,
                    AudioThumbnailCache& cacheToUse);
    ~WaveformDisplay();

    void paint(Graphics&) override;
    void resized() override;
    
    // Mouse handling for seeking
    void mouseDown(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;

    void changeListenerCallback(ChangeBroadcaster* source) override;
    
    // Mouse wheel for zooming
    void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel) override;

    void loadURL(URL audioURL);

    /** Set the relative position of the playhead */
    void setPositionRelative(double pos);
    
    /** Set the accent color for the waveform display */
    void setColour(Colour newColour);
    
    /** Callback when user clicks/drags to seek */
    std::function<void(double)> onPositionChange;
    
    /** Set Zoom Factor (1.0 = full view, > 1.0 = zoomed in) */
    void setZoomFactor(double zoom);
    
    /** Set Loop Points for visualization */
    void setLoopPoints(double start, double end);

private:
    AudioThumbnail audioThumb;
    bool fileLoaded; 
    double position;
    double zoomFactor = 1.0;
    double loopStart = 0.0;
    double loopEnd = 0.0;
    Colour accentColor;
    bool isDragging = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};