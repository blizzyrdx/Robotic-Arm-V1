#include "Slider.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

Slider::Slider(
    std::string label,
    float x,
    float y,
    float width,
    int minimum,
    int maximum,
    int initialValue
)
    : label_(std::move(label)),
      track_{x, y, width, 8.0F},
      minimum_(minimum),
      maximum_(maximum),
      value_(initialValue),
      dragging_(false),
      hovered_(false)
{
    SetValue(initialValue);
}

bool Slider::Contains(float mouseX, float mouseY) const
{
    const SDL_FRect interactionArea{
        track_.x - 12.0F,
        track_.y - 16.0F,
        track_.w + 24.0F,
        40.0F
    };

    return mouseX >= interactionArea.x &&
           mouseX <= interactionArea.x + interactionArea.w &&
           mouseY >= interactionArea.y &&
           mouseY <= interactionArea.y + interactionArea.h;
}

bool Slider::SetValueFromMouse(float mouseX)
{
    float ratio =
        (mouseX - track_.x) / track_.w;

    ratio = std::clamp(ratio, 0.0F, 1.0F);

    const int newValue = static_cast<int>(
        std::round(
            static_cast<float>(minimum_) +
            ratio * static_cast<float>(maximum_ - minimum_)
        )
    );

    if (newValue == value_)
    {
        return false;
    }

    value_ = newValue;
    return true;
}

bool Slider::HandleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_MOUSE_MOTION)
    {
        hovered_ = Contains(event.motion.x, event.motion.y);

        if (dragging_)
        {
            return SetValueFromMouse(event.motion.x);
        }
    }

    if (
        event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event.button.button == SDL_BUTTON_LEFT
    )
    {
        if (Contains(event.button.x, event.button.y))
        {
            dragging_ = true;
            return SetValueFromMouse(event.button.x);
        }
    }

    if (
        event.type == SDL_EVENT_MOUSE_BUTTON_UP &&
        event.button.button == SDL_BUTTON_LEFT
    )
    {
        dragging_ = false;
    }

    return false;
}

SDL_FRect Slider::GetKnobRect() const
{
    const float range =
        static_cast<float>(maximum_ - minimum_);

    const float ratio =
        range == 0.0F
            ? 0.0F
            : static_cast<float>(value_ - minimum_) / range;

    const float knobCenterX =
        track_.x + ratio * track_.w;

    const float knobCenterY =
        track_.y + track_.h / 2.0F;

    return SDL_FRect{
        knobCenterX - 10.0F,
        knobCenterY - 14.0F,
        20.0F,
        28.0F
    };
}

void Slider::Draw(SDL_Renderer* renderer) const
{
    if (renderer == nullptr)
    {
        return;
    }

    // Servo name.
    SDL_SetRenderDrawColor(renderer, 225, 230, 240, 255);
    SDL_RenderDebugText(
        renderer,
        track_.x - 200.0F,
        track_.y,
        label_.c_str()
    );

    // Empty part of the track.
    SDL_SetRenderDrawColor(renderer, 57, 65, 82, 255);
    SDL_RenderFillRect(renderer, &track_);

    const float range =
        static_cast<float>(maximum_ - minimum_);

    const float ratio =
        range == 0.0F
            ? 0.0F
            : static_cast<float>(value_ - minimum_) / range;

    // Filled portion of the track.
    const SDL_FRect filledTrack{
        track_.x,
        track_.y,
        track_.w * ratio,
        track_.h
    };

    SDL_SetRenderDrawColor(renderer, 62, 132, 220, 255);
    SDL_RenderFillRect(renderer, &filledTrack);

    // Slider knob.
    const SDL_FRect knob = GetKnobRect();

    if (dragging_)
    {
        SDL_SetRenderDrawColor(renderer, 124, 190, 255, 255);
    }
    else if (hovered_)
    {
        SDL_SetRenderDrawColor(renderer, 100, 170, 245, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 82, 150, 230, 255);
    }

    SDL_RenderFillRect(renderer, &knob);

    SDL_SetRenderDrawColor(renderer, 185, 215, 255, 255);
    SDL_RenderRect(renderer, &knob);

    const std::string valueText =
        std::to_string(value_) + " deg";

    SDL_SetRenderDrawColor(renderer, 225, 230, 240, 255);
    SDL_RenderDebugText(
        renderer,
        track_.x + track_.w + 28.0F,
        track_.y,
        valueText.c_str()
    );
}

int Slider::GetValue() const
{
    return value_;
}

void Slider::SetValue(int value)
{
    value_ = std::clamp(
        value,
        minimum_,
        maximum_
    );
}