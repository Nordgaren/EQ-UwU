/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

struct ChainSettings
{
    float peakFreq{ 0 }, peakGainInDecibels{ 0 }, peakQuality{ 1.f };
    float lowCutFreq{ 0 }, highCutFreq{ 0 };
    
    Slope lowCutSlope{ Slope::Slope_12 }, highCutSlope{ Slope::Slope_12 };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class EQUwUAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    EQUwUAudioProcessor();
    ~EQUwUAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters", createParameterLayout()};



private:
    using Filter = juce::dsp::IIR::Filter<float>;

    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

    MonoChain leftChain, rightChain;

    enum ChainPosition
    {
        LowCut,
        Peak,
        HighCut
    };

    void updatePeakFilter(const ChainSettings& chainSettings);
    using Coefficients = Filter::CoefficientsPtr;
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);

    template<typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& directionLowCut,
                        const CoefficientType& cutCoefficients,
                        //const ChainSettings& chainSettings
                        const Slope& lowCutSlope)
    {
        directionLowCut.template setBypassed<0>(true);
        directionLowCut.template setBypassed<1>(true);
        directionLowCut.template setBypassed<2>(true);
        directionLowCut.template setBypassed<3>(true);

        switch (lowCutSlope)
        {
        case Slope_12:
            *directionLowCut.template get<0>().coefficients = *cutCoefficients[0];
            directionLowCut.template setBypassed<0>(false);
            break;
        case Slope_24:
            *directionLowCut.template get<0>().coefficients = *cutCoefficients[0];
            directionLowCut.template setBypassed<0>(false);
            *directionLowCut.template get<1>().coefficients = *cutCoefficients[1];
            directionLowCut.template setBypassed<1>(false);
            break;
        case Slope_36:
            *directionLowCut.template get<0>().coefficients = *cutCoefficients[0];
            directionLowCut.template setBypassed<0>(false);
            *directionLowCut.template get<1>().coefficients = *cutCoefficients[1];
            directionLowCut.template setBypassed<1>(false);
            *directionLowCut.template get<2>().coefficients = *cutCoefficients[2];
            directionLowCut.template setBypassed<2>(false);
            break;
        case Slope_48:
            *directionLowCut.template get<0>().coefficients = *cutCoefficients[0];
            directionLowCut.template setBypassed<0>(false);
            *directionLowCut.template get<1>().coefficients = *cutCoefficients[1];
            directionLowCut.template setBypassed<1>(false);
            *directionLowCut.template get<2>().coefficients = *cutCoefficients[2];
            directionLowCut.template setBypassed<2>(false);
            *directionLowCut.template get<3>().coefficients = *cutCoefficients[3];
            directionLowCut.template setBypassed<3>(false);
            break;
        }
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQUwUAudioProcessor)
};
