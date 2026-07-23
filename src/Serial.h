#pragma once

#include <libserialport.h>

#include <string>
#include <vector>

class Serial
{
public:
    Serial();
    ~Serial();

    Serial(const Serial&) = delete;
    Serial& operator=(const Serial&) = delete;

    static std::vector<std::string> ListPorts();

    static std::string FindLikelyArduinoPort(
        const std::vector<std::string>& ports
    );

    bool Open(
        const std::string& portName,
        int baudRate = 9600
    );

    void Close();

    bool IsOpen() const;

    bool Send(const std::string& data);

    // Returns complete lines received from the Arduino.
    // Returns an empty string when no complete line is available.
    std::string ReadAvailable();

    const std::string& GetPortName() const;

    const std::string& GetLastError() const;

private:
    bool Configure(int baudRate);

    void SetError(
        const std::string& operation,
        int result
    );

    sp_port* port_;

    std::string portName_;
    std::string lastError_;
    std::string receiveBuffer_;
};