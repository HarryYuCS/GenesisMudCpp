#ifndef MUDCORE_EVENT_BUS_HPP
#define MUDCORE_EVENT_BUS_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <mudcore/event.hpp>

namespace genesis::mudcore {

class EventBus {
public:
    EventBus();
    ~EventBus();

    void enqueueServerEvent(const ServerEvent& event);
    void enqueueClientEvent(const ClientEvent& event);
    ServerEvent dequeueServerEvent();
    ClientEvent dequeueClientEvent();

private:
    std::queue<ServerEvent> serverEventQueue;
    std::queue<ClientEvent> clientEventQueue;

    std::mutex serverEventQueueMutex;
    std::mutext clientEventQueueMutex;
    std::condition_variable serverEventQueueCondition;
    std::condition_variable clientEventQueueCondition;
};

} // namespace genesis::mudcore

#endif // MUDCORE_EVENT_BUS_HPP