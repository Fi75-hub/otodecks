#include <JuceHeader.h>
#include "DeckGUI.h"

// Jog Wheel class
JogWheel::JogWheel()
{
    startTimerHz(60);
}

// Destructor
JogWheel::~JogWheel()
{
    stopTimer();
}

// Paint method
void JogWheel::paint(Graphics& g)
{
    auto bounds = getLocalBounds().toFloat(); // Get the bounds of the component
    auto center = bounds.getCentre(); // Get the center of the component
    
    float maxExpansion = 20.0f; // Maximum expansion of the jog wheel

    auto radius = (jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f) - maxExpansion; // Radius of the jog wheel
    
    // Outer glow
    float baseGlow = playing ? 0.20f : 0.12f;
    if (isDragging) baseGlow += 0.15f; // Extra glow when touched
    
    float levelBoost = audioLevel * 0.45f; // Boost the glow based on audio level
    float glowIntensity = baseGlow + levelBoost;
    
    float outerPulse = audioLevel * 8.0f; // Pulse the glow based on audio level
    
    // Draw the outer glow
    for (int i = 6; i >= 0; --i)
    {
        float glowRadius = (radius + i * 4) + outerPulse;
        float alpha = jmax(0.0f, (glowIntensity / (i + 1)) * 0.8f); // Calculate the alpha of the outer glow
        g.setColour(accentColor.withAlpha(alpha)); // Set the color of the outer glow
        g.fillEllipse(center.x - glowRadius, center.y - glowRadius, 
                      glowRadius * 2, glowRadius * 2); // Fill the outer glow
    }
    
    // Brushed metal outer ring
    float ringRadius = radius + 2.0f;
    float ringWidth = radius * 0.12f;
    
    // Base ring color
    g.setColour(Colour(40, 40, 45));
    g.fillEllipse(center.x - ringRadius, center.y - ringRadius, ringRadius * 2, ringRadius * 2);

    // Brushed effect using multiple gradients
    for (int i = 0; i < 4; ++i)
    {
        float angle = (float)i * MathConstants<float>::halfPi;
        ColourGradient brush(Colour(140, 140, 150).withAlpha(0.15f), 
                            center.x + std::cos(angle) * ringRadius, center.y + std::sin(angle) * ringRadius,
                            Colour(20, 20, 25).withAlpha(0.0f), center.x, center.y, true);

        g.setGradientFill(brush);
        g.fillEllipse(center.x - ringRadius, center.y - ringRadius, ringRadius * 2, ringRadius * 2);
    }

    // Inner bevel of the ring
    g.setColour(Colour(15, 15, 20));
    g.drawEllipse(center.x - (ringRadius - ringWidth), center.y - (ringRadius - ringWidth), 
                  (ringRadius - ringWidth) * 2, (ringRadius - ringWidth) * 2, 1.5f);
    
    // Main platter
    float platterRadius = radius - ringWidth;
    
    ColourGradient platterGradient(Colour(35, 35, 40), center.x, center.y - platterRadius,
                                   Colour(15, 15, 18), center.x, center.y + platterRadius, false);

    g.setGradientFill(platterGradient);
    g.fillEllipse(center.x - platterRadius, center.y - platterRadius, 
                  platterRadius * 2, platterRadius * 2);
    
    // Vinyl record
    float vinylRadius = platterRadius * 0.94f;
    
    // Vinyl base
    g.setColour(Colour(10, 10, 12));
    g.fillEllipse(center.x - vinylRadius, center.y - vinylRadius, 
                  vinylRadius * 2, vinylRadius * 2);
    
    // Grooves
    for (float r = vinylRadius * 0.38f; r < vinylRadius * 0.96f; r += 2.5f)
    {
        float grooveAlpha = 0.05f + 0.05f * std::sin(r * 0.5f);
        g.setColour(Colours::white.withAlpha(grooveAlpha));
        g.drawEllipse(center.x - r, center.y - r, r * 2, r * 2, 0.4f);
    }
    
    // Moving reflections
    Graphics::ScopedSaveState save(g);
    g.addTransform(AffineTransform::rotation((float)(rotation * MathConstants<double>::twoPi), center.x, center.y));
    
    auto drawShine = [&](float angle, float width, float alpha) {
        Path shine;
        float startAngle = angle - width;
        float endAngle = angle + width;
        shine.addPieSegment(center.x - vinylRadius, center.y - vinylRadius, 
                           vinylRadius * 2, vinylRadius * 2, 
                           startAngle, endAngle, 0.45f);
        
        ColourGradient gradient(Colours::white.withAlpha(alpha), center.x, center.y,
                               Colours::white.withAlpha(0.0f), 
                               center.x + std::cos(angle) * vinylRadius, 
                               center.y + std::sin(angle) * vinylRadius, false);
                               
        g.setGradientFill(gradient);
        g.fillPath(shine);
    };

    drawShine(-MathConstants<float>::pi * 0.25f, 0.4f, 0.08f);
    drawShine(MathConstants<float>::pi * 0.75f, 0.3f, 0.06f);

    // Center label
    float labelRadius = radius * 0.28f;
    
    // Label core
    ColourGradient labelGradient(accentColor.brighter(0.4f), center.x, center.y - labelRadius,
                                 accentColor.darker(0.6f), center.x, center.y + labelRadius, false);
    g.setGradientFill(labelGradient);
    g.fillEllipse(center.x - labelRadius, center.y - labelRadius, labelRadius * 2, labelRadius * 2);
    
    // Label details
    g.setColour(Colours::black.withAlpha(0.3f));
    g.drawEllipse(center.x - labelRadius * 0.85f, center.y - labelRadius * 0.85f, 
                  labelRadius * 1.7f, labelRadius * 1.7f, 1.0f);
    
    // Spindle hole
    float spindleRadius = radius * 0.04f;
    g.setColour(Colour(5, 5, 10));
    g.fillEllipse(center.x - spindleRadius, center.y - spindleRadius, spindleRadius * 2, spindleRadius * 2);
    
    // Position marker
    float markerAngle = (float)(rotation * MathConstants<double>::twoPi);
    float innerR = labelRadius * 1.05f;
    float outerR = vinylRadius * 0.98f;
    
    Point<float> p1(center.x + std::cos(markerAngle) * innerR, center.y + std::sin(markerAngle) * innerR);
    Point<float> p2(center.x + std::cos(markerAngle) * outerR, center.y + std::sin(markerAngle) * outerR);
    
    // Shadow
    g.setColour(Colours::black.withAlpha(0.5f));
    g.drawLine(p1.x + 1, p1.y + 1, p2.x + 1, p2.y + 1, 3.0f);
    
    // Needle
    g.setColour(Colours::white);
    g.drawLine(p1.x, p1.y, p2.x, p2.y, 2.5f);
    
    // Glow tip
    g.setColour(accentColor);
    g.fillEllipse(p2.x - 3, p2.y - 3, 6, 6);
    g.setColour(Colours::white);
    g.fillEllipse(p2.x - 1.5f, p2.y - 1.5f, 3, 3);
}

void JogWheel::resized()
{
}

// Mouse down
void JogWheel::mouseDown(const MouseEvent& event)
{
    auto center = getLocalBounds().getCentre().toFloat();
    float dx = event.position.x - center.x;
    float dy = event.position.y - center.y;
    lastMouseAngle = std::atan2(dy, dx);
    dragStartRotation = rotation;
    isDragging = true;
    repaint();
}

// Mouse drag
void JogWheel::mouseDrag(const MouseEvent& event)
{
    if (isDragging)
    {
        auto center = getLocalBounds().getCentre().toFloat();
        float dx = event.position.x - center.x;
        float dy = event.position.y - center.y;
        float currentAngle = std::atan2(dy, dx);
        
        float angleDiff = currentAngle - (float)lastMouseAngle;
        
        // Handle wrap-around at +/- PI
        if (angleDiff > MathConstants<float>::pi)
            angleDiff -= MathConstants<float>::twoPi;
        else if (angleDiff < -MathConstants<float>::pi)
            angleDiff += MathConstants<float>::twoPi;
        
        // Convert angle difference to rotation (full circle = 1.0)
        double rotationChange = angleDiff / MathConstants<float>::twoPi;
        
        // Update visual rotation for responsive feedback
        rotation += rotationChange;
        
        // Keep rotation in 0-1 range
        while (rotation < 0.0) rotation += 1.0;
        while (rotation >= 1.0) rotation -= 1.0;
        
        // Send scratch command to player
        if (onScratch)
            onScratch(rotationChange);
        
        lastMouseAngle = currentAngle;
        repaint();
    }
}

// Mouse up
void JogWheel::mouseUp(const MouseEvent& event)
{
    isDragging = false;
    repaint();
}

// Timer callback
void JogWheel::timerCallback()
{
    if (playing && !isDragging)
    {
        rotation += 0.02;  // Rotation speed when playing
        if (rotation > 1.0)
            rotation -= 1.0;
        repaint();
    }
}

// Set position relative
void JogWheel::setPositionRelative(double pos)
{
    if (!isDragging)
    {
        rotation = pos;
        // Keep in valid range
        while (rotation < 0.0) rotation += 1.0;
        while (rotation >= 1.0) rotation -= 1.0;
        repaint();
    }
}

// Set playing
void JogWheel::setPlaying(bool isPlaying)
{
    playing = isPlaying;
}

// Set accent color
void JogWheel::setAccentColor(Colour color)
{
    accentColor = color;
    repaint();
}

// Set audio level
void JogWheel::setAudioLevel(float level)
{
    audioLevel = level;
    repaint();
}


// DJ Button
DJButton::DJButton(const String& buttonName) : TextButton(buttonName)
{
}

// Paint button
void DJButton::paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.5f);
    bool isActive = getToggleState() || shouldDrawButtonAsDown;
    String text = getButtonText();
    
    // Color logic
    bool isStopButton = text == "STOP";
    Colour baseAccent = isStopButton ? Colours::red : accentColor;
    
    // Neon outer glow
    float intensity = shouldDrawButtonAsHighlighted ? 1.0f : 0.6f;
    if (shouldDrawButtonAsDown) intensity = 1.2f;
    
    for (int i = 4; i >= 1; --i)
    {
        float alpha = (0.12f * intensity) / (float)i;
        g.setColour(baseAccent.withAlpha(alpha));
        g.drawRoundedRectangle(bounds.expanded((float)i * 1.2f), 6, 2.5f);
    }
    
    // Main body gradient
    Colour topColor = isActive ? baseAccent.brighter(0.1f) : Colour(50, 50, 55);
    Colour bottomColor = isActive ? baseAccent.darker(0.6f) : Colour(30, 30, 35);
    
    ColourGradient bodyGrad(topColor, 0, bounds.getY(),
                           bottomColor, 0, bounds.getBottom(), false);
    g.setGradientFill(bodyGrad);
    g.fillRoundedRectangle(bounds, 5);
    
    // Internal bevel
    g.setColour(Colours::white.withAlpha(0.05f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 4.5f, 1.0f);
    
    // Glossy top overlay
    ColourGradient gloss(Colours::white.withAlpha(0.12f), 0, bounds.getY(),
                        Colours::white.withAlpha(0.0f), 0, bounds.getCentreY(), false);
    g.setGradientFill(gloss);
    g.fillRoundedRectangle(bounds.removeFromTop(bounds.getHeight() * 0.45f), 4);
    
    // Restore bounds for icon/text drawing
    bounds = getLocalBounds().toFloat().reduced(2.0f);
    
    // Text drawing
    Colour textCol = isActive ? Colours::white : (shouldDrawButtonAsHighlighted ? baseAccent.brighter(0.4f) : baseAccent);
    g.setColour(textCol.withAlpha(1.0f)); 
    g.setFont(Font(12.0f, Font::bold));
    g.drawText(text, bounds, Justification::centred);
    
    // Accent border
    g.setColour(isActive ? Colours::white.withAlpha(0.8f) : baseAccent.withAlpha(0.7f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.5f), 5, isActive ? 1.5f : 1.0f);
}

// Set accent color
void DJButton::setAccentColor(Colour color)
{
    accentColor = color;
    repaint();
}

// DJ Knob
DJKnob::DJKnob()
{
}

// Set value
void DJKnob::setValue(double newValue, NotificationType notification)
{
    value = jlimit(minValue, maxValue, newValue);
    if (notification != dontSendNotification && onValueChange)
        onValueChange(value);
    repaint();
}

// Get value
double DJKnob::getValue() const
{
    return value;
}

// Set bipolar
void DJKnob::setBipolar(bool bipolar)
{
    isBipolar = bipolar;
    repaint();
}

// Paint knob
void DJKnob::paint(Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2);
    auto center = bounds.getCentre();
    auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2;
    
    // Cached angles
    float startAngle = MathConstants<float>::pi * 0.75f;
    float endAngle = MathConstants<float>::pi * 2.25f;
    
    // Shadows
    g.setColour(Colour(0, 0, 0).withAlpha(0.6f));
    g.fillEllipse(center.x - radius + 2, center.y - radius + 4, radius * 2, radius * 2);

    // Base ring
    ColourGradient housingGrad(Colour(60, 60, 65), center.x - radius, center.y - radius,
                               Colour(30, 30, 35), center.x + radius, center.y + radius, false);
    g.setGradientFill(housingGrad);
    g.fillEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2);
    
    // Housing bezel
    g.setColour(Colour(20, 20, 25));
    g.drawEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2, 1.0f);
    
    // Value arc
    float arcRadius = radius - 5;
    
    // Background track
    Path backgroundArc;
    backgroundArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 
                                 0, startAngle, endAngle, true);
    g.setColour(Colour(15, 15, 20));
    g.strokePath(backgroundArc, PathStrokeType(5.0f, PathStrokeType::curved, PathStrokeType::rounded));
    
    // Active value arc
    float normValue = (float)((value - minValue) / (maxValue - minValue));
    float valueAngle = startAngle + normValue * (endAngle - startAngle);
    
    Path valueArc;
    if (isBipolar)
    {
        float centerAngle = (startAngle + endAngle) / 2.0f;
        // If value < 0.5, draw from value to center. If value > 0.5, draw from center to value.
        // Normalized center is 0.5
        
        if (std::abs(normValue - 0.5f) > 0.01f) // Deadzone
        {
            valueArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 
                                   0, (normValue < 0.5f) ? valueAngle : centerAngle,
                                      (normValue < 0.5f) ? centerAngle : valueAngle, true);
        }
    }
    else
    {
        // Standard unipolar
        if (normValue > 0.01f)
        {
            valueArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 
                                   0, startAngle, valueAngle, true);
        }
    }
    
    // Draw Glow
    auto arcColor = accentColor;
    // Dynamic color for volume (heatmap style) if not bipolar
    if (!isBipolar && normValue > 0.8f) 
        arcColor = arcColor.interpolatedWith(Colours::red, (normValue - 0.8f) * 5.0f);
        
    g.setColour(arcColor.withAlpha(0.6f));
    g.strokePath(valueArc, PathStrokeType(9.0f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour(arcColor);
    g.strokePath(valueArc, PathStrokeType(4.0f, PathStrokeType::curved, PathStrokeType::rounded));

    // Knob cap
    float knobRadius = radius * 0.65f;
    
    // Cap shadow
    g.setColour(Colour(0, 0, 0).withAlpha(0.5f));
    g.fillEllipse(center.x - knobRadius + 1, center.y - knobRadius + 2, knobRadius * 2, knobRadius * 2);
    
    // Main body gradient
    ColourGradient knobGrad(Colour(110, 110, 115), center.x, center.y - knobRadius,
                            Colour(40, 40, 45), center.x, center.y + knobRadius, false);
    g.setGradientFill(knobGrad);
    g.fillEllipse(center.x - knobRadius, center.y - knobRadius, knobRadius * 2, knobRadius * 2);
    
    // Micro-texture
    g.setColour(Colour(30, 30, 35).withAlpha(0.3f));
    for (int i = 0; i < 18; ++i)
    {
        float angle = i * MathConstants<float>::pi / 9.0f;
        float r1 = knobRadius - 1;
        float r2 = knobRadius - 4;
        g.drawLine(center.x + std::cos(angle) * r1, center.y + std::sin(angle) * r1,
                   center.x + std::cos(angle) * r2, center.y + std::sin(angle) * r2, 1.0f);
    }
    
    // Edge Highlight
    g.setColour(Colours::white.withAlpha(0.3f));
    g.drawEllipse(center.x - knobRadius + 1, center.y - knobRadius + 1, 
                  (knobRadius - 1) * 2, (knobRadius - 1) * 2, 1.0f);
    
    // Top Shine
    ColourGradient shineGrad(Colours::white.withAlpha(0.4f), center.x, center.y - knobRadius * 0.8f,
                             Colours::transparentWhite, center.x, center.y, false);
    g.setGradientFill(shineGrad);
    g.fillEllipse(center.x - knobRadius * 0.5f, center.y - knobRadius * 0.7f, 
                  knobRadius, knobRadius * 0.5f);

    // Indicator pointer
    float pointOuter = knobRadius * 0.85f;
    float pointInner = knobRadius * 0.2f;
    float currentAngle = startAngle + normValue * (endAngle - startAngle);
    float pointerAngle = currentAngle - MathConstants<float>::halfPi;
    
    Line<float> pointer(center.x + std::cos(pointerAngle) * pointInner,
                        center.y + std::sin(pointerAngle) * pointInner,
                        center.x + std::cos(pointerAngle) * pointOuter,
                        center.y + std::sin(pointerAngle) * pointOuter);
                        
    g.setColour(Colour(20, 20, 20)); // Dark slot
    g.drawLine(pointer, 5.0f);
    
    g.setColour(arcColor); // Colored fill
    g.drawLine(pointer, 2.0f);
    
    // Center cap
    float capRadius = knobRadius * 0.2f;
    ColourGradient capGradient(Colour(80, 80, 85), center.x, center.y - capRadius,
                               Colour(50, 50, 55), center.x, center.y + capRadius, false);
    g.setGradientFill(capGradient);
    g.fillEllipse(center.x - capRadius, center.y - capRadius, capRadius * 2, capRadius * 2);
    
    // Dragging highlight
    if (isDragging)
    {
        g.setColour(accentColor.withAlpha(0.15f));
        g.fillEllipse(center.x - knobRadius, center.y - knobRadius, knobRadius * 2, knobRadius * 2);
    }
}

// Mouse down
void DJKnob::mouseDown(const MouseEvent& event)
{
    auto center = getLocalBounds().getCentre().toFloat();
    float dx = event.position.x - center.x;
    float dy = event.position.y - center.y;
    lastMouseAngle = std::atan2(dy, dx);
    isDragging = true;
    repaint();
}

// Mouse drag
void DJKnob::mouseDrag(const MouseEvent& event)
{
    if (isDragging)
    {
        auto center = getLocalBounds().getCentre().toFloat();
        float dx = event.position.x - center.x;
        float dy = event.position.y - center.y;
        float currentAngle = std::atan2(dy, dx);
        
        float angleDiff = currentAngle - (float)lastMouseAngle;
        
        // Handle wrap-around
        if (angleDiff > MathConstants<float>::pi)
            angleDiff -= MathConstants<float>::twoPi;
        else if (angleDiff < -MathConstants<float>::pi)
            angleDiff += MathConstants<float>::twoPi;
        
        // Convert angle to value change
        double range = maxValue - minValue;
        double valueChange = (angleDiff / MathConstants<float>::twoPi) * range * 2.0;
        
        // Update value
        double newValue = value + valueChange;
        newValue = jmax(minValue, jmin(maxValue, newValue));
        
        if (newValue != value)
        {
            value = newValue;
            if (onValueChange)
                onValueChange(value);
            repaint();
        }
        
        lastMouseAngle = currentAngle;
    }
}

// Mouse up
void DJKnob::mouseUp(const MouseEvent& event)
{
    isDragging = false;
    repaint();
}

// Set accent color
void DJKnob::setAccentColor(Colour color)
{
    accentColor = color;
    repaint();
}

// Set range
void DJKnob::setRange(double minVal, double maxVal)
{
    minValue = minVal;
    maxValue = maxVal;
    value = jmax(minValue, jmin(maxValue, value));
    repaint();
}


// Deck GUI Implementation
DeckGUI::DeckGUI(DJAudioPlayer* _player, 
                AudioFormatManager& formatManagerToUse,
                AudioThumbnailCache& cacheToUse,
                int _deckNumber
           ) : player(_player),
               deckNumber(_deckNumber),
                accentColor((deckNumber == 1) ? Colour(0, 200, 255) : Colour(255, 100, 150)),
                waveformDisplay(formatManagerToUse, cacheToUse),
                isSyncing(false)
{
    // Initialise Hot Cues (R3)
    hotCues.resize(8, -1.0); // -1.0 means unset
    for (int i = 0; i < 8; ++i)
    {
        hotCueButtons.push_back(std::make_unique<DJButton>(String(i + 1)));
        hotCueButtons[i]->setAccentColor(accentColor.withAlpha(0.6f)); // Slightly more visible when unset
        addAndMakeVisible(*hotCueButtons[i]);
        hotCueButtons[i]->addListener(this);
    }
    
    clearCuesButton.setAccentColor(Colours::red);
    clearCuesButton.setButtonText("CLR");
    addAndMakeVisible(clearCuesButton);
    clearCuesButton.addListener(this);

    // Configure jog wheel
    jogWheel.setAccentColor(accentColor);
    jogWheel.onScratch = [this](double delta) {
        if (player != nullptr)
        {
            double currentPos = player->getPositionRelative();
            player->setPositionRelative(currentPos + delta);
        }
    };

    addAndMakeVisible(jogWheel);
    
    // Configure styled buttons
    playStopButton.setAccentColor(accentColor);
    loopButton.setAccentColor(accentColor);
    loadButton.setAccentColor(accentColor);
    
    addAndMakeVisible(playStopButton);
    addAndMakeVisible(loopButton);
    addAndMakeVisible(loadButton);
    
    playStopButton.addListener(this);
    loopButton.addListener(this);
    loadButton.addListener(this);
    
    // Configure SYNC button
    syncButton.setAccentColor(accentColor);
    addAndMakeVisible(syncButton);
    syncButton.addListener(this);
    
    // Configure BPM label
    bpmLabel.setText("-BPM-", dontSendNotification);
    bpmLabel.setFont(Font(16.0f, Font::bold));
    bpmLabel.setColour(Label::textColourId, accentColor);
    bpmLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(bpmLabel);
    
    // Configure knobs
    speedKnob.setAccentColor(accentColor);
    reverbKnob.setAccentColor(accentColor);
    echoKnob.setAccentColor(accentColor);
    
    speedKnob.setRange(0.5, 2.0);
    speedKnob.setValue(1.0);
    
    reverbKnob.setRange(0.0, 1.0);
    reverbKnob.setValue(0.0);
    echoKnob.setRange(0.0, 1.0);
    echoKnob.setValue(0.0); // Echo off by default
    
    addAndMakeVisible(speedKnob);
    addAndMakeVisible(reverbKnob);
    addAndMakeVisible(echoKnob);
    
    // Set up knob callbacks
    
    speedKnob.onValueChange = [this](double val) {
        if (player != nullptr)
            player->setSpeed(val);
    };
    
    reverbKnob.onValueChange = [this](double val) {
        if (player != nullptr)
            player->setReverb(val);
    };
    echoKnob.onValueChange = [this](double val) {
        if (player != nullptr)
            player->setEcho(val);
    };

    // Filter knob callback
    filterKnob.onValueChange = [this](double value)
    {
        if (player != nullptr)
        {
            player->setFilter(value);
        }
    };
    filterKnob.setRange(0.0, 1.0);
    filterKnob.setValue(0.5); // Center is neutral
    filterKnob.setBipolar(true); // Center zero
    
    // Feedback knob callback
    feedbackKnob.onValueChange = [this](double value)
    {
        if (player != nullptr)
        {
            player->setEchoFeedback(value);
        }
    };
    
    addAndMakeVisible(filterKnob);
    addAndMakeVisible(feedbackKnob);
    
    // Configure labels
    auto setupLabel = [this](Label& label) {
        label.setFont(Font(10.0f, Font::bold));
        label.setColour(Label::textColourId, Colours::grey);
        label.setJustificationType(Justification::centred);
        addAndMakeVisible(label);
    };
    
    setupLabel(speedLabel);
    setupLabel(reverbLabel);
    setupLabel(echoLabel);
    setupLabel(filterLabel);
    setupLabel(feedbackLabel);
    
    // Set colors
    filterKnob.setAccentColor(accentColor);
    feedbackKnob.setAccentColor(accentColor);
    
    // Deck label
    deckLabel.setText("DECK " + String(deckNumber), dontSendNotification);
    deckLabel.setFont(Font(16.0f, Font::bold));
    deckLabel.setColour(Label::textColourId, accentColor);
    deckLabel.setJustificationType(Justification::centredLeft);
    addAndMakeVisible(deckLabel);
    
    // Track name label
    trackNameLabel.setText("", dontSendNotification);
    trackNameLabel.setFont(Font(11.0f));
    trackNameLabel.setColour(Label::textColourId, Colours::lightgrey);
    trackNameLabel.setJustificationType(Justification::centredLeft);
    addAndMakeVisible(trackNameLabel);
    
    // Waveform display with click-to-seek
    waveformDisplay.setColour(accentColor);
    waveformDisplay.onPositionChange = [this](double pos) {
        if (player != nullptr)
            player->setPositionRelative(pos);
    };
    addAndMakeVisible(waveformDisplay);
    
    startTimer(50);
}

DeckGUI::~DeckGUI()
{
    stopTimer();
}

void DeckGUI::paint(Graphics& g)
{
    // Dark gradient background
    ColourGradient bgGradient(Colour(35, 35, 40), 0, 0,
                              Colour(25, 25, 30), 0, (float)getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8);
    
    // Border with accent glow
    g.setColour(accentColor.withAlpha(0.3f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 8, 2);
    
    // Inner border
    g.setColour(Colour(50, 50, 55));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(3), 6, 1);
}

void DeckGUI::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Top section (deck label, track name, and BPM)
    auto topSection = area.removeFromTop(40);
    deckLabel.setBounds(topSection.removeFromLeft(80));
    
    // Position BPM on the right side of the top section
    auto bpmArea = topSection.removeFromRight(80);
    bpmLabel.setBounds(bpmArea);
    bpmLabel.setVisible(true);
    
    trackNameLabel.setBounds(topSection.reduced(5, 10));
    
    // Waveform display
    auto waveformSection = area.removeFromTop(80);
    waveformDisplay.setBounds(waveformSection.reduced(0, 5));
    
    // Hot Cue strip below waveform (R3)
    auto hotCueStrip = area.removeFromTop(30);
    hotCueStrip.removeFromTop(2);
    {
        int totalBtns = 9; // 8 hot cues + 1 clear button
        int stripSpacing = 3;
        int stripBtnWidth = (hotCueStrip.getWidth() - (totalBtns - 1) * stripSpacing) / totalBtns;
        int stripX = hotCueStrip.getX();
        int stripY = hotCueStrip.getY();
        int stripH = hotCueStrip.getHeight() - 2;
        
        for (int i = 0; i < 8; ++i)
        {
            hotCueButtons[i]->setBounds(stripX + i * (stripBtnWidth + stripSpacing),
                                       stripY, stripBtnWidth, stripH);
        }
        clearCuesButton.setBounds(stripX + 8 * (stripBtnWidth + stripSpacing),
                                 stripY, stripBtnWidth, stripH);
    }
    
    // Main control area
    auto controlArea = area;
    
    // Knobs section (Effects and Speed)
    auto knobSection = controlArea.removeFromRight(190);
    knobSection.removeFromTop(15); 
    knobSection.removeFromLeft(5); 
    
    int knobSize = 55;
    int labelHeight = 15;
    int knobSpacing = 8;
    
    // Manual Knob Layout
    int startX = knobSection.getX();
    int startY = knobSection.getY();
    int colSpacing = knobSize + knobSpacing;
    int rowSpacing = knobSize + labelHeight + 10;
    
    // Row 1 (Speed (Left), Reverb (Right))
    speedKnob.setBounds(startX, startY, knobSize, knobSize);
    speedLabel.setBounds(startX, startY + knobSize, knobSize, labelHeight);
    
    reverbKnob.setBounds(startX + colSpacing * 2, startY, knobSize, knobSize);
    reverbLabel.setBounds(startX + colSpacing * 2, startY + knobSize, knobSize, labelHeight);
    
    // Center (Filter (Middle of everything))
    int filterX = startX + colSpacing;
    int filterY = startY + (rowSpacing / 2); 
    
    filterKnob.setBounds(filterX, filterY, knobSize, knobSize);
    filterLabel.setBounds(filterX, filterY + knobSize, knobSize, labelHeight);
    
    // Row 2 (Echo (Left), Feedback (Right))
    int row2Y = startY + rowSpacing;
    
    echoKnob.setBounds(startX, row2Y, knobSize, knobSize);
    echoLabel.setBounds(startX, row2Y + knobSize, knobSize, labelHeight);
    
    feedbackKnob.setBounds(startX + colSpacing * 2, row2Y, knobSize, knobSize);
    feedbackLabel.setBounds(startX + colSpacing * 2, row2Y + knobSize, knobSize, labelHeight);
    
    // Center area layout
    // Left side of center: Transport buttons
    auto transportArea = controlArea.removeFromLeft(75);
    
    int buttonHeight = 36;
    int buttonWidth = 70;
    int buttonSpacing = 10;
    int totalStackHeight = (buttonHeight * 4) + (buttonSpacing * 3);
    
    // Center vertically in the available area
    int topPadding = (transportArea.getHeight() - totalStackHeight) / 2;
    transportArea.removeFromTop(topPadding + 5);
    
    int transportStartX = transportArea.getX() + 5;
    
    playStopButton.setBounds(transportArea.removeFromTop(buttonHeight).withWidth(buttonWidth).withX(transportStartX));
    transportArea.removeFromTop(buttonSpacing);
    syncButton.setBounds(transportArea.removeFromTop(buttonHeight).withWidth(buttonWidth).withX(transportStartX));
    transportArea.removeFromTop(buttonSpacing);
    loopButton.setBounds(transportArea.removeFromTop(buttonHeight).withWidth(buttonWidth).withX(transportStartX));
    transportArea.removeFromTop(buttonSpacing);
    loadButton.setBounds(transportArea.removeFromTop(buttonHeight).withWidth(buttonWidth).withX(transportStartX));
    
    // Center (Jog wheel)
    auto jogArea = controlArea;
    
    // Jog wheel takes remaining space centered
    int jogSize = (int)(jmin(jogArea.getWidth(), jogArea.getHeight()) * 0.99f);
    jogWheel.setBounds(jogArea.getCentreX() - jogSize / 2, 
                       jogArea.getCentreY() - jogSize / 2,
                       jogSize, jogSize);
}

void DeckGUI::buttonClicked(Button* button)
{
    if (player == nullptr) return;
    
    if (button == &playStopButton)
    {
        if (player->isPlaying())
        {
            DBG("Deck " << deckNumber << ": Stop");
            player->stop();
            isSyncing = false; // Turn off sync lock when stopping manually
            syncButton.setToggleState(false, dontSendNotification);
            jogWheel.setPlaying(false);
            playStopButton.setButtonText("PLAY");
        }
        else
        {
            DBG("Deck " << deckNumber << ": Play");
            player->start();
            jogWheel.setPlaying(true);
            playStopButton.setButtonText("STOP");
        }
    }

    // Loop button
    if (button == &loopButton)
    {
        isLooping = !isLooping;
        player->setLooping(isLooping);
        loopButton.setToggleState(isLooping, dontSendNotification);
        DBG("Deck " << deckNumber << ": Loop " << (isLooping ? "ON" : "OFF"));
    }
    if (button == &loadButton)
    {
        auto fileChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;
        fChooser.launchAsync(fileChooserFlags, [this](const FileChooser& chooser)
        {
            File chosenFile = chooser.getResult();
            if (chosenFile.exists())
            {
                loadFile(URL{chosenFile});
            }
        });
    }
    if (button == &syncButton)
    {
        DBG("Deck " << deckNumber << ": Sync Pressed");
        
        if (onSyncPressed)
        {
            onSyncPressed();
        }
    }
    
    // Handle Hot Cue buttons (R3)
    for (int i = 0; i < 8; ++i)
    {
        if (button == hotCueButtons[i].get())
        {
            if (ModifierKeys::getCurrentModifiers().isShiftDown())
            {
                hotCues[i] = -1.0;
                DBG("Deck " << deckNumber << ": Cleared Hot Cue " << (i + 1));
            }
            else if (hotCues[i] < 0.0) // Unset
            {
                hotCues[i] = player->getPositionRelative();
                DBG("Deck " << deckNumber << ": Set Hot Cue " << (i + 1) << " to " << hotCues[i]);
            }
            else // Jump
            {
                player->setPositionRelative(hotCues[i]);
                waveformDisplay.setPositionRelative(hotCues[i]);
                DBG("Deck " << deckNumber << ": Jump to Hot Cue " << (i + 1));
            }
            updateHotCueVisuals();
            saveHotCues();
            return;
        }
    }
    
    // Clear hot cues button
    if (button == &clearCuesButton)
    {
        std::fill(hotCues.begin(), hotCues.end(), -1.0);
        updateHotCueVisuals();
        saveHotCues();
        DBG("Deck " << deckNumber << ": All Hot Cues Cleared");
    }
}

// File drag and drop
bool DeckGUI::isInterestedInFileDrag(const StringArray& files)
{
    return true;
}

// Files dropped
void DeckGUI::filesDropped(const StringArray& files, int x, int y)
{
    if (files.size() == 1)
    {
        loadFile(URL{File{files[0]}});
    }
}

// Drag source details
bool DeckGUI::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
    return true;
}

// Item dropped
void DeckGUI::itemDropped(const SourceDetails& dragSourceDetails)
{
    String fileList = dragSourceDetails.description.toString();
    StringArray files;
    files.addLines(fileList);
    
    if (files.size() > 0)
    {
        loadFile(URL{File{files[0]}});
    }
}

// Timer callback
void DeckGUI::timerCallback()
{
    if (player == nullptr) return;
    
    double pos = player->getPositionRelative();
    
    // Handle NaN
    if (std::isnan(pos)) pos = 0.0;
    
    // Update waveform position
    waveformDisplay.setPositionRelative(pos);
    
    // Update loop points visualization
    std::pair<double, double> loopPoints = player->getLoopPoints();
    waveformDisplay.setLoopPoints(loopPoints.first, loopPoints.second);
 
    // Update jog wheel with audio level for pulsing effect
    // Mix L and R channels for a mono pulse
    float rmsL = player->getRMSLevel(0);
    float rmsR = player->getRMSLevel(1);
    float avgLevel = (rmsL + rmsR) * 0.5f;
    
    // Convert linear RMS to a visual scale
    float visualLevel = jmin(1.0f, avgLevel * 2.5f);
    jogWheel.setAudioLevel(visualLevel);
    
    // Check for updated BPM from player
    double currentBPM = player->getCurrentBPM();
    double detected = player->getDetectedBPM();
    
    // Store the base detected BPM for sync logic
    if (detected > 0.0)
        detectedBPM = detected;

    if (currentBPM > 0.0)
    {
        bpmLabel.setText(String(currentBPM, 1) + " BPM", dontSendNotification);
    }
    else if (detected <= 0.0 && !currentFilePath.isEmpty())
    {
        bpmLabel.setText("-BPM-", dontSendNotification);
    }
    
    // Handle looping (Restart from beginning when song ends)
    if (isLooping && pos >= 0.99)
    {
        player->setPositionRelative(0.0);
        player->start(); // Ensure playback continues
    }
    
    // Update Play/Stop button text base on current state
    if (player->isPlaying())
    {
        if (playStopButton.getButtonText() != "STOP")
            playStopButton.setButtonText("STOP");
    }
    else
    {
        if (playStopButton.getButtonText() != "PLAY")
            playStopButton.setButtonText("PLAY");
    }

}


// Load file
void DeckGUI::loadFile(URL audioURL)
{
    if (player == nullptr || !player->loadURL(audioURL))
        return;

    waveformDisplay.loadURL(audioURL);
    
    // Update track name label
    String fileName = audioURL.getLocalFile().getFileNameWithoutExtension();
    trackNameLabel.setText(fileName, dontSendNotification);
    
    // Store path and load hot cues
    currentFilePath = audioURL.getLocalFile().getFullPathName();
    loadHotCues();
    
    // Notify listeners that a file has been loaded
    if (onFileLoaded)
        onFileLoaded(currentFilePath);
    
    // Reset control variables
    cuePoint = 0.0;
    isLooping = false;
    loopButton.setToggleState(false, dontSendNotification);
    isSyncing = false;
    syncButton.setToggleState(false, dontSendNotification);
    
    // Reset BPM display
    detectedBPM = 0.0;
    bpmLabel.setText("-BPM-", dontSendNotification);
}

// Get deck name
String DeckGUI::getDeckName() const
{
    return "Deck " + String(deckNumber);
}

// Sync functionality
void DeckGUI::syncToOtherDeck(DeckGUI* otherDeck)
{
    if (otherDeck == nullptr || player == nullptr) return;
    
    double otherBPM = otherDeck->getBPM();
    double myBPM = getBPM();
    
    if (otherBPM > 0.0 && myBPM > 0.0)
    {
        // Calculate required speed ratio
        double currentSpeed = player->getCurrentSpeed();
        double targetSpeed = (otherBPM / myBPM);
        
        // Take into account the other deck's current playback speed
        targetSpeed *= otherDeck->getSpeed();
        
        // Limit speed change
        targetSpeed = jlimit(0.5, 2.0, targetSpeed);
        
        // Apply speed
        setSpeed(targetSpeed);
        
        // Phase Sync: Align beats
        // Calculate the other deck's offset from its nearest beat
        double otherPos = otherDeck->player->getPositionRelative() * otherDeck->player->getTrackLengthInSeconds();
        double otherBeat = otherDeck->player->getNearestBeat(otherPos);
        double phaseOffset = otherPos - otherBeat;
        
        // Find nearest beat and apply the same offset
        double myPos = player->getPositionRelative() * player->getTrackLengthInSeconds();
        double myBeat = player->getNearestBeat(myPos);
        double targetPos = myBeat + phaseOffset;
        
        // Move playhead
        if (targetPos >= 0.0)
        {
            double relPos = targetPos / player->getTrackLengthInSeconds();
            player->setPositionRelative(relPos);
        }
    }
}

double DeckGUI::getBPM() const
{
    return detectedBPM; // Returns the detected BPM
}

// Returns the current speed
double DeckGUI::getSpeed() const
{
    if (player != nullptr)
        return player->getCurrentSpeed();
    return 1.0;
}

void DeckGUI::setSpeed(double speed)
{
    if (player != nullptr)
    {
        player->setSpeed(speed);
        // Update the knob visual
        speedKnob.setValue(speed);
    }
}

// Updates the hot cue visuals
void DeckGUI::updateHotCueVisuals()
{
    for (int i = 0; i < 8; ++i)
    {
        if (hotCues[i] >= 0.0)
        {
            hotCueButtons[i]->setAccentColor(accentColor); // Bright if set
            hotCueButtons[i]->setToggleState(true, dontSendNotification); // Filled if set
        }
        else
        {
            hotCueButtons[i]->setAccentColor(accentColor.withAlpha(0.6f)); // Slightly more visible when unset
            hotCueButtons[i]->setToggleState(false, dontSendNotification); // Outlined if unset
        }
    }
}

// Returns the hot cues file
File DeckGUI::getHotCuesFile()
{
    File appDataDir = File::getSpecialLocation(File::userApplicationDataDirectory);
    File otoDecksDir = appDataDir.getChildFile("OtoDecks");
    if (!otoDecksDir.exists()) otoDecksDir.createDirectory();
    return otoDecksDir.getChildFile("hotcues.csv");
}

// Saves the hot cues
void DeckGUI::saveHotCues()
{
    if (currentFilePath.isEmpty()) return;
    
    File file = getHotCuesFile();
    StringArray lines;
    
    if (file.existsAsFile())
        file.readLines(lines);
    
    String myLine = "\"" + currentFilePath + "\"";
    for (double cue : hotCues)
        myLine += "," + String(cue, 4);
    
    bool found = false;
    for (int i = 0; i < lines.size(); ++i)
    {
        if (lines[i].startsWith("\"" + currentFilePath + "\""))
        {
            lines.set(i, myLine);
            found = true;
            break;
        }
    }
    
    if (!found)
        lines.add(myLine); // Add hot cues to file
    
    // Replace hot cues in file
    file.replaceWithText(lines.joinIntoString("\n"));
}

// Loads the hot cues
void DeckGUI::loadHotCues()
{
    // Reset defaults first
    std::fill(hotCues.begin(), hotCues.end(), -1.0);
    
    if (currentFilePath.isEmpty()) 
    {
        updateHotCueVisuals();
        return;
    }
    
    // Load hot cues from file
    File file = getHotCuesFile();
    if (!file.existsAsFile()) 
    {
        updateHotCueVisuals();
        return;
    }
    
    StringArray lines;
    file.readLines(lines);
    
    // Read hot cues from file
    for (const String& line : lines)
    {
        if (line.startsWith("\"" + currentFilePath + "\""))
        {
            StringArray tokens;
            tokens.addTokens(line, ",", "\"");
            
            for (int i = 0; i < 8 && (i + 1) < tokens.size(); ++i)
            {
                hotCues[i] = tokens[i + 1].getDoubleValue();
            }
            break;
        }
    }
    
    updateHotCueVisuals(); // Update hot cues visuals
}