#ifndef MUDCORE_EVENT_BUS_HPP
#define MUDCORE_EVENT_BUS_HPP

#include <mudcore/event.hpp>

#include <mutex>
#include <queue>
#include <vector>

namespace genesis::mudcore {

/**
 * @brief Thread-safe inbound queue (io thread produces, main thread consumes in poll()).
 */
class EventBus {
public:
    EventBus() = default;

    void enqueueInboundEvent(Event event);

    std::vector<Event> drainInboundEvents();

    bool isEmpty() const;

private:
    mutable std::mutex inboundEventQueueMutex_;
    std::queue<Event> inboundEventQueue_;
};

} // namespace genesis::mudcore

#endif // MUDCORE_EVENT_BUS_HPP
