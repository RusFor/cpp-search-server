#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {
	std::map<std::multiset<std::string_view>, int> m_words;
	auto data = search_server.GetDocuments();
	std::multiset<std::string_view> str{};
	for (const auto& [id, words] : data) {
		for (const auto& [word, freq] : words) {
			str.insert(word);
		}
		if (m_words.count(str)) {
			int del;
			if (m_words.at(str) > id) {
				del = m_words.at(str);
				m_words.at(str) = id;
			}
			else {
				del = id;
			}
			std::cout << "Found duplicate document id "s << del << std::endl;
			search_server.RemoveDocument(del);
		} else {
		m_words[str] = id;
	}
		str.clear();
	}
}
