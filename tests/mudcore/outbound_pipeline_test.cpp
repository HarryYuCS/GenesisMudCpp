#include <mudcore/outbound_pipeline.hpp>

#include <gtest/gtest.h>

TEST(OutboundPipeline, Process_PassThrough) {
    genesis::mudcore::OutboundPipeline pipeline;
    const auto result = pipeline.process("look");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "look");
}

TEST(OutboundPipeline, Process_EmptyInput) {
    genesis::mudcore::OutboundPipeline pipeline;
    EXPECT_FALSE(pipeline.process("").has_value());
}

TEST(OutboundPipeline, Process_PreservesWhitespace) {
    genesis::mudcore::OutboundPipeline pipeline;
    const auto result = pipeline.process("  look  ");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "  look  ");
}
