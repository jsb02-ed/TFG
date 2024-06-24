/*
  ==============================================================================

    AudioSetupComponent.h
    Created: 7 May 2024 8:22:01pm
    Author:  josep

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MainComponent.h"
#include "AnalyserComponent.h"


//==============================================================================
class AudioSetupComponent : public juce::AudioAppComponent,
    public juce::ChangeListener,
    private juce::Timer
{

public:
    //==============================================================================
    AudioSetupComponent()
        : audioSetupComp(deviceManager,
            0,     // minimum input channels
            2,   // maximum input channels
            0,     // minimum output channels
            0,   // maximum output channels
            false, // ability to select midi inputs
            false, // ability to select midi output device
            false, // treat channels as stereo pairs
            false) // hide advanced options
    {
        addAndMakeVisible(audioSetupComp);
        /*audioSetupComp.setOpaque(true);
        audioSetupComp.addToDesktop(0);*/
        //addAndMakeVisible(diagnosticsBox);
        addAndMakeVisible(analyser); // Add the analyser component

        diagnosticsBox.setMultiLine(true);
        diagnosticsBox.setReturnKeyStartsNewLine(true);
        diagnosticsBox.setReadOnly(true);
        diagnosticsBox.setScrollbarsShown(true);
        diagnosticsBox.setCaretVisible(false);
        diagnosticsBox.setPopupMenuEnabled(true);
        diagnosticsBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x32ffffff));
        diagnosticsBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(0x1c000000));
        diagnosticsBox.setColour(juce::TextEditor::shadowColourId, juce::Colour(0x16000000));

        cpuUsageLabel.setText("CPU Usage", juce::dontSendNotification);
        cpuUsageText.setJustificationType(juce::Justification::right);
        addAndMakeVisible(&cpuUsageLabel);
        addAndMakeVisible(&cpuUsageText);

        setSize(760, 360);

        setAudioChannels(2, 2);
        deviceManager.addChangeListener(this);

        startTimer(50);
    }

    ~AudioSetupComponent() override
    {
        deviceManager.removeChangeListener(this);
        shutdownAudio();
    }

    void prepareToPlay(int, double) override {}

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto* device = deviceManager.getCurrentAudioDevice();

        auto activeInputChannels = device->getActiveInputChannels();
        auto activeOutputChannels = device->getActiveOutputChannels();

        auto maxInputChannels = activeInputChannels.countNumberOfSetBits();
        auto maxOutputChannels = activeOutputChannels.countNumberOfSetBits();

        if (bufferToFill.buffer->getNumChannels() > 1)
        {
            auto* channelData1 = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
            auto* channelData2 = bufferToFill.buffer->getReadPointer(1, bufferToFill.startSample);

            for (auto i = 0; i < bufferToFill.numSamples; ++i)
                analyser.pushNextSampleIntoFifo(channelData1[i],channelData2[i]);
        }

        bufferToFill.clearActiveBufferRegion();
    }

    void releaseResources() override {}

    void paint(juce::Graphics& g) override
    {
        /*g.setColour(juce::Colours::grey);
        g.fillRect(getLocalBounds().removeFromRight(proportionOfWidth(0.4f)));*/
    }

    void resized() override
    {
        auto rect = getLocalBounds();

        /*audioSetupComp.setBounds(rect.removeFromLeft(proportionOfWidth(0.6f)));
        rect.reduce(10, 10);*/

        audioSetupComp.setBounds(rect.removeFromRight(proportionOfWidth(0.25f)));
        rect.reduce(10, 10);

        auto topLine(rect.removeFromTop(20));
        //cpuUsageLabel.setBounds(getWidth() - getWidth()/4, 0, 60, 20);
        //cpuUsageText.setBounds(topLine);
        rect.removeFromTop(20);

        diagnosticsBox.setBounds(rect);

        analyser.setBounds(getWidth() / 16, getHeight() / 8, 5 * getWidth() / 8, 6 * getHeight() / 8);

    }

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override
    {
        dumpDeviceInfo();
    }

    static juce::String getListOfActiveBits(const juce::BigInteger& b)
    {
        juce::StringArray bits;

        for (auto i = 0; i <= b.getHighestBit(); ++i)
            if (b[i])
                bits.add(juce::String(i));

        return bits.joinIntoString(", ");
    }

    void timerCallback() override
    {
        auto cpu = deviceManager.getCpuUsage() * 100;
        cpuUsageText.setText(juce::String(cpu, 6) + " %", juce::dontSendNotification);
    }

    void dumpDeviceInfo()
    {
        logMessage("--------------------------------------");
        logMessage("Current audio device type: " + (deviceManager.getCurrentDeviceTypeObject() != nullptr
            ? deviceManager.getCurrentDeviceTypeObject()->getTypeName()
            : "<none>"));

        if (auto* device = deviceManager.getCurrentAudioDevice())
        {
            logMessage("Current audio device: " + device->getName().quoted());
            logMessage("Sample rate: " + juce::String(device->getCurrentSampleRate()) + " Hz");
            logMessage("Block size: " + juce::String(device->getCurrentBufferSizeSamples()) + " samples");
            logMessage("Bit depth: " + juce::String(device->getCurrentBitDepth()));
            logMessage("Input channel names: " + device->getInputChannelNames().joinIntoString(", "));
            logMessage("Active input channels: " + getListOfActiveBits(device->getActiveInputChannels()));
            logMessage("Output channel names: " + device->getOutputChannelNames().joinIntoString(", "));
            logMessage("Active output channels: " + getListOfActiveBits(device->getActiveOutputChannels()));
        }
        else
        {
            logMessage("No audio device open");
        }
    }

    void logMessage(const juce::String& m)
    {
        diagnosticsBox.moveCaretToEnd();
        diagnosticsBox.insertTextAtCaret(m + juce::newLine);
    }

    //==========================================================================
    juce::Random random;
    juce::AudioDeviceSelectorComponent audioSetupComp;
    juce::Label cpuUsageLabel;
    juce::Label cpuUsageText;
    juce::TextEditor diagnosticsBox;

    AnalyserComponent analyser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioSetupComponent)
};
