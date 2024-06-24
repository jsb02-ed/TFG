#pragma once

#include <JuceHeader.h>
#include "OSCSetup.h"
#include "AnalyserComponent.h"
#include "AudioSetupComponent.h"
//#include "SlidersSetup.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  :
    public juce::AudioAppComponent,
    public juce::OSCSender,
    public juce::Slider::Listener, 
    public juce::Button::Listener
    //SpecAnalyzer
    //public juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;

private:
    //==============================================================================
    // Your private member variables go here...


    // Set up the Audio Setup Component
    AudioSetupComponent audioSetup;
    // Set up the Analyser Component
    /*AnalyserComponent analyser;*/

    // Set up the OSCEngine
    OSCSetup* OSCEngine = new OSCSetup();
    //OSCSetup* OSCEngine = OSCSetup::getInstance();


    // Set up Sliders
    //SlidersSetup* SlidersEngine = new SlidersSetup();

    // Adding sliders
    juce::TextButton syncButton;
    juce::TextButton initButton;
    juce::Slider masterFaderSlider;
    juce::Label  levelLabel;
    juce::Slider GEQSlider;
    juce::Label GEQLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};