#pragma once

#include <string>
#include <vector>
#include <deque>

#include "search_server.h"
#include "document.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query) {    
         return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
    }
    
    int GetNoResultRequests() const ;
private:
    const SearchServer& search_server_;
    struct QueryResult {
        bool found;
    };
    std::deque<QueryResult> requests_;
    int current_size_;
    int current_not_found_;
    const static int min_in_day_ = 1440;
    
}; 

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        std::vector<Document> output =  search_server_.FindTopDocuments(raw_query, document_predicate);
        QueryResult req;
        if(output.empty()) {
            req.found = false;
            ++current_not_found_;
        } else {
            req.found = true;            
        }        
        requests_.push_back(req);
        ++current_size_;
        
        if(current_size_ > min_in_day_) {
             if(!requests_.front().found)
                 --current_not_found_;
            requests_.pop_front();
            --current_size_;
        }
        return output;
    }