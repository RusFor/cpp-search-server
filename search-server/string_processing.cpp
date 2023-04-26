#include <string>
#include <iostream>
#include <vector>
#include <set>

#include "string_processing.h"


using namespace std;

std::vector<std::string_view> SplitIntoWordsView(std::string_view str) {
    vector<string_view> result;
    str.remove_prefix(min(str.size(), str.find_first_not_of(" "sv)));

    while (!str.empty()) {
        int64_t space = min(str.find(" "sv), str.size());
        result.push_back(str.substr(0, space));
        str.remove_prefix(space);
        str.remove_prefix(min(str.size(), str.find_first_not_of(" "sv)));
    }

    return result;
}

/*
std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}
*/