#include "KVCanBridge.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

KVCanBridge::KVCanBridge(int channel, long bitrate)
    : channel(channel), bitrate(bitrate), vHandle(-1), runningFlag(false)
{
    initializeVCan();
    initializeHCan();

    runningFlag = true;
    vCanReadThread = std::thread(&KVCanBridge::processVCanMessages, this);
}

KVCanBridge::~KVCanBridge()
{
    runningFlag = false;
    if (vCanReadThread.joinable())
    {
        vCanReadThread.join();
    }

    finalizeHCan();
    finalizeVCan();
}

void KVCanBridge::initializeVCan()
{
    canInitializeLibrary();

    vHandle = canOpenChannel(channel, canOPEN_ACCEPT_VIRTUAL);
    if (vHandle < 0)
    {
        throw std::runtime_error(getErrorText((canStatus)vHandle, "canOpenChannel failed"));
    }

    canStatus status = canSetBusParams(vHandle, bitrate, 0, 0, 0, 0, 0);
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canSetBusParams failed"));
    }

    status = canSetBusOutputControl(vHandle, canDRIVER_NORMAL);
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canSetBusOutputControl failed"));
    }

    status = canBusOn(vHandle);
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canBusOn failed"));
    }
}

void KVCanBridge::finalizeVCan()
{
    canStatus status = canBusOff(vHandle);
    if (status != canOK)
    {
        std::cerr << getErrorText(status, "canBusOff failed") << std::endl;
    }

    status = canClose(vHandle);
    if (status != canOK)
    {
        std::cerr << getErrorText(status, "canClose failed") << std::endl;
    }
}

void KVCanBridge::initializeHCan()
{
    hrxListenerHandle = isobus::CANHardwareInterface::get_can_frame_received_event_dispatcher().add_listener(
        [this](const isobus::CANMessageFrame& canFrame) { onHCanRx(canFrame); });

    htxListenerHandle = isobus::CANHardwareInterface::get_can_frame_transmitted_event_dispatcher().add_listener(
        [this](const isobus::CANMessageFrame& canFrame) { onHCanTx(canFrame); });
}

void KVCanBridge::finalizeHCan()
{
    isobus::CANHardwareInterface::get_can_frame_received_event_dispatcher().remove_listener(hrxListenerHandle);
    isobus::CANHardwareInterface::get_can_frame_transmitted_event_dispatcher().remove_listener(htxListenerHandle);
}

std::string KVCanBridge::getErrorText(canStatus status, const std::string& context)
{
    char msg[64];
    canGetErrorText(status, msg, sizeof(msg));
    return context + " (" + msg + ")";
}

void KVCanBridge::onHCanRx(const isobus::CANMessageFrame& canFrame)
{
    bridgeHtoVCan(canFrame);
}

void KVCanBridge::onHCanTx(const isobus::CANMessageFrame& canFrame)
{
    {
        std::lock_guard<std::mutex> lock(sentPacketMutex);
        if (sentPacketIds.find(canFrame.identifier) != sentPacketIds.end())
        {
            sentPacketIds.erase(canFrame.identifier);
            return;
        }
    }
    bridgeHtoVCan(canFrame);
}

void KVCanBridge::bridgeHtoVCan(const isobus::CANMessageFrame &canFrame)
{
    unsigned int flags = 0;

    if (canFrame.isExtendedFrame)
    {
        flags |= canMSG_EXT;
    }

    canStatus status = canWrite(vHandle, canFrame.identifier, (void *)canFrame.data, canFrame.dataLength, flags);
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canWrite failed"));
    }
}

void KVCanBridge::processVCanMessages()
{
    while (runningFlag)
    {
        long id;
        unsigned char data[8];
        unsigned int dlc;
        unsigned int flags;
        unsigned long timestamp;

        isobus::CANMessageFrame frame;

        canStatus status = canReadWait(vHandle, &id, data, &dlc, &flags, &timestamp, 1000); // 1000 ms timeout
        if (status == canOK)
        {
            if(flags & canMSG_EXT)
            {
                frame.isExtendedFrame = true;
            }
            else
            {
                frame.isExtendedFrame = false;
            }
            frame.identifier = id;
            frame.dataLength = dlc;
            frame.channel = 0;
            std::copy(data, data + dlc, frame.data);
            
            {
                std::lock_guard<std::mutex> lock(sentPacketMutex);
                sentPacketIds.insert(id);
            }
            isobus::CANHardwareInterface::transmit_can_frame(frame);
        }
        else if (status != canERR_NOMSG)
        {
            std::cerr << getErrorText(status, "canReadWait failed") << std::endl;
        }
    }
}
