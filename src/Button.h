#pragma once

#include <SDL3/SDL.h>

#include <string>

class Button
{
public:
    Button(
        float x,
        float y,
        float width,
        float height,
        std::string label
    );

    // Returns true when the button is clicked.
    bool HandleEvent(const SDL_Event& event);

    void Draw(SDL_Renderer* renderer) const;

    void SetLabel(const std::string& label);

private:
    bool Contains(float mouseX, float mouseY) const;

    SDL_FRect bounds_;
    std::string label_;

    bool hovered_;
    bool pressed_;
};