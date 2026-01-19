#include "AnalysisResults.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {
struct FrameDetections {
    int frame_index = 0;
};

struct Track {
    int track_id = 0;
};
}  // namespace

TEST(AnalysisResultsTests, StoresAndRetrievesTypedValuesByKey) {
    AnalysisResults results;

    std::vector<FrameDetections> frames{{.frame_index = 1}, {.frame_index = 2}};
    std::vector<Track> tracks{{.track_id = 42}};

    results.set("frame_detections", frames);
    results.set("tracks", tracks);

    ASSERT_TRUE(results.has("frame_detections"));
    ASSERT_TRUE(results.has("tracks"));

    const auto& stored_frames = results.get<std::vector<FrameDetections>>("frame_detections");
    const auto& stored_tracks = results.get<std::vector<Track>>("tracks");

    ASSERT_EQ(stored_frames.size(), 2u);
    EXPECT_EQ(stored_frames[0].frame_index, 1);
    EXPECT_EQ(stored_frames[1].frame_index, 2);

    ASSERT_EQ(stored_tracks.size(), 1u);
    EXPECT_EQ(stored_tracks[0].track_id, 42);
}

