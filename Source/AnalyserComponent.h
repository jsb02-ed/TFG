/*
  ==============================================================================

    AnalyserComponent.h
    Created: 2 May 2024 9:37:34pm
    Author:  josep

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MainComponent.h"

//==============================================================================
class AnalyserComponent : public juce::AudioAppComponent,
    private juce::Timer,
    public juce::Button::Listener,
    public juce::Slider::Listener
{
public:
    AnalyserComponent()
        : forwardFFT(fftOrder),
        forwardFFT2(fftOrder),
        window(fftSize, juce::dsp::WindowingFunction<float>::hann)

    {
        setOpaque(true);
        startTimer(50);
        setSize(700, 500);

        // Add buttons and sliders
        addAndMakeVisible(detectButton);
        detectButton.setButtonText("Detect");
        detectButton.addListener(this);

        addAndMakeVisible(applyButton);
        applyButton.setButtonText("Apply");
        applyButton.addListener(this);

        addAndMakeVisible(resetButton);
        resetButton.setButtonText("Reset");
        resetButton.addListener(this);

        addAndMakeVisible(maxClustersSlider);
        maxClustersSlider.setRange(1, 5, 1); // Range from 1 to 5, with step size of 1
        maxClustersSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        maxClustersSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
        maxClustersSlider.setValue(5); // Default value
        maxClustersSlider.addListener(this);

        addAndMakeVisible(maxClustersLabel);
        maxClustersLabel.setText("Maximum Filters", juce::dontSendNotification);
        maxClustersLabel.setJustificationType(juce::Justification::centred);
        maxClustersLabel.attachToComponent(&maxClustersSlider, true);

        //addAndMakeVisible(thresholdSlider);
        addChildComponent(thresholdSlider);
        thresholdSlider.setSliderStyle(juce::Slider::LinearVertical);
        thresholdSlider.setRange(0.5, 1.0);
        thresholdSlider.setTextValueSuffix("");
        thresholdSlider.addListener(this);

        addAndMakeVisible(showThresholdButton);
        showThresholdButton.setButtonText("Show Threshold");
        showThresholdButton.addListener(this);

    }

    ~AnalyserComponent() override
    {
        shutdownAudio();
    }

    //==============================================================================

    void prepareToPlay(int, double) override {}
    void releaseResources() override {}
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {}

    //==============================================================================
    
    // JUCE GUI functions ==========================================================
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        g.setOpacity(1.0f);
        g.setColour(juce::Colours::white);
        drawFrame(g,mode);
    }

    // Set position and size of buttons and sliders
    void resized() override {
        detectButton.setBounds(getWidth() - 330, 10, 100, 30);
		applyButton.setBounds(getWidth() - 220, 10, 100, 30);
        resetButton.setBounds(getWidth() - 110, 10, 100, 30);
        maxClustersSlider.setBounds(getWidth() - 670, 10, 300, 30);
        maxClustersLabel.setBounds(getWidth() - 655, 17, 100, 30);
		thresholdSlider.setBounds(0, 0, 30, getHeight() / 4);
        showThresholdButton.setBounds(60, 0, 100, 30);
    }

    // Manage button clicks
    void buttonClicked(juce::Button* button) override {
        if (button == &detectButton)
        {
            autoMagnitude(0);
        }
        else if (button == &applyButton)
        {
            autoMagnitude(1);
        }
        else if (button == &resetButton)
        {
			magnitudeDetected = false;
			clusters.clear();
            OSCEngine->OSCSender.resetStEq();
		}
        else if (button == &showThresholdButton)
        {
            if (showThresholdButton.getToggleState())
            {
				thresholdSlider.setVisible(true);
                showThreshold = true;
			}
            else
            {
				thresholdSlider.setVisible(false);
                showThreshold = false;
			}
		}
    };

    // Manage slider changes
    void sliderValueChanged(juce::Slider* slider) override {
        if (slider == &maxClustersSlider)
        {
            maxClusters = maxClustersSlider.getValue();
        }
        else if (slider == &thresholdSlider)
        {
            clusterThreshold = thresholdSlider.getValue();
        }
    };

    // END OF JUCE GUI functions ===================================================

    // FFT functions ===============================================================
    // Timer that manages when to draw the next frame of the spectrum
    void timerCallback() override
    {
        if (nextFFTBlockReady)
        {
            drawNextFrameOfSpectrum(mode);
            nextFFTBlockReady = false;
            repaint();
        }
    }

    // FIFO buffer for single channel mode (not in use)
    void pushNextSampleIntoFifo(float sample) noexcept
    {
        // if the fifo contains enough data, set a flag to say
        // that the next frame should now be rendered..
        if (fifoIndex == fftSize)
        {
            if (!nextFFTBlockReady)
            {
                juce::zeromem(fftData, sizeof(fftData));
                memcpy(fftData, fifo, sizeof(fifo));
                nextFFTBlockReady = true;
            }
            fifoIndex = 0;
        }
        fifo[fifoIndex++] = sample;
    }

    // FIFO buffer for dual channel mode
    void pushNextSampleIntoFifo(float sample, float sample2) noexcept
    {
        // if the fifo contains enough data, set a flag to say
        // that the next frame should now be rendered..
        if (fifoIndex == fftSize)
        {
            if (!nextFFTBlockReady)
            {
                juce::zeromem(fftData, sizeof(fftData));
                juce::zeromem(fftData2, sizeof(fftData2));
                juce::zeromem(fftInput, sizeof(fftInput)); // !
                juce::zeromem(fftInput2, sizeof(fftInput2)); // !
                //memcpy(fftData, fifo, sizeof(fifo));
                //memcpy(fftData2, fifo2, sizeof(fifo2));
                for (int i = 0; i < fftSize; ++i) // ---------------------!
                {
                    fftInput[i] = std::complex<float>(fifo[i], 0.0f);
                    fftInput2[i] = std::complex<float>(fifo2[i], 0.0f);
                } // ----------------------------------------------------!
                nextFFTBlockReady = true;
            }
            fifoIndex = 0;
        }
        fifo[fifoIndex] = sample;
        fifo2[fifoIndex] = sample2;
        fifoIndex++;
    }

    void drawNextFrameOfSpectrum(int mode = 0)
    {
        // Single channel mode (not in use)
        //if (mode == 0)
        //{
        //    // First apply a windowing function to our data
        //    window.multiplyWithWindowingTable(fftData, fftSize);

        //    // Then render our FFT data
        //    forwardFFT.performFrequencyOnlyForwardTransform(fftData);

        //    auto mindB = -100.0f;
        //    auto maxdB = 0.0f;

        //    for (int i = 0; i < scopeSize; ++i)
        //    {
        //        auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * 0.2f);
        //        auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.5f));
        //        auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(fftData[fftDataIndex])
        //            - juce::Decibels::gainToDecibels((float)fftSize)),
        //            mindB, maxdB, 0.0f, 1.0f);

        //        scopeData[i] = level;
        //    }
        //}

        // Dual channel mode
        /*else*/ if (mode == 1) {
            // First apply a windowing function to our data
            //window.multiplyWithWindowingTable(fftData, fftSize);
            //window.multiplyWithWindowingTable(fftData2, fftSize);
            window.multiplyWithWindowingTable(reinterpret_cast<float*>(fftInput), fftSize); //!
            window.multiplyWithWindowingTable(reinterpret_cast<float*>(fftInput2), fftSize);//!

            // Then render our FFT data
            //forwardFFT.performFrequencyOnlyForwardTransform(fftData);
            //forwardFFT2.performFrequencyOnlyForwardTransform(fftData2);
            forwardFFT.perform(fftInput, fftData, false);//!
            forwardFFT2.perform(fftInput2, fftData2, false);//!

            // Define min and max dB values
            auto mindB = -60.0f;
            auto maxdB = -40.0f;

            // Define min and max frequencies
            const float minFreq = 20.0f;
            const float maxFreq = 20000.0f;
            int sampleRate = 48000;

            // Remap values from FFT data to the scope size
            for (int i = 0; i < scopeSize; ++i)
            {
                float proportionX = (float)i / (float)scopeSize;
                float logMinFreq = std::log10(minFreq);
                float logMaxFreq = std::log10(maxFreq);
                float logFreq = logMinFreq + proportionX * (logMaxFreq - logMinFreq);
                float freq = std::pow(10, logFreq);

                auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)((freq / (sampleRate / 2)) * (fftSize / 2)));

                /*auto level = juce::jmap(juce::jlimit(mindB, maxdB,
                    juce::Decibels::gainToDecibels(fftData[fftDataIndex]) -
                    juce::Decibels::gainToDecibels((float)fftSize)),
                    mindB, maxdB, 0.0f, 1.0f);*/ //----------------------!!!

                auto level = juce::jmap(juce::jlimit(mindB, maxdB,
                    juce::Decibels::gainToDecibels(std::abs(fftData[fftDataIndex])) -
                    juce::Decibels::gainToDecibels((float)fftSize)),
                    mindB, maxdB, 0.0f, 1.0f);

                /*auto level2 = juce::jmap(juce::jlimit(mindB, maxdB,
                    juce::Decibels::gainToDecibels(fftData2[fftDataIndex]) -
                    juce::Decibels::gainToDecibels((float)fftSize)),
                    mindB, maxdB, 0.0f, 1.0f);*/ //----------------------!!!

                auto level2 = juce::jmap(juce::jlimit(mindB, maxdB,
                    juce::Decibels::gainToDecibels(std::abs(fftData2[fftDataIndex])) -
                    juce::Decibels::gainToDecibels((float)fftSize)),
                    mindB, maxdB, 0.0f, 1.0f);

                rtaMeasurement[i] = level;
                rtaReference[i] = level2;

                phaseDifference[i] = std::arg(fftData[fftDataIndex] / fftData2[fftDataIndex]); //!!!!!!!!!
            }
        }

        // Calculate relative magnitude
        juce::FloatVectorOperations::subtract(magnitude, rtaMeasurement, rtaReference, scopeSize);
        juce::FloatVectorOperations::add(magnitude, 1, scopeSize);
        juce::FloatVectorOperations::multiply(magnitude, 0.5, scopeSize);

        juce::FloatVectorOperations::copy(phase, phaseDifference.data(), scopeSize);

        // Average magnitude and phase
        averageMagnPhase();

        //maxMagnitude(); //Old function no longer used

    }

    void averageMagnPhase()
    {
        for (int i = 0; i < scopeSize; i++)
        {
            averageMagnitudeFifo.add(magnitude[i]);
            averagePhaseFifo.add(phase[i]);
        }
        if (averageMagnitudeFifo.size() > averageFifoSize) {
            for (int i = 0; i < scopeSize; ++i)
		    {
			    averageMagnitudeOut[i] = 0;
                averagePhaseOut[i] = 0;
			    for (int j = 0; j < averageNumber; ++j)
			    {
                    averageMagnitudeOut[i] = averageMagnitudeOut[i] + averageMagnitudeFifo[j * scopeSize + i];
                    averagePhaseOut[i] = averagePhaseOut[i] + averagePhaseFifo[j * scopeSize + i];
			    }
                averageMagnitudeOut[i] /= averageNumber;
                averagePhaseOut[i] /= averageNumber;
		    }
            for (int i = 0; i < scopeSize; ++i)
            {
                averageMagnitudeFifo.removeAndReturn(i); 
                averagePhaseFifo.removeAndReturn(i);
            }
            //averageMagnitudeFifo.clear();
            //averagePhaseFifo.clear();
        } 
    }

    // END OF FFT functions ========================================================

    // Old function no longer used
    /*void maxMagnitude()
	{
        avgMagnitude.clear();
        avgMagnitudeSorted.clear();

        for (int i = 0; i < scopeSize; i++)
        {
            avgMagnitude.add(averageMagnitudeOut[i]);
            avgMagnitudeSorted.add(averageMagnitudeOut[i]);
        }

        avgMagnitudeSorted.sort();

        float max = avgMagnitudeSorted[scopeSize - 1];
        float min = avgMagnitudeSorted[0];
        maxIdx = avgMagnitude.indexOf(max);
        minIdx = avgMagnitude.indexOf(min);
	}*/

    // FFT plotting functions ======================================================

    void drawFrame(juce::Graphics& g, int mode = 0)
    {
        auto width = getLocalBounds().getWidth();
        auto height = getLocalBounds().getHeight() / 2;

        // Draw the scope
        // Single channel mode
        if (mode == 0)
        {
            for (int i = 1; i < scopeSize; ++i)
            {
                g.drawLine({ (float)juce::jmap(i - 1, 0, scopeSize - 1, 0, width),
                                      juce::jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
                              (float)juce::jmap(i,     0, scopeSize - 1, 0, width),
                                      juce::jmap(scopeData[i],     0.0f, 1.0f, (float)height, 0.0f) });
            }
        }

        // Dual channel mode
        else if (mode == 1) {

            // Draw grid
            g.setColour(juce::Colours::white);
            drawFrequencyScale(g, width, height);
            drawAmplitudeScale(g, width, height);
            drawFrequencyScale(g, width, height * 2);
            drawPhaseScale(g, width, height);

            for (int i = 1; i < scopeSize; ++i)
            {
                // Set the color deppending on the cluster
                juce::Colour colour = belongs2cluster(i - 1);
                g.setColour(colour);

                // Draw the measurement curve
                float x1 = juce::jmap((float)(i - 1), 0.0f, (float)(scopeSize - 1), 0.0f, (float)width);
                float y1 = juce::jmap(averageMagnitudeOut[i - 1], 0.0f, 1.0f, (float)height, 0.0f);
                float x2 = juce::jmap((float)i, 0.0f, (float)(scopeSize - 1), 0.0f, (float)width);
                float y2 = juce::jmap(averageMagnitudeOut[i], 0.0f, 1.0f, (float)height, 0.0f);
                g.drawLine(x1, y1, x2, y2, 3);

                // Draw the threshold line
                if (showThreshold)
                {
                    g.setColour(juce::Colours::purple);
                    float thresholdY = juce::jmap(clusterThreshold, 0.5f, 1.0f, (float)height / 2, 0.0f);
                    g.drawHorizontalLine(thresholdY, 0.0f, (float)width);
                    //Following 2 lines create a dashed line. Unabled because it slows down the GUI
                    //const float dashLengths[2] = { 4, 5 };
                    //g.drawDashedLine(juce::Line<float>(0.0f, thresholdY, (float)width, thresholdY), dashLengths, 2);
                }
            }
            // Draw phase -------------------------!!!!
            float v_offset = getHeight() / 2;
            g.setColour(juce::Colours::red);
            for (int i = 1; i < scopeSize; ++i)
            {
                float x1 = juce::jmap((float)(i - 1), 0.0f, (float)(scopeSize - 1), 0.0f, (float)width);
                float y1 = juce::jmap(averagePhaseOut[i - 1], -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, (float)height, 0.0f) + v_offset;
                float x2 = juce::jmap((float)i, 0.0f, (float)(scopeSize - 1), 0.0f, (float)width);
                float y2 = juce::jmap(averagePhaseOut[i], -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, (float)height, 0.0f) + v_offset;

                // Check for phase wrap
                const float phaseWrapThreshold = juce::MathConstants<float>::pi * 0.7f; // Add some tolerance
                if (std::abs(averagePhaseOut[i] - averagePhaseOut[i - 1]) < phaseWrapThreshold)
                {
                    g.drawLine(x1, y1, x2, y2, 2);
                }
            }
        }
        g.setColour(juce::Colours::white);
    }

    void drawFrequencyScale(juce::Graphics& g, int width, int height)
    {
        // Define min and max frequencies
        const float minFreq = 20.0f;
        const float maxFreq = 20000.0f;

        // Define the range of the logarithmic scale
        float logMinFreq = std::log10(minFreq);
        float logMaxFreq = std::log10(maxFreq);

        // Number of divisions on the x-axis
        const int numDivisions = 10;

        // Draw the x-axis line
        g.setColour(juce::Colours::white);
        g.drawLine(0, height - 20, width, height - 20);

        // Draw frequency labels
        for (int i = 0; i <= numDivisions; ++i)
        {
            float proportionX = (float)i / (float)numDivisions;
            float logFreq = logMinFreq + proportionX * (logMaxFreq - logMinFreq);
            float freq = std::pow(10, logFreq);

            // Calculate x position for this frequency
            int x = proportionX * width;

            // Draw a tick mark
            g.drawLine(x, height - 20, x, height - 10);

            // Draw the frequency label
            juce::String freqLabel = (freq >= 1000.0f) ? juce::String(freq / 1000.0f, 1) + " kHz" : juce::String((int)freq) + " Hz";
            if (i==0) g.drawText(freqLabel, x, height - 10, 60, 10, juce::Justification::left);
			else if (i == numDivisions) g.drawText(freqLabel, x - 60, height - 10, 60, 10, juce::Justification::right);
			else g.drawText(freqLabel, x - 30, height - 10, 60, 10, juce::Justification::centred);
        }
    }

    void drawAmplitudeScale(juce::Graphics& g, int width, int height)
    {
        // Define min and max amplitudes
        const float minAmp = 18;
        const float maxAmp = -18;

        // Number of divisions on the y-axis
        const int numDivisions = 12;

        // Draw amplitude labels
        for (int i = 1; i <= numDivisions - 1; ++i)
        {
            float proportionX = (float)i / (float)numDivisions;
            float amp = minAmp + proportionX * (maxAmp - minAmp);

            // Calculate x position for this frequency
            int y = proportionX * height;

            // Draw an horizontal line
            g.setColour(juce::Colours::lightgrey);
            g.drawLine(0, y, width, y, 0.5);
            g.setColour(juce::Colours::white);

            // Draw the frequency label
            juce::String ampLabel = juce::String(amp) + " dB"; 
            if (i == numDivisions/2) g.drawText(ampLabel, 5, y - 15, 60, 10, juce::Justification::centredLeft);
            else g.drawText(ampLabel, 5, y - 5, 60, 10, juce::Justification::centredLeft);
        }
    }

    void drawPhaseScale(juce::Graphics& g, int width, int height)
    {
        // Define min and max angles
        const float minPh = 225;
        const float maxPh = -225;

        // Number of divisions on the y-axis
        const int numDivisions = 10;

        // Draw phase labels
        for (int i = 1; i <= numDivisions - 1; ++i)
        {
            float proportionX = (float)i / (float)numDivisions;
            float ph = minPh + proportionX * (maxPh - minPh);

            // Calculate x position for this frequency
            int y = proportionX * height + height;

            // Draw an horizontal line
            g.setColour(juce::Colours::lightgrey);
            g.drawLine(0, y, width, y, 0.5);
            g.setColour(juce::Colours::white);

            // Draw the frequency label
            juce::String ampLabel = juce::String(ph) + " deg";
            if (i == numDivisions / 2) g.drawText(ampLabel, 5, y - 15, 60, 10, juce::Justification::centredLeft);
            else g.drawText(ampLabel, 5, y - 5, 60, 10, juce::Justification::centredLeft);
        }
    }

    // END OF FFT plotting functions ===============================================

    // Converts a bin in the scope to a frequency
    float bin2freq(int bin) {
        // Define min and max frequencies
        const float minFreq = 20.0f;
        const float maxFreq = 20000.0f;

        // Convert the value to a proportion (0.0 to 1.0)
        float proportionX = (float)bin / (float)scopeSize;

        // Define the range of the logarithmic scale
        float logMinFreq = std::log10(minFreq);
        float logMaxFreq = std::log10(maxFreq);

        // Calculate the logarithmic frequency
        float logFreq = logMinFreq + proportionX * (logMaxFreq - logMinFreq);
        float freq = std::pow(10, logFreq);

        return freq;
    }

    std::vector<int> findSignificantPeaks(const std::vector<float>& data, float threshold)
    {
        std::vector<int> peaks;

        for (int i = 1; i < data.size() - 1; ++i)
        {
            if (std::fabs(data[i]) > threshold &&
                std::fabs(data[i]) > std::fabs(data[i - 1]) &&
                std::fabs(data[i]) > std::fabs(data[i + 1]))
            {
                peaks.push_back(i);
            }
        }

        return peaks;
    }

    std::vector<std::vector<int>> groupPeaksIntoClusters(const std::vector<int>& peaks, int minDistance)
    {
        std::vector<std::vector<int>> clusters;
        if (peaks.empty()) return clusters;

        std::vector<int> currentCluster = { peaks[0] };

        for (int i = 1; i < peaks.size(); ++i)
        {
            if (peaks[i] - peaks[i - 1] <= minDistance)
            {
                currentCluster.push_back(peaks[i]);
            }
            else
            {
                clusters.push_back(currentCluster);
                currentCluster = { peaks[i] };
            }
        }

        clusters.push_back(currentCluster);
        return clusters;
    }

    std::vector<std::vector<int>> limitClusters(std::vector<std::vector<int>>& clusters, int maxClusters)
    {
        while (clusters.size() > maxClusters)
        {
            // Find the two closest clusters
            int minDistance = std::numeric_limits<int>::max();
            int mergeIdx1 = 0;
            int mergeIdx2 = 1;

            for (int i = 0; i < clusters.size() - 1; ++i)
            {
                int distance = clusters[i + 1].front() - clusters[i].back();
                if (distance < minDistance)
                {
                    minDistance = distance;
                    mergeIdx1 = i;
                    mergeIdx2 = i + 1;
                }
            }

            // Merge the two closest clusters
            clusters[mergeIdx1].insert(clusters[mergeIdx1].end(), clusters[mergeIdx2].begin(), clusters[mergeIdx2].end());
            clusters.erase(clusters.begin() + mergeIdx2);
        }

        return clusters;
    }

    void applyFilters(std::vector<std::vector<int>>& clusters, std::vector<float>& data)
    {
        for (int i = 0; i < clusters.size(); ++i)
        {
            //for (int idx : clusters[i])
            //{
                //if (clusters[i].size() > 5)
                //{
                    int peaks = clusters[i].size();
                    int sum = std::accumulate(clusters[i].begin(), clusters[i].end(), 0);
                    int midIndex = sum / peaks;
                    float freq = bin2freq(midIndex);

                    DBG("Cluster " << i << ": " << peaks << " peaks, midIndex = " << midIndex << ", freq = " << freq << ", value = " << averageMagnitudeOut[midIndex]);

                    // Set Quality factor
                    float quality;
                    if (peaks == 1) {
                        quality = 4.0f;
                    }
                    else if (peaks >= 1 && peaks <= 4) {
                        quality = 2.2f;
                    }
                    else if (peaks >= 5 && peaks <= 14) {
                        quality = 1.5f;
                    }
                    else {
                        quality = 0.5f;
                    }

                    // Set Gain
                    float gain;
                    if (averageMagnitudeOut[midIndex] > 0.71f) {
						gain = -15.0f;
					}
                    else if (averageMagnitudeOut[midIndex] > 0.68f) {
						gain = -10.0f;
					}
                    else if (averageMagnitudeOut[midIndex] > 0.52) {
                        gain = -5.0f;
                    }
                    else {
                        gain = 0.0f;
                    }

                    // Construct the OSC message paths with the current cluster index i
                    std::string freqPath = "/main/st/eq/" + std::to_string(i+2) + "/f";
                    std::string qualityPath = "/main/st/eq/" + std::to_string(i+2) + "/q";
                    std::string gainPath = "/main/st/eq/" + std::to_string(i+2) + "/g";

                    // Convert std::string to juce::OSCAddressPattern
                    juce::OSCAddressPattern freqPattern(freqPath);
                    juce::OSCAddressPattern qualityPattern(qualityPath);
                    juce::OSCAddressPattern gainPattern(gainPath);

                    // Send OSC messages
                    OSCEngine->OSCSender.sendCustom(freqPattern, juce::String(freq));
                    OSCEngine->OSCSender.sendCustom(qualityPattern, juce::String(quality));
                    OSCEngine->OSCSender.sendCustom(gainPattern, juce::String(gain));

                //}
            //}
        }
    }

    juce::Colour belongs2cluster(int index) {
        for (int i = 0; i < clusters.size(); ++i)
        {
            for (int idx : clusters[i])
            {
                if (index == idx)
                {
                    switch (i)
                    {
                        case 0:
						    return juce::Colours::aqua;
                        case 1:
                            return juce::Colours::hotpink;
                        case 2:
                            return juce::Colours::orange;
                        case 3:
                            return juce::Colours::green;
                        case 4:
                            return juce::Colours::red;
                    }
                }
            }
        }
        return juce::Colours::royalblue;
    }



    void autoMagnitude(bool apply) {

        if (!apply)
        {
            data = std::vector<float>(averageMagnitudeOut, averageMagnitudeOut + scopeSize);
            float threshold = clusterThreshold; // Threshold to detect significant peaks
            int minDistance = 0; // Minimum distance between peaks to consider them in the same cluster
            //int maxClusters = 5; // Maximum number of clusters

            // Step 1: Identify significant peaks
            peaks = findSignificantPeaks(data, threshold);

            // Step 2: Group peaks into clusters
            clusters = groupPeaksIntoClusters(peaks, minDistance);

            // Step 3: Limit the number of clusters
            clusters = limitClusters(clusters, maxClusters);

            // Step 4: Set flag to true
            magnitudeDetected = true;

            return;
        }
        if (magnitudeDetected)
		{
			// Step 5: Apply filters to the data within each cluster
            applyFilters(clusters, data);
            DBG("Filters applied!");
            return;
		}
        DBG("You must detect magnitude before applying filters!");
    }


    enum
    {
        fftOrder = 13,
        fftSize = 1 << fftOrder,
        scopeSize = 201,
        averageNumber = 20,
        averageFifoSize = scopeSize * averageNumber
    };

private:
    juce::dsp::FFT forwardFFT;
    juce::dsp::FFT forwardFFT2;
    juce::dsp::WindowingFunction<float> window;

    float fifo[fftSize];
    float fifo2[fftSize];
    //float fftData[2 * fftSize];
    //float fftData2[2 * fftSize];
    std::complex<float> fftInput[fftSize]; // -------!
    std::complex<float> fftInput2[fftSize];// -------!
    std::complex<float> fftData[fftSize]; // -------!
    std::complex<float> fftData2[fftSize];// -------!
    int fifoIndex = 0;
    std::array<float, fftSize / 2> phaseDifference;

    bool nextFFTBlockReady = false;
    float scopeData[scopeSize];
    float rtaMeasurement[scopeSize];
    float rtaReference[scopeSize];
    float magnitude[scopeSize];
    float phase[scopeSize]; // -----------------!
    int fifoAverageIndex = 0;
    juce::Array<float> averageMagnitudeFifo;
    juce::Array<float> averagePhaseFifo;
    float averageMagnitudeOut[scopeSize];
    float averagePhaseOut[scopeSize];
    juce::Array<float> avgMagnitude;
    juce::Array<float> avgMagnitudeSorted;
    int maxIdx = -1;
    int minIdx = -1;
    OSCSetup* OSCEngine = new OSCSetup();
    
    float clusterThreshold = 0.5f;
    bool magnitudeDetected = false;
    int maxClusters = 5;
    std::vector<float> data;
    std::vector<int> peaks;
    std::vector<std::vector<int>> clusters;

    bool showThreshold = false;

    // Buttons and labels
    juce::TextButton detectButton;
    juce::TextButton applyButton;
    juce::TextButton resetButton;
    juce::Slider maxClustersSlider;
    juce::Label maxClustersLabel;
    juce::Slider thresholdSlider;
    juce::Label  thresholdLabel;
    juce::ToggleButton showThresholdButton;

    int mode = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalyserComponent)
};
