#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {
	std::vector<std::pair<std::vector<std::string>, int>> text_id;
	auto data = search_server.GetDocuments();
	std::vector<std::string> str{};
	for (const auto& [id, words] : data) {
		for (const auto& [word, freq] : words) {
			str.push_back(word);
		}
		sort(str.begin(), str.end());
		text_id.push_back({ str, id });
		str.clear();
	}

	for (int i = 0; i < text_id.size() - 1; ++i) {
		for (int j = i + 1; j < text_id.size(); ++j) {
			if (text_id[i].first == text_id[j].first) {
				int del;
				if (text_id[i].second < text_id[j].second) {
					del = text_id[j].second;
					text_id.erase(text_id.begin() + j);
					--j;
				}
				else {
					del = text_id[i].second;
					text_id.erase(text_id.begin() + i);
					--i;
				}
				std::cout << "Found duplicate document id "s << del << std::endl;
				search_server.RemoveDocument(del);
			}
		}
	}
}
