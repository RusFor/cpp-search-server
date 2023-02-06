#include "request_queue.h"


RequestQueue::RequestQueue(const SearchServer& search_server) 
    : search_server_(search_server), current_size_(0), current_not_found_ (0) {
    }
   
int RequestQueue::GetNoResultRequests() const {       
    return current_not_found_;    
}