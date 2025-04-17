#include "KvaserVirtualCANPlugin.hpp"
#include <stdexcept>
#include <string>
#include <algorithm>

KvaserVirtualCANPlugin::KvaserVirtualCANPlugin(int channel)
    : channel(channel), handle(-1), isOpen(false)
{
    canInitializeLibrary();
}

KvaserVirtualCANPlugin::~KvaserVirtualCANPlugin()
{
    close();
}

bool KvaserVirtualCANPlugin::get_is_valid() const
{
    return isOpen;
}

void KvaserVirtualCANPlugin::open()
{
    handle = canOpenChannel(channel, canOPEN_ACCEPT_VIRTUAL);
    if (handle < 0)
    {
        throw std::runtime_error("Failed to open Kvaser Virtual CAN channel: " + std::to_string(channel));
    }

    canStatus status = canSetBusParams(handle, canBITRATE_250K, 0, 0, 0, 0, 0);
    if (status != canOK)
    {
        throw std::runtime_error("Failed to set bus parameters: " + std::to_string(status));
    }

    status = canBusOn(handle);
    if (status != canOK)
    {
        throw std::runtime_error("Failed to go bus on: " + std::to_string(status));
    }

    isOpen = true;
}

void KvaserVirtualCANPlugin::close()
{
    if (isOpen)
    {
        canBusOff(handle);
        canClose(handle);
        isOpen = false;
    }
}

bool KvaserVirtualCANPlugin::read_frame(isobus::CANMessageFrame &canFrame)
{
    long id;
    unsigned char data[8];
    unsigned int dlc;
    unsigned int flags;
    unsigned long timestamp;

    canStatus status = canRead(handle, &id, data, &dlc, &flags, &timestamp);
    if (status == canOK)
    {
        if(flags & canMSG_EXT)
        {
            canFrame.isExtendedFrame = true;
        }
        canFrame.identifier = id;
        canFrame.dataLength = dlc;
        std::copy(data, data + dlc, canFrame.data);
        return true;
    }
    return false;
}

bool KvaserVirtualCANPlugin::write_frame(const isobus::CANMessageFrame &canFrame)
{
    unsigned int flags;
    if(canFrame.isExtendedFrame)
    {
        flags = canMSG_EXT;
    }
    else
    {
        flags = 0;
    }
    canStatus status = canWrite(handle, canFrame.identifier, (void*)canFrame.data, canFrame.dataLength, flags);
    return status == canOK;
}