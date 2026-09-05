#pragma once

#include <cstddef>

namespace howlinux {

#ifndef HOWLINUX_VERSION
#define HOWLINUX_VERSION "1.2.0"
#endif
#ifndef HOWLINUX_INSTALL_DATADIR
#define HOWLINUX_INSTALL_DATADIR "share"
#endif

inline constexpr const char* kVersion = HOWLINUX_VERSION;
inline constexpr const char* kInstallDataDirectory = HOWLINUX_INSTALL_DATADIR;

struct SearchConfig {
    double exact_alias_score{100.0};
    double phrase_score{40.0};
    double command_score{30.0};
    double keyword_score{20.0};
    double concept_score{15.0};
    double intent_score{20.0};
    double title_score{10.0};
    double token_score{6.0};
    double fuzzy_score{8.0};

    double confident_score{90.0};
    double confident_margin{15.0};
    double meaningful_score{8.0};

    std::size_t default_limit{5};
    std::size_t maximum_limit{100};
    std::size_t fuzzy_minimum_length{4};
    std::size_t fuzzy_match_limit_per_token{32};
};

}  // namespace howlinux
