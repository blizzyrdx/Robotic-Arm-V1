#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "GUI.h"
#include "Serial.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace
{
    std::string RemoveLineEndings(std::string text)
    {
        while (
            !text.empty() &&
            (
                text.back() == '\n' ||
                text.back() == '\r'
            )
        )
        {
            text.pop_back();
        }

        return text;
    }

    void PrintAvailablePorts(
        const std::vector<std::string>& ports
    )
    {
        std::cout << "Available serial ports:\n";

        if (ports.empty())
        {
            std::cout << "  No serial ports found.\n";
            return;
        }

        for (const std::string& port : ports)
        {
            std::cout << "  " << port << '\n';
        }
    }
}

int main(int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr
            << "SDL initialization failed: "
            << SDL_GetError()
            << '\n';

        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (
        !SDL_CreateWindowAndRenderer(
            "Robot Arm Controller",
            1000,
            720,
            0,
            &window,
            &renderer
        )
    )
    {
        std::cerr
            << "Could not create the window: "
            << SDL_GetError()
            << '\n';

        SDL_Quit();
        return 1;
    }

    const bool vsyncEnabled =
        SDL_SetRenderVSync(renderer, 1);

    GUI gui;
    Serial serial;

    const std::vector<std::string> ports =
        Serial::ListPorts();

    PrintAvailablePorts(ports);

    std::string selectedPort;

    // You may provide an exact port as a command-line argument.
    if (argc >= 2)
    {
        selectedPort = argv[1];
    }
    else
    {
        selectedPort =
            Serial::FindLikelyArduinoPort(ports);
    }

    if (selectedPort.empty())
    {
        gui.SetStatus(
            "No Arduino port found. Check the USB cable."
        );

        std::cerr
            << "No likely Arduino serial port was found.\n";
    }
    else
    {
        std::cout
            << "Trying port: "
            << selectedPort
            << '\n';

        if (serial.Open(selectedPort, 9600))
        {
            gui.SetStatus(
                "Connected to " +
                selectedPort +
                ". Waiting for Arduino..."
            );

            std::cout
                << "Connected to "
                << selectedPort
                << '\n';

            /*
             * Classic boards such as the Uno can reset when the
             * serial port is opened. Wait for the bootloader and
             * sketch to start before transmitting.
             */
            std::this_thread::sleep_for(
                std::chrono::milliseconds(2000)
            );

            gui.SetStatus(
                "Connected to " +
                selectedPort
            );
        }
        else
        {
            gui.SetStatus(
                "Connection failed: " +
                serial.GetLastError()
            );

            std::cerr
                << serial.GetLastError()
                << '\n';
        }
    }

    bool running = true;

    while (running)
    {
        SDL_Event event{};

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            gui.HandleEvent(event);
        }

        if (gui.ConsumeSendRequest())
        {
            const std::string packet =
                gui.BuildPacket();

            if (!serial.IsOpen())
            {
                gui.SetStatus(
                    "Cannot send: Arduino is not connected."
                );
            }
            else if (serial.Send(packet))
            {
                const std::string displayedPacket =
                    RemoveLineEndings(packet);

                gui.SetStatus(
                    "Sent: " +
                    displayedPacket
                );

                std::cout
                    << "Sent: "
                    << displayedPacket
                    << '\n';
            }
            else
            {
                gui.SetStatus(
                    "Send failed: " +
                    serial.GetLastError()
                );

                std::cerr
                    << serial.GetLastError()
                    << '\n';
            }
        }

        if (serial.IsOpen())
        {
            std::string received =
                serial.ReadAvailable();

            if (!received.empty())
            {
                received =
                    RemoveLineEndings(received);

                gui.SetStatus(
                    "Arduino: " +
                    received
                );

                std::cout
                    << "Arduino: "
                    << received
                    << '\n';
            }
        }

        gui.Draw(renderer);
        SDL_RenderPresent(renderer);

        if (!vsyncEnabled)
        {
            SDL_Delay(16);
        }
    }

    serial.Close();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}