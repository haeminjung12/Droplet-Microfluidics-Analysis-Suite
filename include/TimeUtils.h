#pragma once

#include <cstddef>

namespace TimeUtils {
    // Spec reference: Docs/TECHSPEC_SPLIT/06_data_formats.md (`timestamp_inferred_s = file_order_index / fps_manual`)

    // Returns inferred timestamp in seconds for a logical frame index and manual FPS.
    // For non-positive FPS, returns 0.0.
    double inferredTimestamp(std::size_t frame_index, double fps_manual);
}

