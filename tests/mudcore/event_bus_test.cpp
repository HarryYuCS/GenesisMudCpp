#include <mudcore/event.hpp>
#include <mudcore/event_bus.hpp>

#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <variant>

namespace {

genesis::mudcore::Event makeMudTextEvent(std::string text) {
    genesis::mudcore::Event event;
    event.type = genesis::mudcore::EventType::MudText;
    event.payload = std::move(text);
    return event;
}

} // namespace

TEST(EventBus, EnqueueDrain_PreservesOrder) {
    genesis::mudcore::EventBus bus;

    bus.enqueueInboundEvent(makeMudTextEvent("A"));
    bus.enqueueInboundEvent(makeMudTextEvent("B"));
    bus.enqueueInboundEvent(makeMudTextEvent("C"));

    const auto drained = bus.drainInboundEvents();
    ASSERT_EQ(drained.size(), 3U);
    EXPECT_EQ(std::get<std::string>(drained[0].payload), "A");
    EXPECT_EQ(std::get<std::string>(drained[1].payload), "B");
    EXPECT_EQ(std::get<std::string>(drained[2].payload), "C");
}

TEST(EventBus, Drain_EmptyQueue) {
    genesis::mudcore::EventBus bus;
    EXPECT_TRUE(bus.drainInboundEvents().empty());
    EXPECT_TRUE(bus.isEmpty());
}

TEST(EventBus, MudText_PayloadRoundTrip) {
    genesis::mudcore::EventBus bus;
    bus.enqueueInboundEvent(makeMudTextEvent("Hello, Genesis."));

    const auto drained = bus.drainInboundEvents();
    ASSERT_EQ(drained.size(), 1U);
    EXPECT_EQ(drained[0].type, genesis::mudcore::EventType::MudText);
    EXPECT_EQ(std::get<std::string>(drained[0].payload), "Hello, Genesis.");
    EXPECT_TRUE(bus.isEmpty());
}

TEST(EventBus, GmcpRaw_PayloadRoundTrip) {
    genesis::mudcore::EventBus bus;

    genesis::mudcore::Event event;
    event.type = genesis::mudcore::EventType::GmcpRaw;
    event.payload = std::string(R"(Char.Vitals {"hp":1})");
    bus.enqueueInboundEvent(event);

    const auto drained = bus.drainInboundEvents();
    ASSERT_EQ(drained.size(), 1U);
    EXPECT_EQ(drained[0].type, genesis::mudcore::EventType::GmcpRaw);
    EXPECT_EQ(std::get<std::string>(drained[0].payload), R"(Char.Vitals {"hp":1})");
}

TEST(EventBus, Error_PayloadRoundTrip) {
    genesis::mudcore::EventBus bus;

    genesis::mudcore::Event event;
    event.type = genesis::mudcore::EventType::Error;
    event.payload = genesis::network::NetworkError{"connection reset"};
    bus.enqueueInboundEvent(event);

    const auto drained = bus.drainInboundEvents();
    ASSERT_EQ(drained.size(), 1U);
    EXPECT_EQ(drained[0].type, genesis::mudcore::EventType::Error);
    EXPECT_EQ(std::get<genesis::network::NetworkError>(drained[0].payload).errorMessage, "connection reset");
}

TEST(EventBus, Connected_Disconnected_Monostate) {
    genesis::mudcore::EventBus bus;

    genesis::mudcore::Event connected;
    connected.type = genesis::mudcore::EventType::Connected;
    connected.payload = std::monostate{};

    genesis::mudcore::Event disconnected;
    disconnected.type = genesis::mudcore::EventType::Disconnected;
    disconnected.payload = std::monostate{};

    bus.enqueueInboundEvent(connected);
    bus.enqueueInboundEvent(disconnected);

    const auto drained = bus.drainInboundEvents();
    ASSERT_EQ(drained.size(), 2U);
    EXPECT_EQ(drained[0].type, genesis::mudcore::EventType::Connected);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(drained[0].payload));
    EXPECT_EQ(drained[1].type, genesis::mudcore::EventType::Disconnected);
}

TEST(EventBus, DrainReturnsAllPendingEvents) {
    genesis::mudcore::EventBus bus;

    for (int i = 0; i < 3; ++i) {
        bus.enqueueInboundEvent(makeMudTextEvent("line"));
    }

    const auto drained = bus.drainInboundEvents();
    EXPECT_EQ(drained.size(), 3U);
    EXPECT_TRUE(bus.isEmpty());
}

TEST(EventBus, ConcurrentEnqueue_DrainAll) {
    genesis::mudcore::EventBus bus;
    constexpr int kThreads = 4;
    constexpr int kEventsPerThread = 25;

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&bus]() {
            for (int i = 0; i < kEventsPerThread; ++i) {
                bus.enqueueInboundEvent(makeMudTextEvent("event"));
            }
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }

    const auto drained = bus.drainInboundEvents();
    EXPECT_EQ(drained.size(), static_cast<std::size_t>(kThreads * kEventsPerThread));
}
