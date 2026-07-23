#pragma once

#include "Button.h"
#include "Slider.h"

#include <SDL3/SDL.h>

#include <array>
#include <string>
#include <vector>

class GUI
{
public:
    GUI();

    void HandleEvent(const SDL_Event& event);

    void Draw(SDL_Renderer* renderer) const;

    const std::array<int, 6>& GetAngles() const;

    // Returns true once for every requested transmission.
    bool ConsumeSendRequest();

    // Produces:
    // 90,90,90,90,90,20\n
    std::string BuildPacket() const;
    
    void SetStatus(const std::string& status);

private:
    void SynchronizeAngles();

    void RequestAutomaticSend();

    std::vector<Slider> sliders_;

    Button homeButton_;
    Button openButton_;
    Button closeButton_;
    Button liveButton_;
    Button sendButton_;

    std::array<int, 6> angles_;

    bool liveUpdate_;
    bool sendRequested_;

    std::string status_;
};