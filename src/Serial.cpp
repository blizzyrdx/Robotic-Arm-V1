#include "Serial.h"

#include <cstddef>
#include <string>
#include <vector>

Serial::Serial()
    : port_(nullptr)
{
}

Serial::~Serial()
{
    Close();
}

std::vector<std::string> Serial::ListPorts()
{
    std::vector<std::string> names;

    sp_port** portList = nullptr;

    const int result = sp_list_ports(&portList);

    if (result != SP_OK || portList == nullptr)
    {
        return names;
    }

    for (int index = 0; portList[index] != nullptr; ++index)
    {
        const char* name =
            sp_get_port_name(portList[index]);

        if (name != nullptr)
        {
            names.emplace_back(name);
        }
    }

    sp_free_port_list(portList);

    return names;
}

std::string Serial::FindLikelyArduinoPort(
    const std::vector<std::string>& ports
)
{
    // Prefer macOS outgoing serial-device names.
    const std::vector<std::string> preferredNames{
        "/dev/cu.usbmodem",
        "/dev/cu.usbserial",
        "/dev/cu.wchusbserial",
        "/dev/cu.SLAB_USBtoUART",
        "/dev/tty.usbmodem",
        "/dev/tty.usbserial",
        "/dev/tty.wchusbserial",
        "/dev/tty.SLAB_USBtoUART"
    };

    for (const std::string& preferred : preferredNames)
    {
        for (const std::string& port : ports)
        {
            if (port.find(preferred) != std::string::npos)
            {
                return port;
            }
        }
    }

    // Fallback for another USB-to-serial device name.
    for (const std::string& port : ports)
    {
        const bool looksLikeUsb =
            port.find("usb") != std::string::npos ||
            port.find("USB") != std::string::npos;

        const bool looksLikeBluetooth =
            port.find("Bluetooth") != std::string::npos;

        if (looksLikeUsb && !looksLikeBluetooth)
        {
            return port;
        }
    }

    return "";
}

void Serial::SetError(
    const std::string& operation,
    int result
)
{
    std::string reason;

    switch (result)
    {
        case SP_ERR_ARG:
            reason = "invalid argument";
            break;

        case SP_ERR_FAIL:
            reason = "operating-system or device failure";
            break;

        case SP_ERR_MEM:
            reason = "memory allocation failure";
            break;

        case SP_ERR_SUPP:
            reason = "operation not supported";
            break;

        default:
            reason = "unknown error";
            break;
    }

    lastError_ =
        operation +
        " failed: " +
        reason +
        " (" +
        std::to_string(result) +
        ")";
}

bool Serial::Configure(int baudRate)
{
    int result = sp_set_baudrate(port_, baudRate);

    if (result != SP_OK)
    {
        SetError("Setting baud rate", result);
        return false;
    }

    result = sp_set_bits(port_, 8);

    if (result != SP_OK)
    {
        SetError("Setting data bits", result);
        return false;
    }

    result = sp_set_parity(
        port_,
        SP_PARITY_NONE
    );

    if (result != SP_OK)
    {
        SetError("Setting parity", result);
        return false;
    }

    result = sp_set_stopbits(port_, 1);

    if (result != SP_OK)
    {
        SetError("Setting stop bits", result);
        return false;
    }

    result = sp_set_flowcontrol(
        port_,
        SP_FLOWCONTROL_NONE
    );

    if (result != SP_OK)
    {
        SetError("Setting flow control", result);
        return false;
    }

    return true;
}

bool Serial::Open(
    const std::string& portName,
    int baudRate
)
{
    Close();

    lastError_.clear();
    receiveBuffer_.clear();

    int result = sp_get_port_by_name(
        portName.c_str(),
        &port_
    );

    if (result != SP_OK || port_ == nullptr)
    {
        SetError("Finding serial port", result);
        port_ = nullptr;
        return false;
    }

    result = sp_open(
        port_,
        SP_MODE_READ_WRITE
    );

    if (result != SP_OK)
    {
        SetError("Opening serial port", result);

        sp_free_port(port_);
        port_ = nullptr;

        return false;
    }

    if (!Configure(baudRate))
    {
        Close();
        return false;
    }

    result = sp_flush(
        port_,
        SP_BUF_BOTH
    );

    if (result != SP_OK)
    {
        SetError("Clearing serial buffers", result);
        Close();
        return false;
    }

    portName_ = portName;

    return true;
}

void Serial::Close()
{
    if (port_ != nullptr)
    {
        sp_close(port_);
        sp_free_port(port_);

        port_ = nullptr;
    }

    portName_.clear();
    receiveBuffer_.clear();
}

bool Serial::IsOpen() const
{
    return port_ != nullptr;
}

bool Serial::Send(const std::string& data)
{
    if (!IsOpen())
    {
        lastError_ = "Serial port is not open.";
        return false;
    }

    if (data.empty())
    {
        lastError_ = "Cannot send an empty command.";
        return false;
    }

    const int bytesWritten = sp_blocking_write(
        port_,
        data.data(),
        data.size(),
        1000
    );

    if (bytesWritten < 0)
    {
        SetError("Writing serial data", bytesWritten);
        return false;
    }

    if (
        static_cast<std::size_t>(bytesWritten) !=
        data.size()
    )
    {
        lastError_ =
            "Only " +
            std::to_string(bytesWritten) +
            " of " +
            std::to_string(data.size()) +
            " bytes were written.";

        return false;
    }

    const int drainResult = sp_drain(port_);

    if (drainResult != SP_OK)
    {
        SetError(
            "Waiting for serial transmission",
            drainResult
        );

        return false;
    }

    lastError_.clear();

    return true;
}

std::string Serial::ReadAvailable()
{
    if (!IsOpen())
    {
        return "";
    }

    char temporaryBuffer[256];

    while (true)
    {
        const int bytesRead = sp_nonblocking_read(
            port_,
            temporaryBuffer,
            sizeof(temporaryBuffer)
        );

        if (bytesRead < 0)
        {
            SetError("Reading serial data", bytesRead);
            return "";
        }

        if (bytesRead == 0)
        {
            break;
        }

        receiveBuffer_.append(
            temporaryBuffer,
            static_cast<std::size_t>(bytesRead)
        );
    }

    const std::size_t lastNewline =
        receiveBuffer_.find_last_of('\n');

    if (lastNewline == std::string::npos)
    {
        return "";
    }

    const std::string completeLines =
        receiveBuffer_.substr(
            0,
            lastNewline + 1
        );

    receiveBuffer_.erase(
        0,
        lastNewline + 1
    );

    return completeLines;
}

const std::string& Serial::GetPortName() const
{
    return portName_;
}

const std::string& Serial::GetLastError() const
{
    return lastError_;
}