#include "KVCanEcho.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>

KVCanEcho::KVCanEcho(int channel, long bitrate)
    : channel(channel), bitrate(bitrate), handle(-1)
{
    canInitializeLibrary();
    openChannel();
    setBusParams();
    setBusOutputControl();
    goBusOn();
    startTestMessages(0x123, "HELLO!");
}

KVCanEcho::~KVCanEcho()
{
    stopTestMessages();
    if (testThread.joinable())
    {
        testThread.join();
    }
    goBusOff();
    closeChannel();
}

void KVCanEcho::openChannel()
{
    handle = canOpenChannel(channel, canOPEN_ACCEPT_VIRTUAL);
    if (handle < 0)
    {
        throw std::runtime_error(getErrorText((canStatus)handle, "canOpenChannel failed"));
    }
}

void KVCanEcho::setBusParams()
{
    canStatus status = canSetBusParams(handle, bitrate, 0, 0, 0, 0, 0);
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canSetBusParams failed"));
    }
}

void KVCanEcho::setBusOutputControl()
{
    canStatus status = canSetBusOutputControl(handle, canDRIVER_NORMAL);
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canSetBusOutputControl failed"));
    }
}

void KVCanEcho::goBusOn()
{
    canStatus status = canBusOn(handle);
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canBusOn failed"));
    }
}

void KVCanEcho::goBusOff()
{
    canStatus status = canBusOff(handle);
    if (status != canOK)
    {
        std::cerr << getErrorText(status, "canBusOff failed") << std::endl;
    }
}

void KVCanEcho::closeChannel()
{
    canStatus status = canClose(handle);
    if (status != canOK)
    {
        std::cerr << getErrorText(status, "canClose failed") << std::endl;
    }
}

void KVCanEcho::sendMessage(int id, const std::string& message)
{
    canStatus status = canWrite(handle, id, (void *)message.c_str(), message.size(), 0);
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canWrite failed"));
    }

    status = canWriteSync(handle, 500); // Wait up to 500 ms for the message to be sent
    if (status != canOK)
    {
        throw std::runtime_error(getErrorText(status, "canWriteSync failed"));
    }
}

std::string KVCanEcho::getErrorText(canStatus status, const std::string& context)
{
    char msg[64];
    canGetErrorText(status, msg, sizeof(msg));
    return context + " (" + msg + ")";
}

void KVCanEcho::startTestMessages(int id, const std::string& message)
{
    runningFlag = true;
    testThread = std::thread(&KVCanEcho::testMessageLoop, this, id, message);
}

void KVCanEcho::stopTestMessages()
{
    runningFlag = false;
    if (testThread.joinable())
    {
        testThread.join();
    }
}

void KVCanEcho::testMessageLoop(int id, const std::string& message)
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
