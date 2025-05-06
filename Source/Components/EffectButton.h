/*
  ==============================================================================

    EffectButton.h
    Created: 15 Apr 2025 10:16:29pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class EffectButton : public juce::Component,
                     public juce::Timer
{
public:
  EffectButton(const juce::String &effectName);
  ~EffectButton() override;

  void paint(juce::Graphics &) override;
  void resized() override;

  // Mouse interaction
  void mouseDown(const juce::MouseEvent &e) override;
  void mouseDrag(const juce::MouseEvent &e) override;
  void mouseUp(const juce::MouseEvent &e) override;

  // Animation
  void timerCallback() override;
  void startAnimationToPosition(int targetY);
  void setTargetPosition(int targetY);
  bool isAnimating() const { return animating; }
  bool isDragging() const { return dragging; }
  int getTargetPosition() const { return static_cast<int>(targetY); }

  // State management
  bool isActive() const { return active; }
  void setActive(bool shouldBeActive);
  const juce::String &getEffectName() const { return effectName; }

  // Callback for state changes
  std::function<void(bool)> onStateChange;

private:
  // Custom constrainer that only allows vertical movement
  class VerticalOnlyConstrainer : public juce::ComponentBoundsConstrainer
  {
  public:
    explicit VerticalOnlyConstrainer(int fixedX, const juce::Rectangle<int> &validBounds)
        : originalX(fixedX), allowedBounds(validBounds) {}

    void checkBounds(juce::Rectangle<int> &bounds,
                     const juce::Rectangle<int> &previousBounds,
                     const juce::Rectangle<int> &limits,
                     bool, bool, bool, bool) override
    {
      // Keep X position fixed
      bounds.setX(originalX);
      bounds.setWidth(previousBounds.getWidth());

      // First, ensure we're within the allowed bounds
      if (!allowedBounds.isEmpty())
      {
        // Constrain Y position within allowed bounds, accounting for the button's height
        const int minY = allowedBounds.getY();
        const int maxY = allowedBounds.getBottom() - bounds.getHeight();
        bounds.setY(juce::jlimit(minY, maxY, bounds.getY()));
      }

      // Then apply any additional limits from the parent component
      if (!limits.isEmpty())
      {
        const int minY = limits.getY();
        const int maxY = limits.getBottom() - bounds.getHeight();
        bounds.setY(juce::jlimit(minY, maxY, bounds.getY()));
      }

      // Maintain the original width and height
      bounds.setWidth(previousBounds.getWidth());
      bounds.setHeight(previousBounds.getHeight());
    }

  private:
    int originalX;
    juce::Rectangle<int> allowedBounds;
  };

  void loadSvgIcon();
  int calculateInsertPosition(int newY) const;

  // Member variables
  juce::String effectName;
  bool active = false;
  bool dragging = false;
  float cornerSize = 6.0f;   // Rounded corner size
  int statusCircleSize = 12; // Size of the active status circle
  std::unique_ptr<juce::Drawable> icon;
  juce::File iconFile; // Store the SVG file reference

  juce::Rectangle<int> originalPos; // Store original position before drag
  juce::ComponentDragger dragger;
  std::unique_ptr<VerticalOnlyConstrainer> constrainer;

  // Animation state
  bool animating = false;
  float currentY = 0.0f;
  float targetY = 0.0f;
  float animationSpeed = 0.3f;                      // Adjust this value to control animation speed
  static constexpr float animationThreshold = 1.0f; // Stop animation when within this distance
  int dragOffsetY = 0;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectButton)
};
