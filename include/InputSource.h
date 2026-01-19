#pragma once

#include <cstddef>

#include <opencv2/core/mat.hpp>

class InputSource {
public:
    enum class Type { SingleImage, ImageSequence, MultiPageTIFF, CameraStream };

    virtual ~InputSource() = default;
    virtual Type getType() const = 0;
    virtual std::size_t getTotalFrames() const = 0;  // 0 for streams
    virtual cv::Mat getFrame(std::size_t logical_index) = 0;  // Indexed access with caching
    virtual double getTimestamp(std::size_t logical_index) const = 0;  // Based on file order and FPS
};

