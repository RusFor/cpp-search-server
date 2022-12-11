
// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
                    "Stop words must be excluded from documents"s);
    }
}

///////////////////////////////////
void TestMinusWords() {
    const int doc_id = 42;
    const string content = "cat in the city minus"s;
    const vector<int> ratings = {1, 2, 3};
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("minus"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    
        {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("-minus"s).empty());
    }
}
//////////////////////////////////////
void TestMatching() {
    const int doc_id = 42;
    const string content = "cat in the city minus"s;
    const vector<int> ratings = {1, 2, 3};
    const tuple<vector<string>, DocumentStatus> compare = {{"city", "minus"}, DocumentStatus::ACTUAL};
    const tuple<vector<string>, DocumentStatus> compare_empty = {{}, DocumentStatus::ACTUAL};

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.MatchDocument("minus city"s, 42);
        ASSERT(found_docs == compare);
    }
    
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.MatchDocument("minus -city"s, 42);
        ASSERT(found_docs == compare_empty);
    }
    
        {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.MatchDocument("minus in city"s, 42);
        ASSERT(found_docs == compare);
    }
}
////////////////////////////////
void TestRelevance() {
    const int ndoc_first = 2;
    const int ndoc_second = 1;
    const int ndoc_third = 0;
    
    {
        SearchServer search_server;
    search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL,
            { 5, -12, 2, 1 });

        const auto& docs = search_server.FindTopDocuments("ухоженный кот"s);
        ASSERT_EQUAL(docs[0].id, ndoc_first);
        ASSERT_EQUAL(docs[1].id, ndoc_second);
        ASSERT_EQUAL(docs[2].id, ndoc_third);
    }

}
////////////////////////////
void TestRating() {
    const int ndoc_first = -1;
    const int ndoc_second = 5;
    const int ndoc_third = 2;
    
    {
        SearchServer search_server;
    search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL,
            { 5, -12, 2, 1 });

        const auto& docs = search_server.FindTopDocuments("ухоженный кот"s);
        ASSERT(docs[0].rating == ndoc_first);
        ASSERT(docs[1].rating == ndoc_second);
        ASSERT(docs[2].rating == ndoc_third);
    }

}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestMatching);
    RUN_TEST(TestRelevance);
    RUN_TEST(TestRating);

}

// --------- Окончание модульных тестов поисковой системы -----------