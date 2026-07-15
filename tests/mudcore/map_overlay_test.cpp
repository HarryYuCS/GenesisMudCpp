#include <mudcore/map_overlay.hpp>

#include <gtest/gtest.h>

using genesis::mudcore::overlayPlayerMarker;

TEST(MapOverlay, PlacesXAtCoordinates) {
    const std::string map =
        "...\n"
        "...\n"
        "...";
    EXPECT_EQ(overlayPlayerMarker(map, 1, 1),
              "...\n"
              ".X.\n"
              "...");
}

TEST(MapOverlay, PlacesXOnFirstLine) {
    EXPECT_EQ(overlayPlayerMarker("abc", 0, 0), "Xbc");
}

TEST(MapOverlay, OutOfBoundsReturnsUnchanged) {
    const std::string map = "ab\ncd";
    EXPECT_EQ(overlayPlayerMarker(map, 5, 0), map);
    EXPECT_EQ(overlayPlayerMarker(map, 0, 5), map);
    EXPECT_EQ(overlayPlayerMarker(map, -1, 0), map);
    EXPECT_EQ(overlayPlayerMarker(map, 0, -1), map);
}

TEST(MapOverlay, EmptyMapUnchanged) {
    EXPECT_EQ(overlayPlayerMarker("", 0, 0), "");
}

TEST(MapOverlay, PreservesTrailingNewlineStructure) {
    // Split treats a trailing newline as an extra empty line; OOB on that line is fine.
    const std::string map = "ab\n";
    EXPECT_EQ(overlayPlayerMarker(map, 1, 0), "aX\n");
}
