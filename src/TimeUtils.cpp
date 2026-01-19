#include "TimeUtils.h"

namespace TimeUtils {

double inferredTimestamp(const std::size_t frame_index, const double fps_manual) {
    if (fps_manual <= 0.0) {
        return 0.0;
    }
    return static_cast<double>(frame_index) / fps_manual;
}

} // namespace TimeUtils

