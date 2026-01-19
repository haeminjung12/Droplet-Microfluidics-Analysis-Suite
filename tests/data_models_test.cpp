#include "DataModels.h"

#include <gtest/gtest.h>

TEST(DataModels, CanInstantiateAndAccessMembers) {
    Detection detection{};
    detection.centroid = cv::Point2f(1.0F, 2.0F);
    detection.area_px2 = 3.0F;
    detection.perimeter_px = 4.0F;
    detection.diameter_eq_px = 5.0F;
    detection.major_axis_px = 6.0F;
    detection.minor_axis_px = 7.0F;
    detection.angle_deg = 8.0F;
    detection.circularity = 9.0F;
    detection.aspect_ratio = 10.0F;
    detection.bounding_box = cv::Rect(11, 12, 13, 14);
    detection.contour.push_back(cv::Point(15, 16));
    detection.fluorescence["gfp"] = FluorescenceMetrics{
        .mean = 17.0F,
        .integrated = 18.0F,
        .min = 19.0F,
        .max = 20.0F,
        .bg_corrected_mean = 21.0F,
        .sbr = 22.0F,
        .bg_method = "local_annulus",
    };

    FrameDetections frame{};
    frame.frame_index_logical = 23U;
    frame.timestamp_inferred_s = 24.0;
    frame.detections.push_back(detection);

    Track track{};
    track.track_id = 25U;
    track.droplet_ids.push_back(26U);
    track.frame_indices.push_back(27U);
    track.centroids.push_back(cv::Point2f(28.0F, 29.0F));
    track.timestamps.push_back(30.0);
    track.velocities.push_back(31.0F);
    track.crossed_line = true;
    track.line_crossing_frame = 32U;

    EXPECT_FLOAT_EQ(frame.detections.front().centroid.x, 1.0F);
    EXPECT_EQ(track.track_id, 25U);
    EXPECT_EQ(track.droplet_ids.front(), 26U);
    EXPECT_TRUE(track.crossed_line);
    EXPECT_EQ(track.line_crossing_frame, 32U);
    EXPECT_EQ(frame.detections.front().fluorescence.at("gfp").bg_method, "local_annulus");
}

