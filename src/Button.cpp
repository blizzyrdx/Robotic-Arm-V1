#include "Button.h"

#include <utility>

Button::Button(
    float x,
    float y,
    float width,
    float height,
    std::string label
)
    : bounds_{x, y, width, height},
      label_(std::move(label)),
      hovered_(false),
      pressed_(false)
{
}

bool Button::Contains(float mouseX, float mouseY) const
{
    return mouseX >= bounds_.x &&
           mouseX <= bounds_.x + bounds_.w &&
           mouseY >= bounds_.y &&
           mouseY <= bounds_.y + bounds_.h;
}

bool Button::HandleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_MOUSE_MOTION)
    {
        hovered_ = Contains(event.motion.x, event.motion.y);
    }

    if (
        event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event.button.button == SDL_BUTTON_LEFT
    )
    {
        if (Contains(event.button.x, event.button.y))
        {
            pressed_ = true;
        }
    }

    if (
        event.type == SDL_EVENT_MOUSE_BUTTON_UP &&
        event.button.button == SDL_BUTTON_LEFT
    )
    {
        const bool mouseInside =
            Contains(event.button.x, event.button.y);

        const bool clicked =
            pressed_ && mouseInside;

        pressed_ = false;
        hovered_ = mouseInside;

        return clicked;
    }

    return false;
}

void Button::Draw(SDL_Renderer* renderer) const
{
    if (renderer == nullptr)
    {
        return;
    }

    if (pressed_)
    {
        SDL_SetRenderDrawColor(renderer, 53, 104, 170, 255);
    }
    else if (hovered_)
    {
        SDL_SetRenderDrawColor(renderer, 64, 125, 203, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 45, 55, 72, 255);
    }

    SDL_RenderFillRect(renderer, &bounds_);

    SDL_SetRenderDrawColor(renderer, 115, 150, 200, 255);
    SDL_RenderRect(renderer, &bounds_);

    constexpr float characterWidth = 8.0F;
    constexpr float characterHeight = 8.0F;

    const float textWidth =
        static_cast<float>(label_.length()) * characterWidth;

    const float textX =
        bounds_.x + (bounds_.w - textWidth) / 2.0F;

    const float textY =
        bounds_.y + (bounds_.h - characterHeight) / 2.0F;

    SDL_SetRenderDrawColor(renderer, 240, 245, 255, 255);
    SDL_RenderDebugText(
        renderer,
        textX,
        textY,
        label_.c_str()
    );
}

void Button::SetLabel(const std::string& label)
{
    label_ = label;
}