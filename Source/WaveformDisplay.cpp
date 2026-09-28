#include <JuceHeader.h>
#include "WaveformDisplay.h"

// Constructor
WaveformDisplay::WaveformDisplay(AudioFormatManager& formatManagerToUse,
                                 AudioThumbnailCache& cacheToUse) :
                                 audioThumb(1000, formatManagerToUse, cacheToUse), 
                                 fileLoaded(false), 
                                 position(0),
                                 accentColor(Colours::cyan)
{
    audioThumb.addChangeListener(this);
}

// Destructor
WaveformDisplay::~WaveformDisplay()
{
}

// Paint method
void WaveformDisplay::paint(Graphics& g)
{
    auto bounds = getLocalBounds().toFloat(); // Get the bounds of the component
    
    // Dark gradient background
    ColourGradient bgGradient(Colour(25, 25, 30), 0, 0,
                              Colour(15, 15, 20), 0, bounds.getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(bounds, 4);
    
    // Inner border
    g.setColour(Colour(40, 40, 45));
    g.drawRoundedRectangle(bounds.reduced(1), 4, 1);

    if (fileLoaded && audioThumb.getTotalLength() > 0.0)
    {
        // Draw waveform with gradient effect
        // Calculate visible range based on zoom
        double totalLength = audioThumb.getTotalLength();
        double visibleLength = totalLength / zoomFactor;
        double currentSeconds = position * totalLength;
        double startTime = currentSeconds - (visibleLength / 2.0);
        double endTime = startTime + visibleLength;
        
        // Keep the waveform in view
        if (startTime < 0) 
        {
            startTime = 0;
            endTime = visibleLength;
        }

        if (endTime > totalLength)
        {
            endTime = totalLength;
            startTime = totalLength - visibleLength;
            if (startTime < 0) startTime = 0;
        }

        // Draw loop region
        if (loopEnd > loopStart)
        {
            float loopStartX = (float)((loopStart - startTime) / visibleLength * getWidth());
            float loopEndX = (float)((loopEnd - startTime) / visibleLength * getWidth());
            float loopWidth = loopEndX - loopStartX;
            
            if (loopWidth > 0 && loopEndX > 0 && loopStartX < getWidth())
            {
                g.setColour(Colours::yellow.withAlpha(0.2f));
                g.fillRect(loopStartX, 0.0f, loopWidth, (float)getHeight());
                g.setColour(Colours::yellow.withAlpha(0.6f));
                g.drawLine(loopStartX, 0.0f, loopStartX, (float)getHeight(), 1.0f);
                g.drawLine(loopEndX, 0.0f, loopEndX, (float)getHeight(), 1.0f);
            }
        }

        g.setColour(accentColor.darker(0.5f));
        audioThumb.drawChannel(g, getLocalBounds().reduced(2), 
            startTime, 
            endTime, 
            0, 
            0.8f);
        
        g.setColour(accentColor);
        audioThumb.drawChannel(g, 
            getLocalBounds().reduced(2), startTime, 
            endTime, 
            0, 
            1.0f);
        
        // Center line
        g.setColour(Colour(50, 50, 55));
        g.drawLine(0, getHeight() / 2.0f, (float)getWidth(), getHeight() / 2.0f, 1.0f);
        
        // Adjust playhead position based on zoom
        float playheadFraction = (float)((currentSeconds - startTime) / visibleLength);
        float playheadX = playheadFraction * getWidth();
        
        playheadX = jmax(0.0f, jmin((float)getWidth(), playheadX));
        
        g.setColour(Colours::white.withAlpha(0.1f));
        g.fillRect(0.0f, 0.0f, playheadX, (float)getHeight());
        
        // Playhead glow
        for (int i = 3; i >= 0; --i)
        {
            g.setColour(accentColor.withAlpha(0.1f - i * 0.02f));
            g.fillRect(playheadX - i * 2 - 1, 0.0f, (float)(i * 4 + 3), (float)getHeight());
        }
        
        g.setColour(Colours::white);
        g.drawLine(playheadX, 0, playheadX, (float)getHeight(), 2.0f);
        
        Path topMarker;
        topMarker.addTriangle(playheadX - 6, 0, playheadX + 6, 0, playheadX, 10);
        g.setColour(isDragging ? Colours::white : accentColor);
        g.fillPath(topMarker);
        
        Path bottomMarker;
        bottomMarker.addTriangle(playheadX - 6, (float)getHeight(), 
                                  playheadX + 6, (float)getHeight(), 
                                  playheadX, (float)getHeight() - 10);
        g.fillPath(bottomMarker);
        
        // Time markers
        g.setColour(Colour(60, 60, 65));
        int numMarkers = 8;
        for (int i = 1; i < numMarkers; ++i)
        {
            float x = (getWidth() / (float)numMarkers) * i;
            g.drawLine(x, 0, x, 5, 1.0f);
            g.drawLine(x, (float)getHeight() - 5, x, (float)getHeight(), 1.0f);
        }
        
        setMouseCursor(MouseCursor::PointingHandCursor);
    }
    else 
    {
        // Default design when no file loaded
        auto bounds = getLocalBounds().toFloat();
        
        if (bounds.getWidth() > 50 && bounds.getHeight() > 20)
        {
            float centerY = bounds.getCentreY();
            
            g.setColour(accentColor.withAlpha(0.15f));
            for (int i = 0; i < 5; ++i)
            {
                float lineY = centerY + (i - 2) * 8.0f;
                if (lineY > 0 && lineY < bounds.getHeight())
                    g.drawLine(bounds.getX() + 20, lineY, bounds.getRight() - 20, lineY, 0.5f);
            }
            
            // Draw decorative waveform bars in center
            g.setColour(accentColor.withAlpha(0.3f));
            float barWidth = 3.0f;
            float barSpacing = 6.0f;
            int numBars = 15;
            float startX = bounds.getCentreX() - (numBars * barSpacing) / 2.0f;
            
            for (int i = 0; i < numBars; ++i)
            {
                // Create a wave-like pattern with varying heights
                float height = 10.0f + 15.0f * std::sin((float)i * 0.5f + 1.0f);
                float x = startX + i * barSpacing;
                float y = centerY - height / 2.0f;
                if (height > 0 && y > 0)
                    g.fillRoundedRectangle(x, y, barWidth, height, 1.5f);
            }
        }
        
        setMouseCursor(MouseCursor::NormalCursor);
    }
}

void WaveformDisplay::resized()
{
}

void WaveformDisplay::mouseDown(const MouseEvent& event)
{
    if (fileLoaded && audioThumb.getTotalLength() > 0.0)
    {
        isDragging = true;
        
        double totalLength = audioThumb.getTotalLength();
        double visibleLength = totalLength / zoomFactor;
        double currentSeconds = position * totalLength;
        double startTime = currentSeconds - (visibleLength / 2.0);
        
        if (startTime < 0) startTime = 0;
        if (startTime + visibleLength > totalLength) startTime = totalLength - visibleLength;
        if (startTime < 0) startTime = 0;
        
        double clickTime = startTime + (event.position.x / getWidth()) * visibleLength;
        double newPosition = clickTime / totalLength;
        
        newPosition = jmax(0.0, jmin(1.0, newPosition));
        
        position = newPosition;
        
        if (onPositionChange)
            onPositionChange(newPosition);
        
        repaint();
    }
}

void WaveformDisplay::mouseDrag(const MouseEvent& event)
{
    if (fileLoaded && isDragging && audioThumb.getTotalLength() > 0.0)
    {
        // Calculate position from drag location respecting zoom
        double totalLength = audioThumb.getTotalLength();
        double visibleLength = totalLength / zoomFactor;
        
        double currentSeconds = position * totalLength;
        double startTime = currentSeconds - (visibleLength / 2.0);
        
        if (startTime < 0) startTime = 0;
        if (startTime + visibleLength > totalLength) startTime = totalLength - visibleLength;
        if (startTime < 0) startTime = 0;
        
        double clickTime = startTime + (event.position.x / getWidth()) * visibleLength;
        double newPosition = clickTime / totalLength;
        
        newPosition = jmax(0.0, jmin(1.0, newPosition));
        
        position = newPosition;
        
        if (onPositionChange)
            onPositionChange(newPosition);
        
        repaint();
    }
}

// Mouse up
void WaveformDisplay::mouseUp(const MouseEvent& event)
{
    isDragging = false;
    repaint();
}

void WaveformDisplay::loadURL(URL audioURL)
{
    audioThumb.clear();
    fileLoaded = audioThumb.setSource(new URLInputSource(audioURL)); // Load URL
    if (fileLoaded && audioThumb.getTotalLength() > 0.0)
    {
        DBG("Waveform: File loaded successfully");
        repaint();
    }
    else 
    {
        DBG("Waveform: Failed to load file");
    }
}

void WaveformDisplay::changeListenerCallback(ChangeBroadcaster* source)
{
    DBG("Waveform: Change received, repainting");
    repaint();
}

void WaveformDisplay::setPositionRelative(double pos)
{
    if (!isDragging && pos != position && !std::isnan(pos)) // If not dragging, position is not the same, and position is not NaN
    {
        position = pos;
        repaint();
    }
}

// Set colour
void WaveformDisplay::setColour(Colour newColour)
{
    accentColor = newColour;
    repaint();
}

void WaveformDisplay::setZoomFactor(double zoom)
{
    zoomFactor = jmax(1.0, jmin(50.0, zoom)); // Zoom factor between 1 and 50
    repaint();
}

// Set loop points
void WaveformDisplay::setLoopPoints(double start, double end)
{
    loopStart = start;
    loopEnd = end;
    repaint();
}

void WaveformDisplay::mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel)
{
    if (fileLoaded && audioThumb.getTotalLength() > 0.0)
    {
        // Adjust zoom factor based on wheel delta
        double newZoom = zoomFactor + wheel.deltaY * 5.0; // Sensitivity adjust
        setZoomFactor(newZoom);
    }
}