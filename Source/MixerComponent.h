#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "DeckGUI.h" // For DJKnob definition

// Custom Fader LookAndFeel
class FaderLookAndFeel : public LookAndFeel_V4
{
public:
    FaderLookAndFeel()
    {
        setColour(Slider::thumbColourId, Colour(200, 200, 200));
        setColour(Slider::trackColourId, Colour(40, 40, 45));
        setColour(Slider::backgroundColourId, Colour(20, 20, 25));
    }
    
    void drawLinearSlider(Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const Slider::SliderStyle style, Slider& slider) override
    {
        auto bounds = Rectangle<float>((float)x, (float)y, (float)width, (float)height);
        bool isVertical = slider.isVertical();
        Colour accentColor = slider.findColour(Slider::thumbColourId);

        // Track Background
        g.setColour(slider.findColour(Slider::backgroundColourId));
        g.fillRoundedRectangle(bounds, 4.0f);

        // Track Slot & Tick Marks
        float trackWidth = 6.0f;
        Rectangle<float> trackBounds;
        if (isVertical)
            trackBounds = Rectangle<float>(bounds.getCentreX() - trackWidth/2, bounds.getY(), trackWidth, bounds.getHeight());
        else
            trackBounds = Rectangle<float>(bounds.getX() + 4, bounds.getCentreY() - trackWidth/2, bounds.getWidth() - 8, trackWidth);

        // Slot Shadow
        g.setColour(Colour(10, 10, 12));
        g.fillRoundedRectangle(trackBounds, trackWidth/2);
        g.setColour(Colour(35, 35, 40));
        g.drawRoundedRectangle(trackBounds.expanded(0.5f), trackWidth/2, 1.0f);

        // Tick Marks
        g.setColour(Colours::grey.withAlpha(0.4f));
        int numTicks = 9;
        for (int i = 0; i < numTicks; ++i)
        {
            float frac = (float)i / (numTicks - 1);
            if (isVertical)
            {
                float tickY = trackBounds.getY() + frac * trackBounds.getHeight();
                g.drawLine(trackBounds.getCentreX() - 10, tickY, trackBounds.getCentreX() - 6, tickY, 1.0f);
                g.drawLine(trackBounds.getCentreX() + 6, tickY, trackBounds.getCentreX() + 10, tickY, 1.0f);
            }
            else
            {
                float tickX = trackBounds.getX() + frac * trackBounds.getWidth();
                g.drawLine(tickX, trackBounds.getCentreY() - 10, tickX, trackBounds.getCentreY() - 6, 1.0f);
                g.drawLine(tickX, trackBounds.getCentreY() + 6, tickX, trackBounds.getCentreY() + 10, 1.0f);
            }
        }

        // Thumb Design
        float thumbWidth = isVertical ? 45.0f : 35.0f;
        float thumbHeight = isVertical ? 25.0f : 45.0f;
        
        Rectangle<float> thumbBounds;
        if (isVertical)
            thumbBounds = Rectangle<float>(bounds.getCentreX() - thumbWidth/2, sliderPos - thumbHeight/2, thumbWidth, thumbHeight);
        else
            thumbBounds = Rectangle<float>(sliderPos - thumbWidth/2, bounds.getCentreY() - thumbHeight/2, thumbWidth, thumbHeight);

        // Cap Shadow
        g.setColour(Colours::black.withAlpha(0.6f));
        g.fillRoundedRectangle(thumbBounds.translated(2, 3), 4.0f);

        // Main Cap Body
        ColourGradient capGrad(Colour(75, 75, 80), thumbBounds.getX(), thumbBounds.getY(),
                               Colour(35, 35, 40), thumbBounds.getRight(), thumbBounds.getBottom(), false);
        g.setGradientFill(capGrad);
        g.fillRoundedRectangle(thumbBounds, 4.0f);

        // Simulated Brushed Effect
        g.setColour(Colours::white.withAlpha(0.05f));
        for (int i = 2; i < thumbBounds.getWidth() - 2; i += 3)
            g.drawVerticalLine(thumbBounds.getX() + (float)i, thumbBounds.getY() + 2, thumbBounds.getBottom() - 2);

        // Cap Bevel
        g.setColour(Colours::white.withAlpha(0.1f));
        g.drawRoundedRectangle(thumbBounds.reduced(0.5f), 4.0f, 1.0f);
        g.setColour(Colours::black.withAlpha(0.4f));
        g.drawRoundedRectangle(thumbBounds, 4.0f, 1.5f);

        // Indicator (Plus Sign)
        Rectangle<float> hIndicator, vIndicator;
        float indicatorThickness = 2.0f;
        
        hIndicator = Rectangle<float>(thumbBounds.getX() + 4, thumbBounds.getCentreY() - indicatorThickness/2, thumbBounds.getWidth() - 8, indicatorThickness);
        vIndicator = Rectangle<float>(thumbBounds.getCentreX() - indicatorThickness/2, thumbBounds.getY() + 4, indicatorThickness, thumbBounds.getHeight() - 8);

        // Indicator Glow
        for (int i = 3; i >= 1; --i)
        {
            g.setColour(accentColor.withAlpha(0.12f / i));
            g.fillRoundedRectangle(hIndicator.expanded((float)i * 1.2f), 1.0f);
            g.fillRoundedRectangle(vIndicator.expanded((float)i * 1.2f), 1.0f);
        }
        
        g.setColour(accentColor);
        g.fillRoundedRectangle(hIndicator, 1.0f);
        g.fillRoundedRectangle(vIndicator, 1.0f);
        
        g.setColour(Colours::white.withAlpha(0.7f));
        g.fillRoundedRectangle(hIndicator.reduced(2.0f, 0.5f), 0.5f);
        g.fillRoundedRectangle(vIndicator.reduced(0.5f, 2.0f), 0.5f);
    }

    int getSliderThumbRadius(Slider& slider) override
    {
        return 15;
    }
};

class MixerComponent : public Component, public Timer
{
public:
    MixerComponent(DJAudioPlayer* player1, DJAudioPlayer* player2);
    ~MixerComponent() override;

    void paint(Graphics&) override;
    void resized() override;
    void timerCallback() override;

    // EQ Persistence methods (R4B)
    void loadEQSettings(int deck, const String& path);
    void saveEQSettings(int deck);
    File getEQSettingsFile();

private:
    DJAudioPlayer* player1;
    DJAudioPlayer* player2;
    
    // LookAndFeel
    FaderLookAndFeel faderLnF;

    // Deck 1 Controls
    DJKnob highEQ1;
    DJKnob midEQ1;
    DJKnob lowEQ1;
    Slider volSlider1;
    
    // Deck 2 Controls
    DJKnob highEQ2;
    DJKnob midEQ2;
    DJKnob lowEQ2;
    Slider volSlider2;
    
    // Crossfader
    Slider crossfader;

    // Current file paths for EQ persistence
    String currentFilePath1;
    String currentFilePath2;
    
    // Labels
    Label highLabel1, midLabel1, lowLabel1;
    Label highLabel2, midLabel2, lowLabel2;
    Label crossfaderLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerComponent)
};