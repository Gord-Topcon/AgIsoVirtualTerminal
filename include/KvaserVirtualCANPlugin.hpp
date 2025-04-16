#ifndef KVASER_VIRTUAL_CAN_PLUGIN_HPP
#define KVASER_VIRTUAL_CAN_PLUGIN_HPP

#include <canlib.h>
#include "isobus/hardware_integration/can_hardware_plugin.hpp"
#include "isobus/isobus/can_message_frame.hpp"

class KvaserVirtualCANPlugin : public isobus::CANHardwarePlugin
{
public:
    explicit KvaserVirtualCANPlugin(int channel);
    ~KvaserVirtualCANPlugin() override;

    bool get_is_valid() const override;
    void open() override;
    void close() override;
    bool read_frame(isobus::CANMessageFrame &canFrame) override;
    bool write_frame(const isobus::CANMessageFrame &canFrame) override;

private:
    int channel;
    canHandle handle;
    bool isOpen;
};

#endif // KVASER_VIRTUAL_CAN_PLUGIN_HPP