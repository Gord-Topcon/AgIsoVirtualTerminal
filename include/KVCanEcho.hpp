#ifndef KV_CAN_ECHO_H
#define KV_CAN_ECHO_H

#include <canlib.h>
#include <string>
#include <atomic>
#include <thread>

class KVCanEcho
{
public:
    KVCanEcho(int channel, long bitrate);
    ~KVCanEcho();

    void sendMessage(int id, const std::string& message);
    void startTestMessages(int id, const std::string& message);
    void stopTestMessages();

private:
    int channel;
    long bitrate;
    canHandle handle;
    std::atomic<bool> runningFlag;
    std::thread testThread;

    void openChannel();
    void setBusParams();
    void setBusOutputControl();
    void goBusOn();
    void goBusOff();
    void closeChannel();
    void testMessageLoop(int id, const std::string& message);

    std::string getErrorText(canStatus status, const std::string& context);
};

#endif // KV_CAN_ECHO_H