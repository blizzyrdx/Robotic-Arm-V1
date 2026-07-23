#include "GUI.h"

#include <sstream>
#include <string>

GUI::GUI()
    : sliders_{
          Slider(
              "Root",
              270.0F,
              150.0F,
              540.0F,
              0,
              180,
              90
          ),
          Slider(
              "Arm A1 + mirrored A2",
              270.0F,
              210.0F,
              540.0F,
              0,
              180,
              90
          ),
          Slider(
              "Arm B",
              270.0F,
              270.0F,
              540.0F,
              0,
              180,
              90
          ),
          Slider(
              "Wrist A",
              270.0F,
              330.0F,
              540.0F,
              0,
              180,
              90
          ),
          Slider(
              "Wrist B",
              270.0F,
              390.0F,
              540.0F,
              0,
              180,
              90
          ),
          Slider(
              "Gripper",
              270.0F,
              450.0F,
              540.0F,
              0,
              180,
              20
          )
      },
      homeButton_(
          50.0F,
          535.0F,
          140.0F,
          48.0F,
          "Home"
      ),
      openButton_(
          210.0F,
          535.0F,
          160.0F,
          48.0F,
          "Open gripper"
      ),
      closeButton_(
          390.0F,
          535.0F,
          160.0F,
          48.0F,
          "Close gripper"
      ),
      liveButton_(
          570.0F,
          535.0F,
          170.0F,
          48.0F,
          "Live update: OFF"
      ),
      sendButton_(
          760.0F,
          535.0F,
          190.0F,
          48.0F,
          "Send angles"
      ),
      angles_{},
      liveUpdate_(false),
      sendRequested_(false),
      status_("Searching for the Arduino serial port...")
{
    SynchronizeAngles();
}

void GUI::SynchronizeAngles()
{
    for (std::size_t index = 0; index < angles_.size(); ++index)
    {
        angles_[index] = sliders_[index].GetValue();
    }
}

void GUI::RequestAutomaticSend()
{
    if (liveUpdate_)
    {
        sendRequested_ = true;
        status_ = "Live update requested a transmission.";
    }
}

void GUI::HandleEvent(const SDL_Event& event)
{
    bool sliderChanged = false;

    for (Slider& slider : sliders_)
    {
        if (slider.HandleEvent(event))
        {
            sliderChanged = true;
        }
    }

    if (sliderChanged)
    {
        SynchronizeAngles();

        if (!liveUpdate_)
        {
            status_ =
                "Angles changed. Press Send angles to transmit.";
        }

        RequestAutomaticSend();
    }

    if (homeButton_.HandleEvent(event))
    {
        const std::array<int, 6> homeAngles{
            90,
            90,
            90,
            90,
            90,
            20
        };

        for (std::size_t index = 0;
             index < homeAngles.size();
             ++index)
        {
            sliders_[index].SetValue(homeAngles[index]);
        }

        SynchronizeAngles();
        status_ = "Robot returned to the home pose.";
        RequestAutomaticSend();
    }

    if (openButton_.HandleEvent(event))
    {
        // Adjust this angle later if your gripper opens
        // in the opposite direction.
        sliders_[5].SetValue(90);

        SynchronizeAngles();
        status_ = "Gripper set to the open position.";
        RequestAutomaticSend();
    }

    if (closeButton_.HandleEvent(event))
    {
        // Adjust this angle to match your physical gripper.
        sliders_[5].SetValue(10);

        SynchronizeAngles();
        status_ = "Gripper set to the closed position.";
        RequestAutomaticSend();
    }

    if (liveButton_.HandleEvent(event))
    {
        liveUpdate_ = !liveUpdate_;

        if (liveUpdate_)
        {
            liveButton_.SetLabel("Live update: ON");
            status_ =
                "Live update enabled. Moving a slider requests a send.";
        }
        else
        {
            liveButton_.SetLabel("Live update: OFF");
            status_ =
                "Live update disabled. Use the Send angles button.";
        }
    }

    if (sendButton_.HandleEvent(event))
    {
        SynchronizeAngles();
        sendRequested_ = true;
        status_ =
            "Send requested. Packet printed in the terminal.";
    }
}

void GUI::Draw(SDL_Renderer* renderer) const
{
    if (renderer == nullptr)
    {
        return;
    }

    // Main background.
    SDL_SetRenderDrawColor(renderer, 22, 25, 32, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 235, 240, 250, 255);
    SDL_RenderDebugText(
        renderer,
        40.0F,
        28.0F,
        "ROBOT ARM CONTROLLER"
    );

    SDL_SetRenderDrawColor(renderer, 145, 155, 175, 255);
    SDL_RenderDebugText(
        renderer,
        40.0F,
        52.0F,
        "Drag a slider to select and move a robot joint."
    );

    // Slider panel.
    const SDL_FRect controlPanel{
        35.0F,
        100.0F,
        930.0F,
        405.0F
    };

    SDL_SetRenderDrawColor(renderer, 30, 35, 45, 255);
    SDL_RenderFillRect(renderer, &controlPanel);

    SDL_SetRenderDrawColor(renderer, 64, 74, 94, 255);
    SDL_RenderRect(renderer, &controlPanel);

    for (const Slider& slider : sliders_)
    {
        slider.Draw(renderer);
    }

    SDL_SetRenderDrawColor(renderer, 120, 130, 150, 255);
    SDL_RenderDebugText(
        renderer,
        60.0F,
        485.0F,
        "Arm A2 is automatically mirrored by the Arduino firmware."
    );

    homeButton_.Draw(renderer);
    openButton_.Draw(renderer);
    closeButton_.Draw(renderer);
    liveButton_.Draw(renderer);
    sendButton_.Draw(renderer);

    // Status panel.
    const SDL_FRect statusPanel{
        35.0F,
        610.0F,
        930.0F,
        75.0F
    };

    SDL_SetRenderDrawColor(renderer, 28, 32, 41, 255);
    SDL_RenderFillRect(renderer, &statusPanel);

    SDL_SetRenderDrawColor(renderer, 64, 74, 94, 255);
    SDL_RenderRect(renderer, &statusPanel);

    SDL_SetRenderDrawColor(renderer, 125, 205, 145, 255);
    SDL_RenderDebugText(
        renderer,
        52.0F,
        625.0F,
        status_.c_str()
    );

    std::string packetText =
        "Packet: " + BuildPacket();

    // Remove the newline from the packet preview.
    if (!packetText.empty() && packetText.back() == '\n')
    {
        packetText.pop_back();
    }

    SDL_SetRenderDrawColor(renderer, 175, 185, 205, 255);
    SDL_RenderDebugText(
        renderer,
        52.0F,
        652.0F,
        packetText.c_str()
    );
}

const std::array<int, 6>& GUI::GetAngles() const
{
    return angles_;
}

bool GUI::ConsumeSendRequest()
{
    if (!sendRequested_)
    {
        return false;
    }

    sendRequested_ = false;
    return true;
}

std::string GUI::BuildPacket() const
{
    std::ostringstream packet;

    for (std::size_t index = 0;
         index < angles_.size();
         ++index)
    {
        packet << angles_[index];

        if (index + 1 < angles_.size())
        {
            packet << ',';
        }
    }

    packet << '\n';

    return packet.str();
}
void GUI::SetStatus(const std::string& status)
{
    status_ = status;
}