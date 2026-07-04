#include <mudcore/event_bus.hpp>

#include <utility>

namespace genesis::mudcore {

void EventBus::enqueueInboundEvent(Event event) {
    std::lock_guard lock(inboundEventQueueMutex_);
    inboundEventQueue_.push_back(std::move(event));
}

std::vector<Event> EventBus::drainInboundEvents() {
    std::vector<Event> events;
    std::lock_guard lock(inboundEventQueueMutex_);
    std::swap(events, inboundEventQueue_);
    return events;
}

bool EventBus::isEmpty() const {
    std::lock_guard lock(inboundEventQueueMutex_);
    return inboundEventQueue_.empty();
}

} // namespace genesis::mudcore
