#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <list>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace droplet::proto {

class FrameCache {
public:
    using Frame = std::vector<std::uint8_t>;

    explicit FrameCache(std::size_t memory_budget_bytes = 512ULL * 1024ULL * 1024ULL)
        : memory_budget_(memory_budget_bytes) {}

    Frame get(std::size_t frame_id, const std::function<Frame()>& loader)
    {
        auto it = cache_.find(frame_id);
        if (it != cache_.end()) {
            touch(frame_id);
            return it->second;
        }

        Frame frame = loader();
        if (frame_size_bytes_ == 0) {
            frame_size_bytes_ = frame.size();
            max_frames_ = frame_size_bytes_ == 0 ? 0 : (memory_budget_ / frame_size_bytes_);
        } else if (frame.size() != frame_size_bytes_) {
            throw std::invalid_argument("FrameCache: frame size must remain constant");
        }

        if (max_frames_ == 0) {
            return frame;
        }

        evictIfNeededForInsert();
        cache_.emplace(frame_id, frame);
        lru_order_.push_front(frame_id);
        return cache_.at(frame_id);
    }

    void clear()
    {
        cache_.clear();
        lru_order_.clear();
        frame_size_bytes_ = 0;
        max_frames_ = 0;
    }

    std::size_t getMaxFrames() const { return max_frames_; }
    std::size_t getMemoryBudgetBytes() const { return memory_budget_; }
    std::size_t getFrameSizeBytes() const { return frame_size_bytes_; }
    std::size_t getCachedFrames() const { return cache_.size(); }
    std::size_t getMemoryUsageBytes() const { return cache_.size() * frame_size_bytes_; }

    void setMemoryBudget(std::size_t bytes)
    {
        memory_budget_ = bytes;
        max_frames_ = frame_size_bytes_ == 0 ? 0 : (memory_budget_ / frame_size_bytes_);
        evictIfOverCapacity();
    }

private:
    void evictIfOverCapacity()
    {
        if (max_frames_ == 0) {
            cache_.clear();
            lru_order_.clear();
            return;
        }

        while (cache_.size() > max_frames_ && !lru_order_.empty()) {
            const std::size_t lru_frame_id = lru_order_.back();
            lru_order_.pop_back();
            cache_.erase(lru_frame_id);
        }
    }

    void evictIfNeededForInsert()
    {
        if (max_frames_ == 0) {
            return;
        }

        while (cache_.size() + 1 > max_frames_ && !lru_order_.empty()) {
            const std::size_t lru_frame_id = lru_order_.back();
            lru_order_.pop_back();
            cache_.erase(lru_frame_id);
        }
    }

    void touch(std::size_t frame_id)
    {
        lru_order_.remove(frame_id);
        lru_order_.push_front(frame_id);
    }

    std::size_t memory_budget_{0};
    std::size_t frame_size_bytes_{0};
    std::size_t max_frames_{0};

    std::unordered_map<std::size_t, Frame> cache_;
    std::list<std::size_t> lru_order_;
};

}  // namespace droplet::proto
