/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LadderFilterAudioProcessorEditor::LadderFilterAudioProcessorEditor (LadderFilterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    shadowProperties.radius = 24;
    shadowProperties.offset = juce::Point<int> (-1, 3);
    dialShadow.setShadowProperties (shadowProperties);
            
    sliders.reserve(3);
    sliders = {
        &driveSlider, &cutoffSlider, &resoSlider
    };
            
    labels.reserve(3);
    labels = {
        &driveLabel, &cutoffLabel, &resoLabel
    };
                
    labelTexts.reserve(3);
    labelTexts = {
        driveLabelText, cutoffLabelText, resoLabelText
    };
    
    for (auto i = 0; i < sliders.size(); i++) {
        addAndMakeVisible(sliders[i]);
        sliders[i]->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        sliders[i]->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 32);
        sliders[i]->setColour(0x1001400, juce::Colour::fromFloatRGBA(1, 1, 1, 0.25f));
        sliders[i]->setColour(0x1001700, juce::Colour::fromFloatRGBA(1, 1, 1, 0.0f));
        sliders[i]->setColour(0x1001500, juce::Colour::fromFloatRGBA(0, 0, 0, 0.25f));
        sliders[i]->setLookAndFeel(&customDial);
        sliders[i]->setComponentEffect(&dialShadow);
        }
            
    
    driveSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, driveSliderId, driveSlider);
    resoSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, resoDelaySliderId, resoSlider);
    cutoffSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, cutoffSliderId, cutoffSlider);
    
    driveSlider.setRange(1, 10, 0.25);
    driveSlider.setTextValueSuffix(" dB");
    cutoffSlider.setRange(20, 20000, 1);
    cutoffSlider.setTextValueSuffix(" Hz");
    cutoffSlider.setSkewFactorFromMidPoint(750);
    resoSlider.setRange(0, 1, 0.025);
    resoSlider.setTextValueSuffix(" dB");
    
    
    for (auto i = 0; i < labels.size(); i++) {
        addAndMakeVisible(labels[i]);
        labels[i]->setText(labelTexts[i], juce::dontSendNotification);
        labels[i]->setJustificationType(juce::Justification::centred);
        labels[i]->setColour(0x1000281, juce::Colour::fromFloatRGBA(1, 1, 1, 0.25f));
        labels[i]->attachToComponent(sliders[i], false);
        }
    
    addAndMakeVisible(windowBorder);
    windowBorder.setText("The Ladder");
    windowBorder.setColour(0x1005400, juce::Colour::fromFloatRGBA(1, 1, 1, 0.25f));
    windowBorder.setColour(0x1005410, juce::Colour::fromFloatRGBA(1, 1, 1, 0.25f));
            
    //Making the window resizable by aspect ratio and setting size
    AudioProcessorEditor::setResizable(true, true);
    AudioProcessorEditor::setResizeLimits(600, 200, 900, 300);
    AudioProcessorEditor::getConstrainer()->setFixedAspectRatio(3.0);
    setSize (750, 250);
}

LadderFilterAudioProcessorEditor::~LadderFilterAudioProcessorEditor()
{
}

//==============================================================================
void LadderFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromFloatRGBA(0.1f, 0.12f, 0.16f, 1.0));
                   
    juce::Rectangle<int> background = AudioProcessorEditor::getLocalBounds();
    g.setGradientFill(juce::ColourGradient::vertical(juce::Colour::fromFloatRGBA(0.18f, 0.20f, 0.24f, 1.0), 300 * .01, juce::Colour::fromFloatRGBA(0.12f, 0.14f, 0.18f, 1.0), 300 * .99));
    g.fillRect(background);
}

void LadderFilterAudioProcessorEditor::resized()
{
    //Master bounds object
       juce::Rectangle<int> bounds = getLocalBounds();

       //first column of gui
       juce::FlexBox flexboxColumnOne;
       flexboxColumnOne.flexDirection = juce::FlexBox::Direction::column;
       flexboxColumnOne.flexWrap = juce::FlexBox::Wrap::noWrap;
       flexboxColumnOne.alignContent = juce::FlexBox::AlignContent::stretch;

       juce::Array<juce::FlexItem> itemArrayColumnOne;
       itemArrayColumnOne.add(juce::FlexItem(bounds.getWidth() / 3, bounds.getHeight() / 2, driveSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .30, 0, 0, 0)));

       flexboxColumnOne.items = itemArrayColumnOne;
       flexboxColumnOne.performLayout(bounds.removeFromLeft(bounds.getWidth() / 3));
       /* ============================================================================ */

       //second column of gui
       juce::FlexBox flexboxColumnTwo;
       flexboxColumnTwo.flexDirection = juce::FlexBox::Direction::column;
       flexboxColumnTwo.flexWrap = juce::FlexBox::Wrap::noWrap;
       flexboxColumnTwo.alignContent = juce::FlexBox::AlignContent::stretch;

       juce::Array<juce::FlexItem> itemArrayColumnTwo;
       itemArrayColumnTwo.add(juce::FlexItem(bounds.getWidth() / 3, bounds.getHeight() / 2, cutoffSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .30, 0, 0, 0)));

       flexboxColumnTwo.items = itemArrayColumnTwo;
       flexboxColumnTwo.performLayout(bounds.removeFromLeft(bounds.getWidth() / 2));
       /* ============================================================================ */

       //third column of gui
       juce::FlexBox flexboxColumnThree;
       flexboxColumnThree.flexDirection = juce::FlexBox::Direction::column;
       flexboxColumnThree.flexWrap = juce::FlexBox::Wrap::noWrap;
       flexboxColumnThree.alignContent = juce::FlexBox::AlignContent::stretch;

       juce::Array<juce::FlexItem> itemArrayColumnThree;
       itemArrayColumnThree.add(juce::FlexItem(bounds.getWidth() / 3, bounds.getHeight() / 2, resoSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .30, 0, 0, 0)));

       flexboxColumnThree.items = itemArrayColumnThree;
       flexboxColumnThree.performLayout(bounds.removeFromLeft(bounds.getWidth()));
       /* ============================================================================ */

       windowBorder.setBounds(AudioProcessorEditor::getWidth() * .01, AudioProcessorEditor::getHeight() * 0.04, AudioProcessorEditor::getWidth() * .98, AudioProcessorEditor::getHeight() * .90);
}
