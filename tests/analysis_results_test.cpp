#include "AnalysisResults.h"

#include <gtest/gtest.h>

#if __has_include("DataModels.h")
#include "DataModels.h"
#endif

#include <string>
#include <vector>

namespace {
#if __has_include("DataModels.h")
using FrameDetectionsTestType = FrameDetections;
using TrackTestType = Track;
#else
struct FrameDetectionsTestType {
    int frame_index = 0;
};

struct TrackTestType {
    int track_id = 0;
};
#endif

template <typename Frame>
static void set_frame_index(Frame& frame, int value) {
    if constexpr (requires(Frame& f) { f.frame_index_logical = static_cast<std::size_t>(0); }) {
        frame.frame_index_logical = static_cast<std::size_t>(value);
    } else if constexpr (requires(Frame& f) { f.frame_index = 0; }) {
        frame.frame_index = value;
    }
}

template <typename Frame>
static int get_frame_index(const Frame& frame) {
    if constexpr (requires(const Frame& f) { static_cast<int>(f.frame_index_logical); }) {
        return static_cast<int>(frame.frame_index_logical);
    } else if constexpr (requires(const Frame& f) { f.frame_index; }) {
        return frame.frame_index;
    } else {
        return 0;
    }
}

template <typename Track>
static void set_track_id(Track& track, int value) {
    if constexpr (requires(Track& t) { t.track_id = static_cast<std::size_t>(0); }) {
        track.track_id = static_cast<std::size_t>(value);
    } else if constexpr (requires(Track& t) { t.track_id = 0; }) {
        track.track_id = value;
    }
}

template <typename Track>
static int get_track_id(const Track& track) {
    if constexpr (requires(const Track& t) { static_cast<int>(t.track_id); }) {
        return static_cast<int>(track.track_id);
    } else if constexpr (requires(const Track& t) { t.track_id; }) {
        return track.track_id;
    } else {
        return 0;
    }
}
}  // namespace

TEST(AnalysisResultsTests, StoresAndRetrievesTypedValuesByKey) {
    AnalysisResults results;

    std::vector<FrameDetectionsTestType> frames(2);
    set_frame_index(frames[0], 1);
    set_frame_index(frames[1], 2);

    std::vector<TrackTestType> tracks(1);
    set_track_id(tracks[0], 42);

    results.set("frame_detections", frames);
    results.set("tracks", tracks);

    ASSERT_TRUE(results.has("frame_detections"));
    ASSERT_TRUE(results.has("tracks"));

    const auto& stored_frames =
        results.get<std::vector<FrameDetectionsTestType>>("frame_detections");
    const auto& stored_tracks = results.get<std::vector<TrackTestType>>("tracks");

    ASSERT_EQ(stored_frames.size(), 2u);
    EXPECT_EQ(get_frame_index(stored_frames[0]), 1);
    EXPECT_EQ(get_frame_index(stored_frames[1]), 2);

    ASSERT_EQ(stored_tracks.size(), 1u);
    EXPECT_EQ(get_track_id(stored_tracks[0]), 42);
}
