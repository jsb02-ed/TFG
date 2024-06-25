#include "MainComponent.h"
//#include "SlidersSetup.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    addAndMakeVisible(audioSetup);
    //addAndMakeVisible(analyser); // Add the analyser component
    setSize (1500, 800);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 0);
    }

    // Test OSC Messages
    OSCEngine->OSCSender.sendCh1();
    OSCEngine->OSCSender.sendInfo();
    OSCEngine->OSCSender.sendStatus();

    // Add buttons
    //addAndMakeVisible(initButton);
    initButton.setButtonText("Init");
    initButton.addListener(this);

    //addAndMakeVisible(syncButton);
    syncButton.setButtonText("Sync");
    syncButton.addListener(this);

    addAndMakeVisible(STModeButton);
    STModeButton.setButtonText("Self-test mode");
    STModeButton.addListener(this);

    // Add sliders
    addAndMakeVisible(masterFaderSlider);
    masterFaderSlider.setSliderStyle(juce::Slider::LinearVertical);
    masterFaderSlider.setRange(0.0, 1.0);
    masterFaderSlider.setTextValueSuffix("");
    masterFaderSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    masterFaderSlider.addListener(this);

    addAndMakeVisible(levelLabel);
    levelLabel.setText("Main LR Level", juce::dontSendNotification);
    levelLabel.setJustificationType(juce::Justification::centred);
    levelLabel.attachToComponent(&masterFaderSlider, false);

    //addAndMakeVisible(GEQSlider);
    GEQSlider.setSliderStyle(juce::Slider::LinearVertical);
    GEQSlider.setRange(0.0, 1.0);
    GEQSlider.setTextValueSuffix("");
    GEQSlider.addListener(this);

    //addAndMakeVisible(GEQLabel);
    GEQLabel.setText("Main Level", juce::dontSendNotification);
    GEQLabel.setJustificationType(juce::Justification::centred);
    GEQLabel.attachToComponent(&GEQSlider, false);

    // Add labels
    addAndMakeVisible(IPAddressLabel);
    IPAddressLabel.setText(OSCEngine->IPAddress,juce::dontSendNotification);
    IPAddressLabel.setJustificationType(juce::Justification::centred);
    IPAddressLabel.setEditable(true);
    IPAddressLabel.addListener(this);

    addAndMakeVisible(IPLabel);
    IPLabel.setText("IP Address: ", juce::dontSendNotification);
    IPLabel.setJustificationType(juce::Justification::right);
    IPLabel.attachToComponent(&IPAddressLabel, true);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    
    bufferToFill.clearActiveBufferRegion();

    //analyserComponent->getNextAudioBlock(bufferToFill);

}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//void oscMessageReceived(const juce::OSCMessage &message) override
//{
//	// This function will be called every time a message is received.
//	// You can call message.getAddressPattern().toString() to see the address pattern of the message.
//	// You can then use message[i].getFloat32() to get the values of the message.
//	// For more information, see the OSCMessage class documentation.
//}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //analyserComponent->paint(g);

    // You can add your drawing code here!
}


void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    audioSetup.setBounds(0,0, getWidth(), getHeight());
    /*analyser.setBounds(getWidth() / 8, getHeight() / 8, getWidth() - 2 * getWidth() / 8, getHeight()/2 - getHeight() / 8);*/

    auto topLeft = 50;

    // Add buttons
    initButton.setBounds(topLeft, topLeft, getWidth() / 5, getHeight() / 25);
    syncButton.setBounds(topLeft, topLeft + getHeight() / 20, getWidth() / 5, getHeight() / 25);
    STModeButton.setBounds(12.1 * getWidth() / 16, 7 * getHeight() / 8, getWidth() / 12, getHeight() / 25);

    // Add sliders
    auto sliderLeft = 120;
    masterFaderSlider.setBounds(12 * getWidth() / 16, 3 * getHeight() / 8, getWidth() / 15, getHeight() / 2.5);
    GEQSlider.setBounds(sliderLeft + getWidth() / 25, sliderLeft, getWidth() / 30, getHeight() / 2);

    // Add labels
    IPAddressLabel.setBounds(13 * getWidth() / 16, 7 * getHeight() / 8 + getHeight() / 25, getWidth() / 12, getHeight() / 25);

}

void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &initButton) {
        DBG("Initializing...");
        DBG("Init not implemented!");
        //OSCEngine->OSCSender.fader("main", "st");
    }

    if (button == &syncButton) {
		DBG("Sync button clicked!");
        OSCEngine->OSCSender.fader("main", "st");
	}

    if (button == &STModeButton)
    {
        if (STModeButton.getToggleState())
        {
            DBG("Self-test mode enabled!");
        }
        else
        {
            DBG("Self-test mode disabled!");
        }
    }
}

void MainComponent::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &masterFaderSlider) {
        DBG("Slider value: " + juce::String(masterFaderSlider.getValue()));
        OSCEngine->OSCSender.fader("main", "st", (float)masterFaderSlider.getValue());
        //OSCEngine->OSCSender.fx(8, 15, (float)masterFaderSlider.getValue());
    }

    if (slider == &GEQSlider) {
		DBG("GEQ Slider value: " + juce::String(GEQSlider.getValue()));
        for (int i = 0; i < 32; i++)
        {
        OSCEngine->OSCSender.fx(8, i, (float)GEQSlider.getValue());
        }
	}
}

void MainComponent::labelTextChanged(juce::Label* label)
{
    if (label == &IPAddressLabel) {
		DBG("Changing IP Address to: " + IPAddressLabel.getText() + "...");
		OSCEngine->IPAddress = IPAddressLabel.getText();
        OSCEngine->changeIPAddress();
	}
}
