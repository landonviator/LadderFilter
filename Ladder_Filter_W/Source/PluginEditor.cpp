/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Ladder_Filter_WAudioProcessorEditor::Ladder_Filter_WAudioProcessorEditor(Ladder_Filter_WAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    shadowProperties.radius = 24;
    shadowProperties.offset = juce::Point<int>(-1, 3);
    dialShadow.setShadowProperties(shadowProperties);

    sliders.reserve(4);
    sliders = {
        &driveSlider, &cutoffSlider, &resoSlider, &trimSlider
    };

    labels.reserve(4);
    labels = {
        &driveLabel, &cutoffLabel, &resoLabel, &trimLabel
    };

    labelTexts.reserve(4);
    labelTexts = {
        driveLabelText, cutoffLabelText, resoLabelText, trimLabelText
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
    trimSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, trimSliderId, trimSlider);

    driveSlider.setRange(0, 10, 0.25);
    driveSlider.setTextValueSuffix(" dB");
    cutoffSlider.setRange(20, 20000, 1);
    cutoffSlider.setTextValueSuffix(" Hz");
    cutoffSlider.setSkewFactorFromMidPoint(750);
    resoSlider.setRange(0, 1, 0.025);
    resoSlider.setTextValueSuffix(" dB");
    trimSlider.setRange(-36.0, 36.0);
    trimSlider.setTextValueSuffix(" dB");


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
    setSize(750, 250);
}

Ladder_Filter_WAudioProcessorEditor::~Ladder_Filter_WAudioProcessorEditor()
{
}

//==============================================================================
void Ladder_Filter_WAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromFloatRGBA(0.1f, 0.12f, 0.16f, 1.0));

    juce::Rectangle<int> background = AudioProcessorEditor::getLocalBounds();
    g.setGradientFill(juce::ColourGradient::vertical(juce::Colour::fromFloatRGBA(0.18f, 0.20f, 0.24f, 1.0), 300 * .01, juce::Colour::fromFloatRGBA(0.12f, 0.14f, 0.18f, 1.0), 300 * .99));
    g.fillRect(background);
}

void Ladder_Filter_WAudioProcessorEditor::resized()
{
    //Master bounds object
    juce::Rectangle<int> bounds = getLocalBounds();

    //first column of gui
    juce::FlexBox flexboxColumnOne;
    flexboxColumnOne.flexDirection = juce::FlexBox::Direction::column;
    flexboxColumnOne.flexWrap = juce::FlexBox::Wrap::noWrap;
    flexboxColumnOne.alignContent = juce::FlexBox::AlignContent::stretch;

    juce::Array<juce::FlexItem> itemArrayColumnOne;
    itemArrayColumnOne.add(juce::FlexItem(bounds.getWidth() / 6, bounds.getHeight() / 2, driveSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .30, 0, 0, 0)));

    flexboxColumnOne.items = itemArrayColumnOne;
    flexboxColumnOne.performLayout(bounds.removeFromLeft(bounds.getWidth() / 4));
    /* ============================================================================ */

    //second column of gui
    juce::FlexBox flexboxColumnTwo;
    flexboxColumnTwo.flexDirection = juce::FlexBox::Direction::column;
    flexboxColumnTwo.flexWrap = juce::FlexBox::Wrap::noWrap;
    flexboxColumnTwo.alignContent = juce::FlexBox::AlignContent::stretch;

    juce::Array<juce::FlexItem> itemArrayColumnTwo;
    itemArrayColumnTwo.add(juce::FlexItem(bounds.getWidth() / 6, bounds.getHeight() / 2, cutoffSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .30, 0, 0, 0)));

    flexboxColumnTwo.items = itemArrayColumnTwo;
    flexboxColumnTwo.performLayout(bounds.removeFromLeft(bounds.getWidth() / 3));
    /* ============================================================================ */

    //third column of gui
    juce::FlexBox flexboxColumnThree;
    flexboxColumnThree.flexDirection = juce::FlexBox::Direction::column;
    flexboxColumnThree.flexWrap = juce::FlexBox::Wrap::noWrap;
    flexboxColumnThree.alignContent = juce::FlexBox::AlignContent::stretch;

    juce::Array<juce::FlexItem> itemArrayColumnThree;
    itemArrayColumnThree.add(juce::FlexItem(bounds.getWidth() / 6, bounds.getHeight() / 2, resoSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .30, 0, 0, 0)));

    flexboxColumnThree.items = itemArrayColumnThree;
    flexboxColumnThree.performLayout(bounds.removeFromLeft(bounds.getWidth() / 2));
    /* ============================================================================ */

 //fourth column of gui
    juce::FlexBox flexboxColumnFour;
    flexboxColumnFour.flexDirection = juce::FlexBox::Direction::column;
    flexboxColumnFour.flexWrap = juce::FlexBox::Wrap::noWrap;
    flexboxColumnFour.alignContent = juce::FlexBox::AlignContent::stretch;

    juce::Array<juce::FlexItem> itemArrayColumnFour;
    itemArrayColumnFour.add(juce::FlexItem(bounds.getWidth() / 6, bounds.getHeight() / 2, trimSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .30, 0, 0, 0)));

    flexboxColumnFour.items = itemArrayColumnFour;
    flexboxColumnFour.performLayout(bounds.removeFromLeft(bounds.getWidth()));
    /* ============================================================================ */

    windowBorder.setBounds(AudioProcessorEditor::getWidth() * .01, AudioProcessorEditor::getHeight() * 0.04, AudioProcessorEditor::getWidth() * .98, AudioProcessorEditor::getHeight() * .90);
}
