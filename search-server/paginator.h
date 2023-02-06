#pragma once
#include <vector>
#include <iostream>

#include "document.h"

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end)
    : begin_(begin), end_(end) {}
    
    Iterator begin() const{
        return begin_;
    }    
    Iterator end() const{
        return end_;
    }
    
    int size() const{
        return distance(begin_, end_);
    }
    
private:
    Iterator begin_;
    Iterator end_;

};

template <typename Iterator>
class Paginator {
public:
    
    Paginator(Iterator begin, Iterator end, int page_size) 
    : pages_size_(page_size) {
        for(auto it = begin; distance(it, end) > 0; it += page_size) {
            pages_.push_back({it, it + pages_size_});
        }
    }   
  
    auto begin() const{
        return pages_.begin();
    }    
    auto end() const{
        return pages_.end();
    }
    
    int size() const{
        return pages_size_;
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
    int pages_size_;
};



template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

std::ostream& operator << (std::ostream& out, const Document search){
    return out << "{ document_id = " << search.id << ", relevance = " << search.relevance << ", rating = " << search.rating << " }";
 }

template<typename Iterator>
std::ostream& operator << (std::ostream& out, IteratorRange<Iterator> page){
    for (auto i = page.begin(); i < page.end(); i++) {
        out << *i;
    }
    return out;
}