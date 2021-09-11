#include <cstdint>
#include <string_view>
#include <string>
#include <tuple>
#include <array>
#include <vector>

class CidrBlock {
private:
    std::array<uint32_t, 4> value_{}; // IPv4 or V6 address in host order, IPv4 is the 0th element
    uint8_t length_ = 0;
    uint8_t family_ = 0;

public:
    using Value = decltype(value_);

    CidrBlock() = default;
    explicit CidrBlock(std::string_view str);

    [[nodiscard]] uint8_t length() const { return length_; }
    [[nodiscard]] const Value &value() const { return value_; }

    [[nodiscard]] bool V6() const;
    [[nodiscard]] bool Valid() const;
    [[nodiscard]] bool Contains(const CidrBlock &b) const;
    [[nodiscard]] std::pair<CidrBlock, CidrBlock> Split() const;

    [[nodiscard]] std::string String() const;

    [[nodiscard]] static std::vector<CidrBlock> Exclude(std::vector<CidrBlock> included,
                                                        const std::vector<CidrBlock> &excluded);
};
