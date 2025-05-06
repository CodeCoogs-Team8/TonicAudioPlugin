#include "EffectParametersContainer.h"

EffectParametersContainer::EffectParametersContainer()
{
    setOpaque(false);
}

EffectParametersContainer::~EffectParametersContainer()
{
}

void EffectParametersContainer::paint(juce::Graphics &g)
{
    // Container is transparent
}

void EffectParametersContainer::resized()
{
    updateComponentBounds();
}

void EffectParametersContainer::addEffectParameters(const juce::String &effectName, juce::AudioProcessor *processor)
{
    // Check if we already have this effect
    auto it = std::find_if(effectComponents.begin(), effectComponents.end(),
                           [&effectName](const auto &info)
                           {
                               return info.component->getEffectName() == effectName;
                           });

    if (it == effectComponents.end())
    {
        EffectComponentInfo info;
        info.component = std::make_unique<EffectParameterComponent>(effectName, processor);
        info.position = effectPositions.count(effectName) > 0 ? effectPositions[effectName] : static_cast<int>(effectComponents.size());

        addAndMakeVisible(info.component.get());
        effectComponents.push_back(std::move(info));
        effectPositions[effectName] = info.position;

        updatePositions();
        updateComponentBounds();
    }
}

void EffectParametersContainer::removeEffectParameters(const juce::String &effectName)
{
    // Store the position before removal
    int position = -1;
    auto it = std::find_if(effectComponents.begin(), effectComponents.end(),
                           [&effectName](const auto &info)
                           {
                               return info.component->getEffectName() == effectName;
                           });

    if (it != effectComponents.end())
    {
        position = it->position;
        effectComponents.erase(it);
        // Keep the position in effectPositions for potential restoration
        effectPositions[effectName] = position;

        updatePositions();
        updateComponentBounds();
    }
}

void EffectParametersContainer::reorderEffects(int oldPosition, int newPosition)
{
    if (oldPosition >= 0 && oldPosition < effectComponents.size() &&
        newPosition >= 0 && newPosition < effectComponents.size() &&
        oldPosition != newPosition)
    {
        // Store the component temporarily
        auto component = std::move(effectComponents[oldPosition]);

        // Remove from old position
        effectComponents.erase(effectComponents.begin() + oldPosition);

        // Insert at new position
        effectComponents.insert(effectComponents.begin() + newPosition, std::move(component));

        // Update positions
        updatePositions();

        // Update the layout
        updateComponentBounds();
    }
}

void EffectParametersContainer::updateComponentBounds()
{
    auto bounds = getLocalBounds();
    bounds.reduce(static_cast<int>(padding), static_cast<int>(padding));

    int y = bounds.getY();
    for (auto &info : effectComponents)
    {
        info.component->setBounds(bounds.getX(), y,
                                  bounds.getWidth(),
                                  static_cast<int>(componentHeight));
        y += static_cast<int>(componentHeight + componentSpacing);
    }
}

void EffectParametersContainer::updatePositions()
{
    // Update positions in both vectors and map
    for (size_t i = 0; i < effectComponents.size(); ++i)
    {
        effectComponents[i].position = static_cast<int>(i);
        effectPositions[effectComponents[i].component->getEffectName()] = static_cast<int>(i);
    }
}

int EffectParametersContainer::findComponentIndex(const juce::String &effectName) const
{
    for (size_t i = 0; i < effectComponents.size(); ++i)
    {
        if (effectComponents[i].component->getEffectName() == effectName)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void EffectParametersContainer::setEffectEnabled(const juce::String &effectName, bool shouldBeEnabled)
{
    auto index = findComponentIndex(effectName);
    if (index >= 0)
    {
        effectComponents[index].component->setEnabled(shouldBeEnabled);
    }
}

int EffectParametersContainer::getMinimumHeight() const
{
    if (effectComponents.empty())
        return 0;

    return static_cast<int>(padding * 2 +
                            effectComponents.size() * componentHeight +
                            (effectComponents.size() - 1) * componentSpacing);
}