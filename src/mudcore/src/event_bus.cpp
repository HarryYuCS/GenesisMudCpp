#include <mudcore/event_bus.hpp>

namespace genesis::mudcore {

void EventBus::enqueueInboundEvent(Event event) {
    std::lock_guard lock(inboundEventQueueMutex_);
    inboundEventQueue_.push(std::move(event));
}

std::vector<Event> EventBus::drainInboundEvents() {
    std::lock_guard lock(inboundEventQueueMutex_);
    std::vector<Event> events;
    events.reserve(inboundEventQueue_.size());
    while (!inboundEventQueue_.empty()) {
        events.push_back(std::move(inboundEventQueue_.front()));
        inboundEventQueue_.pop();
    }
    return events;
}

bool EventBus::isEmpty() const {
    std::lock_guard lock(inboundEventQueueMutex_);
    return inboundEventQueue_.empty();
}

} // namespace genesis::mudcore
