/**
 * @file event_bus.hpp
 * @brief Thread-safe inbound event queue between the io thread and the main thread.
 */

#ifndef MUDCORE_EVENT_BUS_HPP
#define MUDCORE_EVENT_BUS_HPP

#include <mudcore/event.hpp>

#include <mutex>
#include <queue>
#include <vector>

namespace genesis::mudcore {

/**
 * @brief Mutex-protected queue of inbound Events.
 *
 * Producers: Session I/O callbacks on the io thread.
 * Consumer: Session::poll() on the main thread.
 */
class EventBus {
public:
    EventBus() = default;

    /**
     * @brief Enqueue an event from the io thread.
     * @param event Event to store (moved into the queue).
     */
    void enqueueInboundEvent(Event event);

    /**
     * @brief Remove and return all queued events.
     *
     * Called from the main thread during Session::poll(). The queue is empty after this call.
     *
     * @return All events that were pending at call time.
     */
    std::vector<Event> drainInboundEvents();

    /**
     * @brief Check whether the inbound queue has no pending events.
     * @return true if the queue is empty.
     */
    bool isEmpty() const;

private:
    mutable std::mutex inboundEventQueueMutex_;
    std::queue<Event> inboundEventQueue_;
};

} // namespace genesis::mudcore

#endif // MUDCORE_EVENT_BUS_HPP
