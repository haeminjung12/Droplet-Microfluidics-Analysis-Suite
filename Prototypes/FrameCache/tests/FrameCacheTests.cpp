#include "FrameCache.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

struct TestFailure {
    std::string message;
};

void require(bool condition, const std::string& message)
{
    if (!condition) {
        throw TestFailure{message};
    }
}

void shouldNotExceedBudgetAndEvictLRU()
{
    constexpr std::size_t frame_size = 4;
    constexpr std::size_t budget = frame_size * 3;

    std::unordered_map<std::size_t, int> load_count;
    droplet::proto::FrameCache cache(budget);

    auto loader_for = [&](std::size_t id) {
        return [&, id]() {
            load_count[id] += 1;
            return droplet::proto::FrameCache::Frame(frame_size, static_cast<std::uint8_t>(id));
        };
    };

    (void)cache.get(1, loader_for(1));
    (void)cache.get(2, loader_for(2));
    (void)cache.get(3, loader_for(3));
    require(cache.getMemoryUsageBytes() <= budget, "cache exceeded memory budget after initial fills");

    (void)cache.get(1, loader_for(1));  // make 1 MRU, 2 becomes LRU
    (void)cache.get(4, loader_for(4));  // should evict 2
    require(cache.getMemoryUsageBytes() <= budget, "cache exceeded memory budget after eviction");

    (void)cache.get(2, loader_for(2));
    require(load_count[2] == 2, "expected frame 2 to be evicted (loader should be called again)");
}

void shouldNotCacheFramesLargerThanBudget()
{
    constexpr std::size_t frame_size = 4;
    constexpr std::size_t budget = 3;

    std::unordered_map<std::size_t, int> load_count;
    droplet::proto::FrameCache cache(budget);

    auto loader_for = [&](std::size_t id) {
        return [&, id]() {
            load_count[id] += 1;
            return droplet::proto::FrameCache::Frame(frame_size, static_cast<std::uint8_t>(id));
        };
    };

    (void)cache.get(1, loader_for(1));
    (void)cache.get(1, loader_for(1));

    require(cache.getCachedFrames() == 0, "expected no caching when frame size exceeds budget");
    require(cache.getMemoryUsageBytes() == 0, "expected memory usage to remain zero when caching is disabled");
    require(load_count[1] == 2, "expected loader to be called every time when caching is disabled");
}

}  // namespace

int main()
{
    try {
        shouldNotExceedBudgetAndEvictLRU();
        shouldNotCacheFramesLargerThanBudget();
    } catch (const TestFailure& failure) {
        std::cerr << "FAILED: " << failure.message << "\n";
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << "\n";
        return 1;
    }

    std::cout << "OK\n";
    return 0;
}
