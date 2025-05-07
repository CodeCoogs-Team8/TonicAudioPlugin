/*
  ==============================================================================

    EffectButton.cpp
    Created: 15 Apr 2025 10:16:29pm
    Author:  Evan Fraustro

  ==============================================================================
*/

#include "EffectButton.h"
#include "ToolbarComponent.h"

EffectButton::EffectButton(const juce::String &name)
    : juce::Component(),
      effectName(name)
{
  setOpaque(false);             // Make sure the component is not opaque
  setPaintingIsUnclipped(true); // This can help with OpenGL rendering

  // Load the SVG icon
  loadSvgIcon();
}

EffectButton::~EffectButton()
{
  // Stop any running animation
  stopTimer();

  // Clear unique_ptrs
  icon.reset();
  constrainer.reset();
}

void EffectButton::setActive(bool shouldBeActive)
{
  if (active != shouldBeActive)
  {
    active = shouldBeActive;
    if (onStateChange)
      onStateChange(active);
    repaint();
  }
}

void EffectButton::loadSvgIcon()
{
  // Start from the executable location
  juce::File executableFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
  DBG("Executable path: " << executableFile.getFullPathName());

  // Navigate up to find project root, but limit the search depth
  juce::File currentDir = executableFile;
  int maxDepth = 5; // Prevent searching too far up
  int depth = 0;

  while (currentDir.exists() &&
         !currentDir.getChildFile("Assets").exists() &&
         currentDir.getParentDirectory() != currentDir &&
         depth < maxDepth)
  {
    currentDir = currentDir.getParentDirectory();
    // Stop if we find the git directory (likely project root)
    if (currentDir.getChildFile(".git").exists() ||
        currentDir.getFileName() == "Delay") // Your project name
    {
      break;
    }
    depth++;
  }

  // If we hit max depth or root without finding the project, try the executable's parent
  if (depth >= maxDepth || currentDir.getFullPathName() == "/")
  {
    currentDir = executableFile.getParentDirectory()
                     .getParentDirectory()  // MacOS
                     .getParentDirectory()  // Contents
                     .getParentDirectory()  // Delay.app
                     .getParentDirectory()  // Debug
                     .getParentDirectory()  // build
                     .getParentDirectory()  // MacOSX
                     .getParentDirectory(); // Builds
  }

  DBG("Looking for Assets folder in: " << currentDir.getFullPathName());

  // Determine which SVG to load based on effect name
  juce::String svgFileName;
  if (effectName == "Delay")
    svgFileName = "delay.svg";
  else if (effectName == "Distortion")
    svgFileName = "distortion.svg";
  else if (effectName == "Reverb")
    svgFileName = "reverb.svg";
  else if (effectName == "Chorus")
    svgFileName = "chorus.svg";
  else if (effectName == "EQ")
    svgFileName = "eq.svg";
  else
    return;

  // Try to load the SVG
  iconFile = currentDir.getChildFile("Assets/Icons/" + svgFileName);
  DBG("Attempting to load SVG from: " << iconFile.getFullPathName());

  if (iconFile.existsAsFile())
  {
    icon = juce::Drawable::createFromSVGFile(iconFile);
    if (icon != nullptr)
    {
      DBG("Successfully loaded SVG from: " << iconFile.getFullPathName());
      return;
    }
    DBG("Failed to create drawable from SVG file");
  }
  else
  {
    DBG("SVG file not found at: " << iconFile.getFullPathName());
    DBG("Current directory was: " << currentDir.getFullPathName());
  }
}

void EffectButton::paint(juce::Graphics &g)
{
  // Don't clear the background - let it be transparent
  auto bounds = getLocalBounds().toFloat().reduced(1.0f);
  const float iconSize = bounds.getHeight() * 0.5f;
  const float padding = bounds.getHeight() * 0.15f;
  const float statusCircleSize = bounds.getHeight() * 0.2f;

  // Create main button path
  juce::Path buttonPath;
  buttonPath.addRoundedRectangle(bounds, cornerSize);

  // Draw button shadow with semi-transparency
  g.setColour(juce::Colour(30, 30, 30).withAlpha(0.9f));
  g.fillPath(buttonPath);

  // Draw main background with flat color
  g.setColour(dragging ? juce::Colour(55, 55, 55).withAlpha(0.95f) : juce::Colour(45, 45, 45).withAlpha(0.95f));
  g.fillPath(buttonPath);

  // Draw outer edge highlight (purple when active or dragging, gray when not)
  if (active || dragging)
    g.setColour(juce::Colour(148, 101, 211).withAlpha(0.95f));
  else
    g.setColour(juce::Colour(70, 70, 70).withAlpha(0.8f));
  g.drawRoundedRectangle(bounds, cornerSize, 2.0f);

  // Set up clipping for internal elements
  g.saveState();
  g.reduceClipRegion(buttonPath);

  // Draw status circle in top left
  auto circleArea = juce::Rectangle<float>(
      bounds.getX() + padding * 0.75f,
      bounds.getY() + padding * 0.75f,
      statusCircleSize,
      statusCircleSize);

  g.setColour(active ? juce::Colour(148, 101, 211).withAlpha(0.95f) : juce::Colour(70, 70, 70).withAlpha(0.8f));
  g.fillEllipse(circleArea);

  // Add subtle highlight to circle
  g.setColour(juce::Colour(255, 255, 255).withAlpha(0.15f));
  g.drawEllipse(circleArea, 1.0f);

  // Draw effect name on the left side with more space
  g.setColour(juce::Colours::white.withAlpha(0.95f));
  g.setFont(bounds.getHeight() * 0.275f);

  // Adjust text bounds to start closer to the left edge
  auto textBounds = bounds.reduced(padding * 1.5f, 0.0f); // Reduced left padding
  textBounds.setWidth(textBounds.getWidth() - iconSize - padding * 2);
  g.drawText(effectName, textBounds, juce::Justification::centredLeft, true);

  // Draw SVG icon if available
  if (icon != nullptr)
  {
    auto iconBounds = juce::Rectangle<float>(
        bounds.getRight() - iconSize - padding,
        bounds.getCentreY() - iconSize / 2,
        iconSize,
        iconSize);

    icon->drawWithin(g, iconBounds,
                     juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize,
                     0.9f); // Slightly transparent
  }
  else
  {
    // Softer placeholder if SVG failed to load
    auto iconBounds = juce::Rectangle<float>(
        bounds.getRight() - iconSize - padding,
        bounds.getCentreY() - iconSize / 2,
        iconSize,
        iconSize);

    // Draw a more subtle placeholder
    g.setColour(juce::Colour(70, 70, 70).withAlpha(0.6f));
    const float thickness = 1.5f;

    // Draw rounded rectangle frame
    g.drawRoundedRectangle(iconBounds, 4.0f, thickness);

    // Draw subtle cross
    g.setColour(juce::Colour(60, 60, 60).withAlpha(0.6f));
    const float inset = iconBounds.getWidth() * 0.2f;
    g.drawLine(iconBounds.getX() + inset, iconBounds.getY() + inset,
               iconBounds.getRight() - inset, iconBounds.getBottom() - inset, thickness);
    g.drawLine(iconBounds.getX() + inset, iconBounds.getBottom() - inset,
               iconBounds.getRight() - inset, iconBounds.getY() + inset, thickness);
  }

  // Restore the graphics state
  g.restoreState();
}

void EffectButton::resized()
{
  // Update constrainer with current bounds
  if (auto *toolbar = dynamic_cast<ToolbarComponent *>(getParentComponent()))
  {
    const int spacing = static_cast<int>(getHeight() * 0.2f);
    const int totalButtons = toolbar->getNumButtons();
    const int validAreaHeight = (getHeight() + spacing) * totalButtons - spacing;

    // Create bounds that start exactly at the top of the workspace
    juce::Rectangle<int> validBounds(
        toolbar->getLocalBounds().withTrimmedTop(0).withTrimmedBottom(
            toolbar->getHeight() - (toolbar->buttonMargin + validAreaHeight)));

    // Update constrainer with calculated bounds
    if (constrainer == nullptr)
    {
      constrainer = std::make_unique<VerticalOnlyConstrainer>(getX(), validBounds);
    }
    else
    {
      constrainer.reset(new VerticalOnlyConstrainer(getX(), validBounds));
    }
  }
}

void EffectButton::mouseDown(const juce::MouseEvent &event)
{
  // Store initial state for all mouse interactions
  originalPos = getBounds();
  dragOffsetY = event.getEventRelativeTo(getParentComponent()).y - getY();

  // Always bring to front when interacting
  toFront(true);

  if (event.mods.isPopupMenu()) // Right click
  {
    // Handle right-click menu if needed in the future
  }
  else // Left click
  {
    // Don't set dragging true immediately - wait for mouseDrag
    // This allows for click detection in mouseUp

    // Calculate valid drag bounds based on button layout
    if (auto *toolbar = dynamic_cast<ToolbarComponent *>(getParentComponent()))
    {
      const int height = getHeight();
      const int spacing = static_cast<int>(height * 0.2f);
      const int totalButtons = toolbar->getNumButtons();
      const int validAreaHeight = (height + spacing) * totalButtons - spacing;

      // Create bounds that start exactly at the top of the workspace
      juce::Rectangle<int> validBounds(
          toolbar->getLocalBounds().withTrimmedTop(0).withTrimmedBottom(
              toolbar->getHeight() - (toolbar->buttonMargin + validAreaHeight)));

      // Update constrainer with current valid bounds
      if (constrainer == nullptr)
      {
        constrainer = std::make_unique<VerticalOnlyConstrainer>(getX(), validBounds);
      }
      else
      {
        constrainer.reset(new VerticalOnlyConstrainer(getX(), validBounds));
      }
    }
  }
}

void EffectButton::mouseDrag(const juce::MouseEvent &event)
{
  // Only start dragging if we've moved a minimum distance
  if (!dragging && event.getDistanceFromDragStart() > 5)
  {
    dragging = true;
    dragger.startDraggingComponent(this, event);
  }

  if (dragging && constrainer != nullptr)
  {
    auto *toolbar = dynamic_cast<ToolbarComponent *>(getParentComponent());
    if (!toolbar)
      return;

    // Calculate new position
    int newY = event.getEventRelativeTo(getParentComponent()).y - dragOffsetY;

    // Create proposed bounds
    auto proposedBounds = getBounds();
    proposedBounds.setY(newY);

    // Calculate valid drag bounds based on current button layout
    const int height = getHeight();
    const int spacing = static_cast<int>(height * 0.2f);
    const int totalButtons = toolbar->getNumButtons();
    const int validAreaHeight = (height + spacing) * totalButtons - spacing;

    // Create bounds that start exactly at the top of the workspace
    juce::Rectangle<int> validBounds(
        toolbar->getLocalBounds().withTrimmedTop(0).withTrimmedBottom(
            toolbar->getHeight() - (toolbar->buttonMargin + validAreaHeight)));

    // Apply constraints using the valid bounds
    constrainer->checkBounds(proposedBounds, getBounds(), validBounds,
                             false, false, false, false);

    // Store last position before calculating new one
    int lastPosition = -1;
    for (size_t i = 0; i < toolbar->getNumButtons(); ++i)
    {
      if (toolbar->getButtonOrder()[i] == this)
      {
        lastPosition = static_cast<int>(i);
        break;
      }
    }

    // Calculate insert position based on constrained Y position
    int newPosition = calculateInsertPosition(proposedBounds.getY());

    // Only reorder if position actually changed and we've moved significantly
    static int dragThreshold = 10; // Pixels to move before initiating reorder
    static int lastDragY = proposedBounds.getY();

    if (newPosition != lastPosition &&
        std::abs(proposedBounds.getY() - lastDragY) > dragThreshold)
    {
      lastDragY = proposedBounds.getY();
      toolbar->reorderButtons(this, newPosition);
    }

    // Update position using constrained bounds
    setBounds(proposedBounds);
  }
}

int EffectButton::calculateInsertPosition(int newY) const
{
  if (auto *toolbar = dynamic_cast<ToolbarComponent *>(getParentComponent()))
  {
    const int buttonHeight = getHeight();
    const int spacing = static_cast<int>(buttonHeight * 0.2f);
    const int margin = toolbar->buttonMargin;
    const float overlapThreshold = 0.6f; // Increased from 0.35f to require more overlap

    int currentPosition = -1;
    for (size_t i = 0; i < toolbar->getNumButtons(); ++i)
    {
      if (toolbar->getButtonOrder()[i] == this)
      {
        currentPosition = static_cast<int>(i);
        break;
      }
    }

    // Calculate the ideal position based on Y coordinate
    int idealPosition = 0;
    int y = margin;

    // Calculate position and exact overlap
    while (y < newY && idealPosition < toolbar->getNumButtons())
    {
      y += buttonHeight + spacing;
      idealPosition++;
    }

    // Calculate exact overlap percentage
    int slotStartY = margin + (idealPosition - 1) * (buttonHeight + spacing);
    float overlapAmount = static_cast<float>(newY - slotStartY) / (buttonHeight + spacing);

    // Add hysteresis to prevent oscillation
    static const float hysteresis = 0.1f;

    // Determine if we should change position based on drag direction and overlap
    bool shouldChangePosition = false;

    if (idealPosition > currentPosition) // Dragging downward
    {
      shouldChangePosition = overlapAmount >= (overlapThreshold + hysteresis);
    }
    else if (idealPosition < currentPosition) // Dragging upward
    {
      shouldChangePosition = overlapAmount <= (1.0f - overlapThreshold - hysteresis);
    }

    // Special case: allow moving to the top if dragged very close to the top
    if (newY < margin + (buttonHeight * 0.2f))
      return 0;

    return shouldChangePosition ? idealPosition : currentPosition;
  }

  return 0;
}

void EffectButton::mouseUp(const juce::MouseEvent &)
{
  if (!dragging)
  {
    // If we didn't drag, this was a click - toggle active state
    setActive(!active);
  }
  else
  {
    // If we were dragging, ensure the button is properly positioned in the grid
    if (auto *toolbar = dynamic_cast<ToolbarComponent *>(getParentComponent()))
    {
      toolbar->updateButtonPositions();
    }
  }

  // Always reset dragging state
  dragging = false;
  repaint();
}

void EffectButton::timerCallback()
{
  if (animating)
  {
    // Calculate new position using smoother easing
    float delta = targetY - currentY;
    float easingFactor = 0.25f; // Smoother easing (was 0.3f)
    currentY += delta * easingFactor;

    // Check if we're close enough to stop
    if (std::abs(delta) < 0.5f) // Smaller threshold for more precise positioning
    {
      currentY = targetY;
      animating = false;
      stopTimer();
    }

    // Update position
    setTopLeftPosition(getX(), static_cast<int>(currentY));
  }
}

void EffectButton::startAnimationToPosition(int newTargetY)
{
  targetY = static_cast<float>(newTargetY);
  currentY = static_cast<float>(getY());
  animating = true;
  startTimerHz(120); // Higher refresh rate for smoother animation (was 60)
}

void EffectButton::setTargetPosition(int newTargetY) // Renamed parameter to avoid shadowing
{
  targetY = static_cast<float>(newTargetY);
  if (!animating)
  {
    currentY = static_cast<float>(getY());
    animating = true;
    startTimerHz(60);
  };
}
