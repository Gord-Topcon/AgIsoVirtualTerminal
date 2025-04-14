#ifndef KV_CAN_ECHO_H
#define KV_CAN_ECHO_H

#include <canlib.h>
#include <string>
#include <atomic>
#include <thread>

class KVCanBridge
{
public:
    KVCanBridge(int channel, long bitrate);
    ~KVCanBridge();

    void sendMessage(int id, const std::string& message);
    void startTestMessages(int id, const std::string& message);
    void stopTestMessages();

private:
    int channel;
    long bitrate;
    canHandle vHandle;
    std::atomic<bool> runningFlag;
    std::thread testThread;

    void initializeVCan();
    void finalizeVCan();

    void testMessageLoop(int id, const std::string& message);

    std::string getErrorText(canStatus status, const std::string& context);
};

#endif // KV_CAN_ECHO_H