/*
  ==============================================================================

    SlidersSetup.h
    Created: 17 Apr 2024 5:16:18pm
    Author:  josep

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "OSCSetup.h"

class SlidersSetup :
    public juce::Component,
    public juce::Slider::Listener
{
public:
	SlidersSetup();

    juce::Slider frequencySlider;
    juce::Label  frequencyLabel;

    void sliderValueChanged(juce::Slider* slider) override;
    void resized() override;

private:

};

SlidersSetup::SlidersSetup()
{
    // Add sliders
    addAndMakeVisible(frequencySlider);
    frequencySlider.setRange(0.0, 1.0);          // [1]
    frequencySlider.setTextValueSuffix(" Hz");     // [2]
    frequencySlider.addListener(this);             // [3]

    addAndMakeVisible(frequencyLabel);
    frequencyLabel.setText("Level", juce::dontSendNotification);
    frequencyLabel.attachToComponent(&frequencySlider, true); // [4]
}

void SlidersSetup::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &frequencySlider)
    {
        if (slider == &frequencySlider) {
            DBG("Slider value: " + juce::String(frequencySlider.getValue()));
            //OSCSetup::OSCEngine->OSCSender.sendCh1((float)frequencySlider.getValue());
        }
	}
}

void SlidersSetup::resized()
{
    auto sliderLeft = 120;
    frequencySlider.setBounds(sliderLeft, 20, getWidth() - sliderLeft - 10, 20);
}