#ifndef KV_CAN_BRIDGE_H
#define KV_CAN_BRIDGE_H

#include <canlib.h>
#include <string>
#include <atomic>
#include <thread>
#include <set>
#include <mutex>
#include "isobus/hardware_integration/can_hardware_interface.hpp"

class KVCanBridge
{
public:
    KVCanBridge(int channel, long bitrate);
    ~KVCanBridge();

private:
    int channel;
    long bitrate;
    canHandle vHandle;
    std::atomic<bool> runningFlag;
    std::thread vCanReadThread;
    std::set<long> sentPacketIds;
    std::mutex sentPacketMutex;

    isobus::EventCallbackHandle hrxListenerHandle;
    isobus::EventCallbackHandle htxListenerHandle;

    void initializeVCan();
    void finalizeVCan();
    void initializeHCan();
    void finalizeHCan();

    void processVCanMessages();

    std::string getErrorText(canStatus status, const std::string& context);

    void onHCanRx(const isobus::CANMessageFrame& canFrame);
    void onHCanTx(const isobus::CANMessageFrame& canFrame);
    void bridgeHtoVCan(const isobus::CANMessageFrame& canFrame);
};

#endif // KV_CAN_BRIDGE_H