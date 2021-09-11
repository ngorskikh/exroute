#include <cstdio>
#include <vector>
#include <algorithm>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "CidrBlock.h"

static void PrintUsage(const char *name) {
    std::printf("Exclude one set of CIDR blocks from another.\n"
                "Usage:\n\n\t%s [-h] [-n] (-i <CIDR block>... | -e <CIDR block>...)...\n\n"
                "-h\tPrint this message.\n"
                "-i <CIDR block>...\tAppend specified blocks to the included set.\n"
                "-e <CIDR block>...\tAppend specified blocks to the excluded set.\n"
                "-n\tSeparate output blocks with newlines instead of commas.\n\n"
                "IPv6 and IPv4 blocks can be intermixed."
                " If no included blocks are given, default route is assumed.\n\n"
                "Example:\n\n\t%s -n -i 0.0.0.0/0 -e 0.0.0.0/8 fe80::/10 -i ::/0 -e 224.0.0.0/3 fc00::/7\n\n",
                name, name);
}

int main(int argc, char **argv) {
    std::vector<CidrBlock> in;
    std::vector<CidrBlock> ex;

    const char *separator = ", ";
    bool ip4_present = false;
    bool ip6_present = false;

    if (argc == 1) {
        PrintUsage(argv[0]);
        return -1;
    }

    std::vector<CidrBlock> *dst = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (0 == std::strcmp("-h", argv[i])) {
            PrintUsage(argv[0]);
            return 0;
        }
        if (0 == std::strcmp("-e", argv[i])) {
            dst = &ex;
            continue;
        }
        if (0 == std::strcmp("-i", argv[i])) {
            dst = &in;
            continue;
        }
        if (0 == std::strcmp("-n", argv[i])) {
            separator = "\n";
            dst = nullptr;
            continue;
        }
        if (!dst) {
            PrintUsage(argv[0]);
            return -1;
        }
        dst->emplace_back(argv[i]);
        if (!dst->back().Valid()) {
            fprintf(stderr, "Invalid CIDR block: %s\n", argv[i]);
            return -1;
        }
        if (dst->back().V6()) {
            ip6_present = true;
        } else {
            ip4_present = true;
        }
    }

    if (in.empty()) {
        if (ex.empty()) {
            fprintf(stderr, "At least one included or excluded block must be specified.\n");
            return -1;
        }
        if (ip6_present) {
            in.emplace_back("::/0");
        }
        if (ip4_present) {
            in.emplace_back("0.0.0.0/0");
        }
    }

    in = CidrBlock::Exclude(std::move(in), ex);

    std::sort(in.begin(), in.end(), [](const CidrBlock &a, const CidrBlock &b) {
        if (a.V6() != b.V6()) {
            return !a.V6();
        }
        return a.value() < b.value();
    });

    for (size_t i = 0; i < in.size(); ++i) {
        printf("%s%s", (i > 0) ? separator : "", in[i].String().c_str());
    }

#ifndef _WIN32
    if (isatty(STDOUT_FILENO)) {
        printf("\n");
    }
#else
    printf("\n");
#endif

    return 0;
}
