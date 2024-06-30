/*
  ==============================================================================

    OSCSetup.h
    Created: 03 Apr 2024 4:19:15pm
    Author:  Josep Sala Baldomà

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class ModOSCSender :
    public juce::OSCSender
{
public:
    ModOSCSender() {
    }

    void fader(juce::String type, juce::String number) {
        this->send("/"+ type + "/" + juce::String(number) + "/mix/fader");
    }

    void fader(juce::String type, juce::String number, float value) {
        this->send("/" + type + "/" + number + "/mix/fader", value);
    }

    void delayOut(juce::String out, bool on) {
        this->send("/outputs/main/" + out + "/delay/on", on);
    }

    void delayOut(juce::String out, float time) {
        time = (time - 0.2) / 500;
        this->send("/outputs/main/" + out + "/delay/time", time);
        DBG(time);
    }

    void fx(int number, int parameter, float value) {
        if (parameter < 10)
        {
            this->send("/fx/" + juce::String(number) + "/par/0" + juce::String(parameter), value);
        }
        else {
            this->send("/fx/" + juce::String(number) + "/par/" + juce::String(parameter), value);
        }
	}

    void sendCh1() {
        if (this->send("/ch/01/mix/fader", (float)0.8250)) {
            DBG("Ch1 sent!");
        }
    }

    void sendCh1(float level) {
        if (this->send("/ch/01/mix/fader", level)) {
            //DBG("Ch1 sent!");
        }
    }

    void askCh1() {
        if (this->send("/ch/01/mix/fader")) {
			DBG("Ch1 asked!");
		}
	}

    void sendInfo() {
        if (this->send("/info")) {
            DBG("Info sent!");
        }
    }

    void sendStatus() {
        if (this->send("/status")) {
            DBG("Status sent!");
        }
    }

    void sendCustom(const juce::OSCAddressPattern& address, juce::String args) {
        if (this->send(address, args)) {
			DBG("Custom command sent!");
		}
    }

    void resetChEq(std::string ch){
        for (int i = 1; i <= 4; i++)
        {
            std::string gainPath = "/ch/" + ch + "/eq/" + std::to_string(i) + "/g";
            juce::OSCAddressPattern gainPattern(gainPath);
            this->sendCustom(gainPattern, "0");
        }
    }

    void resetStEq() {
        for (int i = 1; i <= 6; i++)
        {
            std::string gainPath = "/main/st/eq/" + std::to_string(i) + "/g";
            juce::OSCAddressPattern gainPattern(gainPath);
            this->sendCustom(gainPattern, "0");
        }
    }

    void initX32() {
        sendCustom("/main/st/mix/on", "1");
        sendCustom("/main/st/mix/fader", "-90.0");
        resetStEq();
        sendCustom("/bus/12/mix/on", "1");
        sendCustom("/config/osc/level", "0.3750");
        sendCustom("/config/osc/type", "1");
        sendCustom("/config/osc/dest", "11");
        sendCustom("/-stat/osc/on", "1");
        sendCustom("/bus/12/mix/fader", "0.7478");
        sendCustom("/outputs/main/02/src", "15"); // Send MixBus 12 to OUT2
        sendCustom("/ch/02/source", "60");
        sendCustom("/ch/02/preamp/trim", "0.0");
        sendCustom("/ch/02/mix/on", "1");
        sendCustom("/ch/02/mix/fader", "0.0");
        resetChEq("02");
        resetChEq("01");
        sendCustom("/outputs/main/01/src", "26"); // Send Ch 01 to OUT1
        sendCustom("/config/routing/CARD/1-8", "20");
    }

};


class ModOSCReceiver :
    public juce::OSCReceiver,
    public juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>
{
public:
    ModOSCReceiver()
    {
        // tell the component to listen for OSC messages matching this address:
        //addListener(this, "/xremote");
        addListener(this);

        /*if (this->connectToSocket(socket)) {
            DBG("OSCReceiver Connected!");
        }*/
    }

    void oscMessageReceived(const juce::OSCMessage& message) override
    {
        //int size = message.size();
        for (int i = 0; i < message.size(); i++)
        {
            if (message[i].isFloat32())
			{
				DBG("OSC Received:" << message[i].getFloat32());
			}
			else if (message[i].isInt32())
			{
				DBG("OSC Received:" << message[i].getInt32());
			}
			else if (message[i].isString())
			{
				DBG("OSC Received:" << message[i].getString());
			}
        }
        //DBG("OSC Received:" << message.begin()->getString());
        /*DBG("Message received");
        if (message.size() == 1 && message[0].isFloat32())
            DBG("Message received + if");*/
    }

    void oscBundleReceived(const juce::OSCBundle& bundle) override
    {
        DBG("Bundle received");
    }
};




class OSCSetup
{
private:
    /*static OSCSetup* OSCinstance;*/
public:
    OSCSetup()
    {
        // Create a socket that will listen to the given port number
        socket.bindToPort(10022);
        socket.setEnablePortReuse(true);
        if (OSCSender.connectToSocket(socket, "169.254.121.37", 10023)) {
			DBG("OSCSender Connected!");
		}
        if (OSCReceiver.connectToSocket(socket)) {
            DBG("OSCReceiver Connected!");
        }
    }

    void changeIPAddress() {
        if (OSCSender.connectToSocket(socket, IPAddress, 10023)) {
            DBG("IP Address changed to: " + IPAddress);
        }
        else {
            DBG("IP Address NOT changed");
        }
	}

    /*static OSCSetup* getInstance()
	    {
		    if (OSCinstance == nullptr)
		    {
			    OSCinstance = new OSCSetup();
		    }
		    return OSCinstance;
	    }*/

    juce::DatagramSocket socket;
    ModOSCSender OSCSender;
    ModOSCReceiver OSCReceiver;
    juce::String IPAddress = "169.254.121.37";
};

