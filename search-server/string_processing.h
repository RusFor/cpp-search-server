#pragma once

#include <string>
#include <set>
#include <vector>




std::vector<std::string_view> SplitIntoWordsView(const std::string_view text);

template <typename StringContainer>
std::set<std::string, std::less<>> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set<std::string, std::less<>> non_empty_strings;
    for (const auto& data : strings) {
        if (!data.empty()) {
            non_empty_strings.emplace(data);
        }
    }
    return non_empty_strings;
}