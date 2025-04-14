#include "KVCanBridge.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>

KVCanBridge::KVCanBridge(int channel, long bitrate)
    : channel(channel), bitrate(bitrate), vHandle(-1)
{
    initializeVCan();
    
    startTestMessages(0x123, "HELLO!");
}

KVCanBridge::~KVCanBridge()
{
    stopTestMessages();
    if (testThread.joinable())
    {
        testThread.join();
    }

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

    auto status = canSetBusParams(vHandle, bitrate, 0, 0, 0, 0, 0);
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canSetBusParams failed"));
    }

    auto status = canSetBusOutputControl(vHandle, canDRIVER_NORMAL);
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canSetBusOutputControl failed"));
    }

    auto status = canBusOn(vHandle);
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canBusOn failed"));
    }
}

void KVCanBridge::finalizeVCan()
{
    auto status = canBusOff(vHandle);
    if (status != canOK)
    {
        std::cerr << getErrorText(status, "canBusOff failed") << std::endl;
    }
    auto status = canClose(vHandle);
    if (status != canOK)
    {
        std::cerr << getErrorText(status, "canClose failed") << std::endl;
    }
}

void KVCanBridge::sendMessage(int id, const std::string& message)
{
    canStatus status = canWrite(vHandle, id, (void *)message.c_str(), message.size(), 0);
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canWrite failed"));
    }

    status = canWriteSync(vHandle, 500); // Wait up to 500 ms for the message to be sent
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canWriteSync failed"));
    }
}

std::string KVCanBridge::getErrorText(canStatus status, const std::string& context)
{
    char msg[64];
    canGetErrorText(status, msg, sizeof(msg));
    return context + " (" + msg + ")";
}

void KVCanBridge::startTestMessages(int id, const std::string& message)
{
    runningFlag = true;
    testThread = std::thread(&KVCanBridge::testMessageLoop, this, id, message);
}

void KVCanBridge::stopTestMessages()
{
    runningFlag = false;
    if (testThread.joinable())
    {
        testThread.join();
    }
}

void KVCanBridge::testMessageLoop(int id, const std::string& message)
{
    while (runningFlag)
    {
        try
        {
            sendMessage(id, message);
            std::cout << "Test message sent: ID=" << id << ", Message=" << message << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error sending test message: " << e.what() << std::endl;
        }

        // Wait for 5 seconds before sending the next message
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}
