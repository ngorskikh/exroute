#include "CidrBlock.h"

#include <charconv>
#include <algorithm>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

CidrBlock::CidrBlock(std::string_view str) {
    auto sp = str.find('/');
    if (sp == std::string_view::npos || sp == str.size() - 1) {
        return;
    }
    std::string_view ls = str.substr(sp + 1);
    if (std::errc{} != std::from_chars(ls.data(), ls.data() + ls.size(), length_).ec) {
        return;
    }
    std::string as{str.substr(0, sp)};
    if (length_ <= 32 && inet_pton(AF_INET, as.c_str(), &value_[0])) {
        family_ = AF_INET;
    } else if (length_ <= 128 && inet_pton(AF_INET6, as.c_str(), &value_[0])) {
        family_ = AF_INET6;
    } else {
        return;
    }
    for (uint32_t &v : value_) {
        v = ntohl(v);
    }
}

std::string CidrBlock::String() const {
    char buf[INET6_ADDRSTRLEN]{};
    Value value = value_;
    for (uint32_t &v : value) {
        v = htonl(v);
    }
    if (inet_ntop(family_, &value, buf, sizeof(buf))) {
        std::string str = buf;
        str.push_back('/');
        str.append(std::to_string(length_));
        return str;
    }
    return "";
}

bool CidrBlock::Contains(const CidrBlock &b) const {
    if (!Valid() || family_ != b.family_ || length_ > b.length_) {
        return false;
    }
    for (size_t i = 0; i < value_.size(); ++i) {
        size_t shift = std::max(size_t(0), 32 * (i + 1) - length_);
        uint32_t mask = (shift >= 32) ? 0 : (UINT32_MAX << shift);
        if ((value_[i] & mask) != (b.value_[i] & mask)) {
            return false;
        }
    }
    return true;
}

std::pair<CidrBlock, CidrBlock> CidrBlock::Split() const {
    std::pair<CidrBlock, CidrBlock> ret;
    if ((family_ == AF_INET && length_ < 32) || (family_ == AF_INET6 && length_ < 128)) {
        ret.first.value_ = ret.second.value_ = value_;
        ret.first.length_ = ret.second.length_ = length_ + 1;
        ret.first.family_ = ret.second.family_ = family_;

        size_t off = length_ / 32;
        size_t shift = 31 - (length_ % 32);

        ret.first.value_[off] &= ~(1 << shift);
        ret.second.value_[off] |= 1 << shift;
    }
    return ret;
}

std::vector<CidrBlock> CidrBlock::Exclude(std::vector<CidrBlock> included, const std::vector<CidrBlock> &excluded) {
    std::vector<CidrBlock> ret;
    while (!included.empty()) {
        const CidrBlock &a = included.back();
        for (const CidrBlock &b : excluded) {
            if (b.Contains(a)) {
                goto drop;
            }
            if (a.Contains(b)) {
                goto split;
            }
        }

        ret.push_back(a);

        drop:
        included.pop_back();
        continue;

        split:
        auto[l, r] = a.Split();
        included.pop_back();
        included.push_back(l);
        included.push_back(r);
    }
    return ret;
}

bool CidrBlock::V6() const { return family_ == AF_INET6; }

bool CidrBlock::Valid() const { return family_ != 0; }
