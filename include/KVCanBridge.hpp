#ifndef KV_CAN_BRIDGE_H
#define KV_CAN_BRIDGE_H

#include <canlib.h>
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

    isobus::EventCallbackHandle hrxListenerHandle;
    isobus::EventCallbackHandle htxListenerHandle;

    void initializeVCan();
    void finalizeVCan();
    void initializeHCan();
    void finalizeHCan();

    std::string getErrorText(canStatus status, const std::string& context);

    void onHCanRx(const isobus::CANMessageFrame& canFrame);
    void onHCanTx(const isobus::CANMessageFrame& canFrame);

    void bridgeHtoVCan(const isobus::CANMessageFrame& canFrame);
    //void bridgeVtoHCan(const isobus::CANMessageFrame& canFrame);
};

#endif // KV_CAN_BRIDGE_H