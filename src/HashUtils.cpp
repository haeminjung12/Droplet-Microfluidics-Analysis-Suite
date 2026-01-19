#include "HashUtils.h"

#include <array>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>

#include <bcrypt.h>
#endif

namespace HashUtils {
namespace {

constexpr std::size_t kSha256DigestBytes = 32;

std::string toLowerHex(const std::array<std::byte, kSha256DigestBytes>& digest) {
    static constexpr char kHex[] = "0123456789abcdef";

    std::string hex;
    hex.resize(kSha256DigestBytes * 2);

    std::size_t out_index = 0;
    for (const auto b : digest) {
        const auto value = static_cast<unsigned int>(std::to_integer<unsigned char>(b));
        hex[out_index++] = kHex[(value >> 4U) & 0x0FU];
        hex[out_index++] = kHex[value & 0x0FU];
    }

    return hex;
}

#ifdef _WIN32

void throwIfBcryptFailed(NTSTATUS status, const char* message) {
    if (status == 0) {
        return;
    }
    throw std::runtime_error(std::string(message) + " (NTSTATUS=" + std::to_string(status) + ")");
}

class AlgorithmProvider final {
public:
    AlgorithmProvider() {
        const auto status = BCryptOpenAlgorithmProvider(&handle_, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
        throwIfBcryptFailed(status, "BCryptOpenAlgorithmProvider failed");
    }

    AlgorithmProvider(const AlgorithmProvider&) = delete;
    AlgorithmProvider& operator=(const AlgorithmProvider&) = delete;

    AlgorithmProvider(AlgorithmProvider&& other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }
    AlgorithmProvider& operator=(AlgorithmProvider&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        reset();
        handle_ = other.handle_;
        other.handle_ = nullptr;
        return *this;
    }

    ~AlgorithmProvider() { reset(); }

    BCRYPT_ALG_HANDLE get() const { return handle_; }

private:
    void reset() {
        if (handle_ != nullptr) {
            BCryptCloseAlgorithmProvider(handle_, 0);
            handle_ = nullptr;
        }
    }

    BCRYPT_ALG_HANDLE handle_{nullptr};
};

std::array<std::byte, kSha256DigestBytes> sha256Digest(std::span<const std::byte> data) {
    AlgorithmProvider algorithm;

    DWORD object_length = 0;
    ULONG bytes_written = 0;
    auto status = BCryptGetProperty(
        algorithm.get(),
        BCRYPT_OBJECT_LENGTH,
        reinterpret_cast<PUCHAR>(&object_length),
        static_cast<ULONG>(sizeof(object_length)),
        &bytes_written,
        0);
    throwIfBcryptFailed(status, "BCryptGetProperty(BCRYPT_OBJECT_LENGTH) failed");

    DWORD hash_length = 0;
    status = BCryptGetProperty(
        algorithm.get(),
        BCRYPT_HASH_LENGTH,
        reinterpret_cast<PUCHAR>(&hash_length),
        static_cast<ULONG>(sizeof(hash_length)),
        &bytes_written,
        0);
    throwIfBcryptFailed(status, "BCryptGetProperty(BCRYPT_HASH_LENGTH) failed");
    if (hash_length != kSha256DigestBytes) {
        throw std::runtime_error("SHA-256 digest length mismatch: expected 32 bytes");
    }

    std::vector<unsigned char> hash_object(object_length);
    BCRYPT_HASH_HANDLE hash_handle = nullptr;
    status = BCryptCreateHash(
        algorithm.get(),
        &hash_handle,
        reinterpret_cast<PUCHAR>(hash_object.data()),
        static_cast<ULONG>(hash_object.size()),
        nullptr,
        0,
        0);
    throwIfBcryptFailed(status, "BCryptCreateHash failed");

    const auto* bytes = reinterpret_cast<const unsigned char*>(data.data());
    const auto total_size = data.size();
    if (total_size > static_cast<std::size_t>(std::numeric_limits<ULONG>::max())) {
        BCryptDestroyHash(hash_handle);
        throw std::runtime_error("Input too large for BCryptHashData");
    }

    status = BCryptHashData(hash_handle, const_cast<PUCHAR>(bytes), static_cast<ULONG>(total_size), 0);
    if (status != 0) {
        BCryptDestroyHash(hash_handle);
        throwIfBcryptFailed(status, "BCryptHashData failed");
    }

    std::array<std::byte, kSha256DigestBytes> digest{};
    status = BCryptFinishHash(hash_handle, reinterpret_cast<PUCHAR>(digest.data()), kSha256DigestBytes, 0);
    BCryptDestroyHash(hash_handle);
    throwIfBcryptFailed(status, "BCryptFinishHash failed");

    return digest;
}

std::array<std::byte, kSha256DigestBytes> sha256DigestFile(const std::filesystem::path& file_path) {
    if (!std::filesystem::exists(file_path)) {
        throw std::runtime_error("File does not exist: " + file_path.string());
    }
    if (!std::filesystem::is_regular_file(file_path)) {
        throw std::runtime_error("Path is not a regular file: " + file_path.string());
    }

    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + file_path.string());
    }

    AlgorithmProvider algorithm;

    DWORD object_length = 0;
    ULONG bytes_written = 0;
    auto status = BCryptGetProperty(
        algorithm.get(),
        BCRYPT_OBJECT_LENGTH,
        reinterpret_cast<PUCHAR>(&object_length),
        static_cast<ULONG>(sizeof(object_length)),
        &bytes_written,
        0);
    throwIfBcryptFailed(status, "BCryptGetProperty(BCRYPT_OBJECT_LENGTH) failed");

    DWORD hash_length = 0;
    status = BCryptGetProperty(
        algorithm.get(),
        BCRYPT_HASH_LENGTH,
        reinterpret_cast<PUCHAR>(&hash_length),
        static_cast<ULONG>(sizeof(hash_length)),
        &bytes_written,
        0);
    throwIfBcryptFailed(status, "BCryptGetProperty(BCRYPT_HASH_LENGTH) failed");
    if (hash_length != kSha256DigestBytes) {
        throw std::runtime_error("SHA-256 digest length mismatch: expected 32 bytes");
    }

    std::vector<unsigned char> hash_object(object_length);
    BCRYPT_HASH_HANDLE hash_handle = nullptr;
    status = BCryptCreateHash(
        algorithm.get(),
        &hash_handle,
        reinterpret_cast<PUCHAR>(hash_object.data()),
        static_cast<ULONG>(hash_object.size()),
        nullptr,
        0,
        0);
    throwIfBcryptFailed(status, "BCryptCreateHash failed");

    std::vector<unsigned char> buffer(1 << 20);
    while (file) {
        file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        const auto count = file.gcount();
        if (count <= 0) {
            continue;
        }
        if (count > static_cast<std::streamsize>(std::numeric_limits<ULONG>::max())) {
            BCryptDestroyHash(hash_handle);
            throw std::runtime_error("Read chunk too large for BCryptHashData");
        }

        status = BCryptHashData(hash_handle, buffer.data(), static_cast<ULONG>(count), 0);
        if (status != 0) {
            BCryptDestroyHash(hash_handle);
            throwIfBcryptFailed(status, "BCryptHashData failed");
        }
    }

    std::array<std::byte, kSha256DigestBytes> digest{};
    status = BCryptFinishHash(hash_handle, reinterpret_cast<PUCHAR>(digest.data()), kSha256DigestBytes, 0);
    BCryptDestroyHash(hash_handle);
    throwIfBcryptFailed(status, "BCryptFinishHash failed");

    return digest;
}

#else

std::array<std::byte, kSha256DigestBytes> sha256Digest(std::span<const std::byte>) {
    throw std::runtime_error("HashUtils sha256Digest not implemented on this platform");
}

std::array<std::byte, kSha256DigestBytes> sha256DigestFile(const std::filesystem::path&) {
    throw std::runtime_error("HashUtils sha256DigestFile not implemented on this platform");
}

#endif

} // namespace

std::string sha256Hex(std::span<const std::byte> data) { return toLowerHex(sha256Digest(data)); }

std::string sha256Hex(std::string_view text) {
    const auto* bytes = reinterpret_cast<const std::byte*>(text.data());
    return sha256Hex(std::span<const std::byte>(bytes, text.size()));
}

std::string sha256HexFile(const std::filesystem::path& file_path) { return toLowerHex(sha256DigestFile(file_path)); }

} // namespace HashUtils

