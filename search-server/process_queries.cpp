#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> output(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), output.begin(), [&search_server](const std::string& query) {
        return search_server.FindTopDocuments(query);
        });
    return output;
}

std::vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
    std::vector<Document> output; 
    output.reserve(queries.size() * MAX_RESULT_DOCUMENT_COUNT);
    std::vector<std::vector<Document>> documents = ProcessQueries(search_server, queries);

    for (const std::vector<Document>& docs : documents) {
        for (const Document doc : docs) {
            output.push_back(doc);
        }
    }
    return output;
}