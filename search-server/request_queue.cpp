#include "request_queue.h"


RequestQueue::RequestQueue(const SearchServer& search_server) 
    : search_server_(search_server), current_size_(0), current_not_found_ (0) {
}
   
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    return AddFindRequest(raw_query, 
        [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });    
}

int RequestQueue::GetNoResultRequests() const {       
    return current_not_found_;    
}