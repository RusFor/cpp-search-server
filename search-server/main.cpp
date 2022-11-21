#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        double words_size = words.size();
        for (const string& word : words) {
            double TF = (double)count(words.begin(), words.end(), word) / words_size;
            word_to_document_freqs_[word].insert({document_id, TF});
        }
        ++document_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:

    struct Query {
        set <string> minus_words;
        set <string> plus_words;
    };

    map<string, map<int, double>> word_to_document_freqs_;
    set<string> stop_words_;
    int document_count_ = 0;
    
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-' && !IsStopWord(word.substr(1))) {
                query.minus_words.insert(word.substr(1));
            }
            else if (!IsStopWord(word)) {
                query.plus_words.insert(word);
            }
        }
        return query;
    }

    vector<Document> FindAllDocuments(const Query& query) const {
        map<int, double> document_to_relevance;
        vector<Document> matched_documents;
        
        for (const auto& plus_word : query.plus_words) {
            if (word_to_document_freqs_.count(plus_word)!= 0) {
                double document_with_plusword_size = word_to_document_freqs_.at(plus_word).size();
                for (const auto& [id , TF] : word_to_document_freqs_.at(plus_word)) {
                    document_to_relevance[id] += TF * log(document_count_ / document_with_plusword_size);
                }
            }            
        }

        for (const auto& minus_word : query.minus_words) {
            if (word_to_document_freqs_.count(minus_word) != 0) {
                for (const auto& [id, TF] : word_to_document_freqs_.at(minus_word)) {
                    if(document_to_relevance.count(id)){
                        document_to_relevance.erase(id);
                    }
                }
            }
        }

        for (const auto& [id, relevance] : document_to_relevance) {
            matched_documents.push_back({ id, relevance });
        }

        return matched_documents;
    }

 /*   static int MatchDocument(const DocumentContent& content, const Query& query) {
        if (query.plus_words.empty()) {
            return 0;
        }
        set<string> matched_words;
        for (const string& word : content.words) {
            if (matched_words.count(word) != 0) {
                continue;
            }
            if (query.minus_words.count(word) != 0) {
                return 0;
            }
            if (query.plus_words.count(word) != 0) {
                matched_words.insert(word);
            }
        }
        return static_cast<int>(matched_words.size());
    }*/
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
            << "relevance = "s << relevance << " }"s << endl;
    }
}  