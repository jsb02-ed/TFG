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
    addAndMakeVisible(initButton);
    initButton.setButtonText("X32 Initialization");
    initButton.addListener(this);

    //addAndMakeVisible(syncButton);
    syncButton.setButtonText("Sync");
    syncButton.addListener(this);

    addAndMakeVisible(STModeButton);
    STModeButton.setButtonText("Self-test mode");
    STModeButton.addListener(this);

    addAndMakeVisible(freezeButton);
    freezeButton.setButtonText("Freeze");
    freezeButton.addListener(this);

    addAndMakeVisible(delayRefButton);
    delayRefButton.setButtonText("Reference signal DELAY");
    delayRefButton.addListener(this);

    addAndMakeVisible(delayMeasButton);
    delayMeasButton.setButtonText("Measurement signal DELAY");
    delayMeasButton.addListener(this);

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

    addAndMakeVisible(micSlider);
    micSlider.setSliderStyle(juce::Slider::LinearVertical);
    micSlider.setRange(0.0, 1.0);
    micSlider.setTextValueSuffix("");
    micSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    micSlider.addListener(this);

    addAndMakeVisible(micLabel);
    micLabel.setText("Mic Level", juce::dontSendNotification);
    micLabel.setJustificationType(juce::Justification::centred);
    micLabel.attachToComponent(&micSlider, false);

    //addAndMakeVisible(GEQSlider);
    GEQSlider.setSliderStyle(juce::Slider::LinearVertical);
    GEQSlider.setRange(0.0, 1.0);
    GEQSlider.setTextValueSuffix("");
    GEQSlider.addListener(this);

    //addAndMakeVisible(GEQLabel);
    GEQLabel.setText("Main Level", juce::dontSendNotification);
    GEQLabel.setJustificationType(juce::Justification::centred);
    GEQLabel.attachToComponent(&GEQSlider, false);

    addChildComponent(delayRefSlider);
    delayRefSlider.setSliderStyle(juce::Slider::LinearBar);
    delayRefSlider.setRange(0.3, 100.0,0.1);
    delayRefSlider.setTextValueSuffix(" ms");
    delayRefSlider.addListener(this);

    addChildComponent(delayMeasSlider);
    delayMeasSlider.setSliderStyle(juce::Slider::LinearBar);
    delayMeasSlider.setRange(0.3, 100.0, 0.1);
    delayMeasSlider.setTextValueSuffix(" ms");
    delayMeasSlider.addListener(this);

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

    addAndMakeVisible(magnitudeLabel);
    magnitudeLabel.setText("MAGNITUDE", juce::dontSendNotification);
    magnitudeLabel.setJustificationType(juce::Justification::left);

    addAndMakeVisible(phaseLabel);
    phaseLabel.setText("PHASE", juce::dontSendNotification);
    phaseLabel.setJustificationType(juce::Justification::left);
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
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll (juce::Colour::fromRGB(45, 45, 45));

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
    //initButton.setBounds(topLeft, topLeft, getWidth() / 5, getHeight() / 25);
    initButton.setBounds(11 * getWidth() / 16 - getWidth() / 8, topLeft, getWidth() / 8, getHeight() / 25);
    syncButton.setBounds(topLeft, topLeft + getHeight() / 20, getWidth() / 5, getHeight() / 25);
    STModeButton.setBounds(12.1 * getWidth() / 16, 7 * getHeight() / 8, getWidth() / 12, getHeight() / 25);
    freezeButton.setBounds(11 * getWidth() / 16 - getWidth() / 12, 7 * getHeight() / 8 + 10, getWidth() / 12, getHeight() / 25);
    delayMeasButton.setBounds(9 * getWidth() / 16 - getWidth() / 8, 7 * getHeight() / 8 + 10, getWidth() / 8, getHeight() / 25);
    delayRefButton.setBounds(9 * getWidth() / 16 - 2 * getWidth() / 8, 7 * getHeight() / 8 + 10, getWidth() / 8, getHeight() / 25);

    // Add sliders
    auto sliderLeft = 120;
    masterFaderSlider.setBounds(12 * getWidth() / 16, 3 * getHeight() / 8, getWidth() / 15, getHeight() / 2.5);
    micSlider.setBounds(13 * getWidth() / 16, 3 * getHeight() / 8, getWidth() / 15, getHeight() / 2.5);
    GEQSlider.setBounds(sliderLeft + getWidth() / 25, sliderLeft, getWidth() / 30, getHeight() / 2);
    delayMeasSlider.setBounds(9 * getWidth() / 16 - getWidth() / 8, 7.3 * getHeight() / 8 + 10, getWidth() / 8, getHeight() / 25);
    delayRefSlider.setBounds(9 * getWidth() / 16 - 2 * getWidth() / 8, 7.3 * getHeight() / 8 + 10, getWidth() / 8, getHeight() / 25);
    

    // Add labels
    IPAddressLabel.setBounds(13 * getWidth() / 16, 7 * getHeight() / 8 + getHeight() / 25, getWidth() / 12, getHeight() / 25);
    magnitudeLabel.setBounds(getWidth() / 16, getHeight() / 8 - getHeight() / 20, getWidth() / 6, getHeight() / 20);
    phaseLabel.setBounds(getWidth() / 16, 6.6 * getHeight() / 8 + getHeight() / 20, getWidth() / 6, getHeight() / 20);

}

void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &initButton) {
        DBG("Initializing X32...");
        OSCEngine->OSCSender.initX32();
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
            micSlider.setVisible(false);
            micLabel.setVisible(false);
            OSCEngine->OSCSender.sendCustom("/outputs/main/01/src", "27"); // OUT1 <- CH2
        }
        else
        {
            DBG("Self-test mode disabled!");
            micSlider.setVisible(true);
            micLabel.setVisible(true);
            OSCEngine->OSCSender.sendCustom("/outputs/main/01/src", "26"); // OUT1 <- CH1
        }
    }

    if (button == &delayMeasButton) {
        if (delayMeasOn) {
            delayMeasOn = false;
            OSCEngine->OSCSender.delayOut("01", false);
            delayMeasSlider.setVisible(false);
        }
        else {
            delayMeasOn = true;
            OSCEngine->OSCSender.delayOut("01", true);
            delayMeasSlider.setVisible(true);
        }
    }

    if (button == &delayRefButton) {
        if (delayRefOn) {
            delayRefOn = false;
            OSCEngine->OSCSender.delayOut("02", false);
            delayRefSlider.setVisible(false);
        }
        else {
            delayRefOn = true;
            OSCEngine->OSCSender.delayOut("02", true);
            delayRefSlider.setVisible(true);
        }
    }

    if (button == &freezeButton) {
        if (audioSetup.analyser.freezed) {
            audioSetup.analyser.freezed = false;
            freezeButton.setButtonText("Freeze");
            DBG("Unfreezed data");
        }
        else { 
            audioSetup.analyser.freezed = true;
            freezeButton.setButtonText("Unfreeze");
            audioSetup.analyser.newFreezedMagnitude = true;
            audioSetup.analyser.newFreezedPhase = true;
            DBG("Freezed data");
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

    if (slider == &micSlider) {
        DBG("Mic value: " + juce::String(micSlider.getValue()));
        OSCEngine->OSCSender.fader("ch", "01", (float)micSlider.getValue());
    }

    if (slider == &GEQSlider) {
		DBG("GEQ Slider value: " + juce::String(GEQSlider.getValue()));
        for (int i = 0; i < 32; i++)
        {
        OSCEngine->OSCSender.fx(8, i, (float)GEQSlider.getValue());
        }
	}

    if (slider == &delayMeasSlider) {
        DBG("Measurement delay: " + juce::String(delayMeasSlider.getValue()));
        OSCEngine->OSCSender.delayOut("01", (float)delayMeasSlider.getValue());
    }

    if (slider == &delayRefSlider) {
        DBG("Reference delay: " + juce::String(delayRefSlider.getValue()));
        OSCEngine->OSCSender.delayOut("02", (float)delayRefSlider.getValue());
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
