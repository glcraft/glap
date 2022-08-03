#include <cstddef>
#include <string_view>
#include <vector>
#include <span>
class Compress {
public:
    virtual ~Compress() = default;

    virtual std::vector<std::byte> compress(std::span<std::byte> data) const = 0;
    virtual std::vector<std::byte> decompress(std::span<std::byte> data) const = 0;
    virtual std::size_t compressed_size(std::span<std::byte> data) const = 0;
    virtual std::size_t decompressed_size(std::span<std::byte> data) const = 0;

    virtual std::string_view name() const = 0;
    virtual std::string_view extension() const = 0;
};