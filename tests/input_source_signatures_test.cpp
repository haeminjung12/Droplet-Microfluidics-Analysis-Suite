#include "InputSource.h"

#include <cstddef>
#include <type_traits>

namespace {
class DummyInputSource final : public InputSource {
public:
    Type getType() const override { return Type::SingleImage; }
    std::size_t getTotalFrames() const override { return 1; }
    cv::Mat getFrame(std::size_t) override { return {}; }
    double getTimestamp(std::size_t) const override { return 0.0; }
};

static_assert(std::is_abstract_v<InputSource>);

static_assert(std::is_same_v<decltype(&InputSource::getType), InputSource::Type (InputSource::*)() const>);
static_assert(
    std::is_same_v<decltype(&InputSource::getTotalFrames), std::size_t (InputSource::*)() const>);
static_assert(std::is_same_v<decltype(&InputSource::getFrame), cv::Mat (InputSource::*)(std::size_t)>);
static_assert(
    std::is_same_v<decltype(&InputSource::getTimestamp), double (InputSource::*)(std::size_t) const>);

static_assert(std::is_base_of_v<InputSource, DummyInputSource>);
}  // namespace

