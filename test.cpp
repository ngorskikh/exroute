#include <cstdio>
#include <algorithm>

#include "CidrBlock.h"

#define ASSERT(x) do { if (!(x)) { fprintf(stderr, "Failed: %s\n", #x); return -1; } } while (0)

int main() {
    ASSERT(!CidrBlock("0.0.0.0/0").V6());
    ASSERT(CidrBlock("0.0.0.0/0").String() == "0.0.0.0/0");
    ASSERT(CidrBlock("127.0.0.1/32").String() == "127.0.0.1/32");
    ASSERT(CidrBlock("127.0.0.0/8").String() == "127.0.0.0/8");
    ASSERT(CidrBlock("192.168.0.0/16").String() == "192.168.0.0/16");

    ASSERT(CidrBlock("::/0").V6());
    ASSERT(CidrBlock("::/0").String() == "::/0");
    ASSERT(CidrBlock("::1/128").String() == "::1/128");
    ASSERT(CidrBlock("fc00::/7").String() == "fc00::/7");
    ASSERT(CidrBlock("2000::/3").String() == "2000::/3");

    ASSERT(CidrBlock("192.168.42.53/24").String() == "192.168.42.53/24");
    ASSERT(CidrBlock("192.168.1.0/23").String() == "192.168.1.0/23");
    ASSERT(CidrBlock("cafe:babe::/48").String() == "cafe:babe::/48");
    ASSERT(CidrBlock("cafe:babe::/24").String() == "cafe:babe::/24");
    ASSERT(CidrBlock("cafe:babe::/20").String() == "cafe:babe::/20");

    ASSERT(!CidrBlock("").Valid());
    ASSERT(!CidrBlock("asdf").Valid());
    ASSERT(!CidrBlock("/").Valid());
    ASSERT(!CidrBlock("::/256").Valid());
    ASSERT(!CidrBlock("10/12").Valid());

    ASSERT(CidrBlock("0.0.0.0/0").Valid());
    ASSERT(CidrBlock("0.0.0.0/32").Valid());
    ASSERT(CidrBlock("::/0").Valid());
    ASSERT(CidrBlock("::/128").Valid());

    ASSERT(CidrBlock("0.0.0.0/0").Contains(CidrBlock("127.0.0.0/8")));
    ASSERT(CidrBlock("::/0").Contains(CidrBlock("cafe:babe::/31")));

    ASSERT(CidrBlock("192.168.0.0/23").Contains(CidrBlock("192.168.1.2/32")));
    ASSERT(!CidrBlock("192.168.0.0/24").Contains(CidrBlock("192.168.1.2/32")));

    ASSERT(CidrBlock("172.16.0.0/12").Contains(CidrBlock("172.31.1.0/24")));
    ASSERT(CidrBlock("172.16.0.0/12").Contains(CidrBlock("172.22.0.0/16")));
    ASSERT(CidrBlock("172.0.0.0/11").Contains(CidrBlock("172.16.0.0/12")));

    ASSERT(!CidrBlock("172.16.0.0/12").Contains(CidrBlock("172.32.0.0/12")));
    ASSERT(!CidrBlock("172.16.0.0/12").Contains(CidrBlock("172.0.0.0/11")));

    ASSERT(!CidrBlock("192.168.1.0/24").Contains(CidrBlock("192.168.2.0/24")));
    ASSERT(!CidrBlock("192.168.2.0/24").Contains(CidrBlock("192.168.1.0/24")));

    ASSERT(CidrBlock("2000::/3").Contains(CidrBlock("2606:4700:4700::/48")));
    ASSERT(!CidrBlock("2606:4700:4700::/48").Contains(CidrBlock("2000::/3")));

    ASSERT(CidrBlock("fc00::/7").Contains(CidrBlock("fd8a:feed:beef::/64")));
    ASSERT(!CidrBlock("fc00::/7").Contains(CidrBlock("2001:4860:4860::/48")));

    ASSERT(!CidrBlock("2000::/3").Contains(CidrBlock("fc00::/7")));
    ASSERT(!CidrBlock("fc00::/7").Contains(CidrBlock("2000::/3")));

    ASSERT(!CidrBlock("fc00::/7").Contains(CidrBlock("fe80::/10")));
    ASSERT(!CidrBlock("fe80::/10").Contains(CidrBlock("fc00::/7")));

    ASSERT(!CidrBlock("0.0.0.0/0").Contains(CidrBlock("::/0")));
    ASSERT(!CidrBlock("::/0").Contains(CidrBlock("0.0.0.0/0")));

    ASSERT(!CidrBlock("::/0").Contains(CidrBlock()));
    ASSERT(!CidrBlock("0.0.0.0/0").Contains(CidrBlock()));

    ASSERT(!CidrBlock().Contains(CidrBlock()));
    ASSERT(!CidrBlock().Contains(CidrBlock("1.1.0.0/15")));
    ASSERT(!CidrBlock().Contains(CidrBlock("feed::/9")));

    auto[b1, b2] = CidrBlock("0.0.0.0/0").Split();
    ASSERT(b1.String() == "0.0.0.0/1");
    ASSERT(b2.String() == "128.0.0.0/1");

    auto[b3, b4] = CidrBlock("192.168.0.0/23").Split();
    ASSERT(b3.String() == "192.168.0.0/24");
    ASSERT(b4.String() == "192.168.1.0/24");

    auto[b5, b6] = CidrBlock("abab:abab::/32").Split();
    ASSERT(b5.String() == "abab:abab::/33");
    ASSERT(b6.String() == "abab:abab:8000::/33");

    auto[b7, b8] = CidrBlock("1.1.1.1/32").Split();
    ASSERT(!b7.Valid() && !b8.Valid());

    auto[b9, b10] = CidrBlock("::1/128").Split();
    ASSERT(!b9.Valid() && !b10.Valid());

    auto[b11, b12] = CidrBlock("::/0").Split();
    ASSERT(b11.String() == "::/1");
    ASSERT(b12.String() == "8000::/1");

    auto e1 = CidrBlock::Exclude({CidrBlock("::/0")}, {CidrBlock("::/1")});
    ASSERT(e1.size() == 1 && e1.front().String() == "8000::/1");

    auto e2 = CidrBlock::Exclude({CidrBlock("0.0.0.0/0")}, {CidrBlock("128.0.0.0/1")});
    ASSERT(e2.size() == 1 && e2.front().String() == "0.0.0.0/1");

    auto e3 = CidrBlock::Exclude({CidrBlock("::/0"), CidrBlock("0.0.0.0/0")}, {CidrBlock("8000::/1"), CidrBlock("0.0.0.0/1")});
    ASSERT(e3.size() == 2);
    ASSERT(std::any_of(e3.begin(), e3.end(), [](const CidrBlock &b) {
        return b.String() == "::/1";
    }));
    ASSERT(std::any_of(e3.begin(), e3.end(), [](const CidrBlock &b) {
        return b.String() == "128.0.0.0/1";
    }));

    std::vector<CidrBlock> excluded{CidrBlock("192.168.1.3/32"), CidrBlock("192.168.2.128/25"), CidrBlock("192.168.16/22")};
    std::vector<CidrBlock> included{CidrBlock("192.168.128.0/17"), CidrBlock("192.168.0.0/17")};
    auto e4 = CidrBlock::Exclude(std::move(included), excluded);
    for (int i = 0; i < UINT16_MAX; ++i) {
        char buf[128];
        std::sprintf(buf, "192.168.%i.%i/32", i / 256, i % 256);
        CidrBlock address(buf);
        if (std::none_of(excluded.begin(), excluded.end(), [&](const CidrBlock &b) { return b.Contains(address); })) {
            ASSERT(std::any_of(e4.begin(), e4.end(), [&](const CidrBlock &b) { return b.Contains(address); }));
        }
    }

    CidrBlock b("asdf/32");
    ASSERT(!b.Valid());
    ASSERT(b.String() != "asdf/32");

    return 0;
}
