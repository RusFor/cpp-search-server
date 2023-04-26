#include "search_server.h"
#include <iterator>


SearchServer::SearchServer(const std::string_view stop_words_text) : SearchServer(
    SplitIntoWordsView(stop_words_text))
{
}

SearchServer::SearchServer(const std::string& stop_words_text) : SearchServer(
    SplitIntoWordsView(std::string_view(stop_words_text)))
{
}

void SearchServer::AddDocument(int document_id, const std::string_view document,
    DocumentStatus status, const std::vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id"s);
    }
    documents_strings_.push_back(std::string(document));
    const std::vector<std::string_view> words{ SplitIntoWordsNoStop(documents_strings_.back()) };
    const double inv_word_count = 1.0 / words.size();
    for (const std::string_view& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        document_to_word_freqs_[document_id][word] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    document_ids_.insert(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query,
        [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::execution::sequenced_policy&, const std::string_view& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query,
        [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::execution::sequenced_policy&, const std::string_view& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::execution::parallel_policy&, const std::string_view& raw_query, DocumentStatus status) const {
    return FindTopDocuments(std::execution::par, raw_query,
        [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::execution::parallel_policy&, const std::string_view& raw_query) const {
    return FindTopDocuments(std::execution::par, raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return static_cast<int>(documents_.size());
}

const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const
{
    static const std::map<std::string_view, double>& freqs{};
    return document_to_word_freqs_.count(document_id) == 0 ?
        freqs : document_to_word_freqs_.at(document_id);
}

void SearchServer::RemoveDocument(int document_id) {
    for (auto& [word, freq] : document_to_word_freqs_.at(document_id)) {
        word_to_document_freqs_.at(word).erase(document_id);
    }
    //auto it = std::remove(document_ids_.begin(), document_ids_.end(), document_id);
    document_ids_.erase(document_id);
    documents_.erase(document_id);
    document_to_word_freqs_.erase(document_id);
}

void SearchServer::RemoveDocument(const std::execution::sequenced_policy& policy, int document_id) {
    SearchServer::RemoveDocument(document_id);
}

void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id) {
    //сокращаем
    auto ptr_doc = &document_to_word_freqs_.at(document_id);
    //размер вектора равен размеру словаря нужного документа
    std::vector<std::string_view> wptr_for_remove(ptr_doc->size());

    std::transform(std::execution::par, ptr_doc->begin(), ptr_doc->end(), wptr_for_remove.begin(), [](auto& container) {
        //возвращаем указатель на слово в словаре, которое есть в документе
        return container.first;
        });
    auto lambda = [this, &document_id](const auto& ptr_str) {
        //удаляем id из словаря со словами
        return word_to_document_freqs_.at(ptr_str).erase(document_id);
    };
    std::for_each(std::execution::par, wptr_for_remove.begin(), wptr_for_remove.end(), lambda);
    document_ids_.erase(document_id);
    documents_.erase(document_id);
    document_to_word_freqs_.erase(document_id);
}

const std::map<int, std::map<std::string_view, double>> SearchServer::GetDocuments() {
    return document_to_word_freqs_;
}

std::set<int>::iterator SearchServer::begin() {
    return document_ids_.begin();
}

std::set<int>::iterator SearchServer::end() {
    return document_ids_.end();
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument
(const std::string_view& raw_query, int document_id) const {
    if (raw_query.empty()) throw std::invalid_argument("Query string is empty"s);
    if (document_id > GetDocumentCount()) throw std::out_of_range("Document ID out the range."s);
    const auto query = ParseQuery(raw_query);
    std::vector<std::string_view> matched_words;
    for (const std::string_view& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            return { matched_words, documents_.at(document_id).status };
        }
    }
    for (const std::string_view& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    std::sort(matched_words.begin(), matched_words.end());
    return { matched_words, documents_.at(document_id).status };
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument
(const std::execution::sequenced_policy&, const std::string_view& raw_query, int document_id) const {
    return MatchDocument(raw_query, document_id);
}
//параллельная версия
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument
(const std::execution::parallel_policy&, const std::string_view& raw_query, int document_id) const {
    if (raw_query.empty()) throw std::invalid_argument("Query string is empty"s);
    if (document_id > GetDocumentCount()) throw std::out_of_range("Document ID out the range."s);
    auto query = ParseQuery(raw_query, false);
    std::vector<std::string_view> matched_words;
    if (std::any_of(std::execution::par, query.minus_words.begin(), query.minus_words.end(),
        [this, document_id](const std::string_view& minus_word) {
            return (word_to_document_freqs_.count(minus_word) != 0) && (word_to_document_freqs_.at(minus_word).count(document_id) != 0);
        })) {
        return { matched_words, documents_.at(document_id).status };
    }
    else {
        matched_words.resize(query.plus_words.size());
        std::copy_if(std::execution::par, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(),
            [this, document_id](const std::string_view& word) {
                return (word_to_document_freqs_.count(word) != 0) && (word_to_document_freqs_.at(word).count(document_id));
            });
        auto it_clear_empty = find(matched_words.begin(), matched_words.end(), ""s);
        matched_words.erase(it_clear_empty, matched_words.end());
        std::sort(std::execution::par, matched_words.begin(), matched_words.end());          
        auto it_clear_unique = std::unique(matched_words.begin(), matched_words.end());
        matched_words.erase(it_clear_unique, matched_words.end());
    }
    return { matched_words, documents_.at(document_id).status };
}

bool SearchServer::IsStopWord(const std::string_view word) const {
    return stop_words_.count(word) != 0;
}

bool SearchServer::IsValidWord(const std::string_view word) {
    // A valid word must not contain special characters
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view& text) const {
    std::vector<std::string_view> words;
    for (const std::string_view word : SplitIntoWordsView(text)) {
        if (!IsValidWord(word.substr())) {
            throw std::invalid_argument("Word "s + std::string(word) + " is invalid"s);
        }
        if (!IsStopWord(word.substr())) {
            words.push_back(word.substr());
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string_view& text) const {
    if (text.empty()) {
        throw std::invalid_argument("Query word is empty"s);
    }
    std::string_view word{ text };
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw std::invalid_argument("Query word "s + std::string(text) + " is invalid");
    }

    return { word, is_minus, IsStopWord(word) };
}



SearchServer::Query SearchServer::ParseQuery(const std::string_view& text, bool sort) const {
    Query result;
    for (const std::string_view& word : SplitIntoWordsView(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.push_back(query_word.data);
            }
            else {
                result.plus_words.push_back(query_word.data);
            }
        }
    }
    if (sort) {
        std::sort(result.plus_words.begin(), result.plus_words.end());
        std::sort(result.minus_words.begin(), result.minus_words.end());
        auto it_er = std::unique(result.plus_words.begin(), result.plus_words.end());
        result.plus_words.erase(it_er, result.plus_words.end());
        it_er = std::unique(result.minus_words.begin(), result.minus_words.end());
        result.minus_words.erase(it_er, result.minus_words.end());
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string_view& word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

