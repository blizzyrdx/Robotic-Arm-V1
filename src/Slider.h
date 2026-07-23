#pragma once

#include <SDL3/SDL.h>

#include <string>

class Slider
{
public:
    Slider(
        std::string label,
        float x,
        float y,
        float width,
        int minimum,
        int maximum,
        int initialValue
    );

    // Returns true when the angle changes.
    bool HandleEvent(const SDL_Event& event);

    void Draw(SDL_Renderer* renderer) const;

    int GetValue() const;

    void SetValue(int value);

private:
    bool Contains(float mouseX, float mouseY) const;

    bool SetValueFromMouse(float mouseX);

    SDL_FRect GetKnobRect() const;

    std::string label_;
    SDL_FRect track_;

    int minimum_;
    int maximum_;
    int value_;

    bool dragging_;
    bool hovered_;
};