#pragma once

#include "search_server.h"
#include <vector>
#include <string>
#include <algorithm>
#include <execution>
#include <unordered_set>

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries);

std::vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries);