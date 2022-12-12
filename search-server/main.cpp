
// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    constexpr int doc_id = 42;
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
void TestFindNoDocumentssWithMinusWords() {
    constexpr int doc_id = 42;
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
void TestFindMatchingDocuments() {
    constexpr int doc_id = 42;
    const string content = "cat in the city minus"s;
    const vector<int> ratings = {1, 2, 3};
    const tuple<vector<string>, DocumentStatus> compare = {{"city", "minus"}, DocumentStatus::ACTUAL};
    const tuple<vector<string>, DocumentStatus> compare_empty = {{}, DocumentStatus::ACTUAL};

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.MatchDocument("minus in the city"s, 42);
        ASSERT(found_docs == compare);
    }
    
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.MatchDocument("minus -city"s, 42);
        ASSERT(found_docs == compare_empty);
    }
}
////////////////////////////////
void TestChekRelevance() {

    {
        SearchServer search_server;
    search_server.SetStopWords("и в на"s);
search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9});
        
        //const double dFirstIDF = log(search_server.GetDocumentCount() * 1.0 / 1);
        //const double dSecondIDF = log(search_server.GetDocumentCount() * 1.0 / 2);
        //const double dThirdIDF = log(search_server.GetDocumentCount() * 1.0 / 2);
        
       // const double drelevance_first =  dFirstIDF * (2.0 / 4) + dThirdIDF * (1.0 / 4);
       // const double drelevance_second = dSecondIDF * (1.0 / 4) + dThirdIDF * (1.0 / 4);
       //const double drelevance_third = dSecondIDF * (1.0 / 4) + dThirdIDF * (1.0 / 4);
        
        const auto& docs = search_server.FindTopDocuments("пушистый ухоженный кот"s);
        ASSERT_HINT(docs[0].relevance > docs[1].relevance, "Релевантность первого документа должна быть больше!"s);
        ASSERT_HINT(docs[1].relevance == docs[2].relevance, "Релевантность второго документа должна быть равна релевантности 3го!"s);
        
        //ASSERT_EQUAL_HINT(docs[1].relevance, drelevance_second, "Неправильно посчитана релевантность"s); 
        //ASSERT_EQUAL_HINT(docs[2].relevance, drelevance_third, "Неправильно посчитана релевантность"s);
        
        //{ document_id = 1, relevance = 0.866434, rating = 5 }
        //{ document_id = 0, relevance = 0.173287, rating = 2 }
        //{ document_id = 2, relevance = 0.173287, rating = -1 }
    }

}
////////////////////////////
void TestAddDocumentsWithDifferenRating() {
    constexpr int nrating_first = (5 + (-12) + 2 + 1) / 4;
    constexpr int nrating_second = (7 + 2 + 7) / 3;
    constexpr int nrating_third = (8 + (-3)) / 2;
    
    {
        SearchServer search_server;
    search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL,
            { 5, -12, 2, 1 });

        const auto& docs = search_server.FindTopDocuments("ухоженный кот"s);
        ASSERT(docs[0].rating == nrating_first);
        ASSERT(docs[1].rating == nrating_second);
        ASSERT(docs[2].rating == nrating_third);
    }

}

/////////////////////////////////////////
void TestFindTopDocumentsByPredicate() {
    constexpr int nfirst_id = 0;
    constexpr int nsecond_id = 2; 
    
    {
        SearchServer search_server;
        search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s,DocumentStatus::ACTUAL, {8, -3});
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
        search_server.AddDocument(3, "ухоженный скворец евгений"s,DocumentStatus::BANNED, {9});

       const auto& docs = search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; });
        ASSERT(docs[0].id == nfirst_id);
        ASSERT(docs[1].id == nsecond_id);
    }
}

////////////////////////////////////////////////////
void TestFindTopDocumentsWithDifferentStatus() {
    constexpr int nfirst_id = 3;
    {
        SearchServer search_server;
        search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3});
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
        search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9});
        
        const auto& docs = search_server.FindTopDocuments("пушистый ухоженный кот"s,  DocumentStatus::BANNED); 
        ASSERT_HINT(docs[0].id == nfirst_id, "Статус документа должен быть BANNED"s); 
    }    
}

////////////////////////////////////////////

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestFindNoDocumentssWithMinusWords);
    RUN_TEST(TestFindMatchingDocuments);
    RUN_TEST(TestChekRelevance);
    RUN_TEST(TestAddDocumentsWithDifferenRating);
    RUN_TEST(TestFindTopDocumentsByPredicate);
    RUN_TEST(TestFindTopDocumentsWithDifferentStatus);
}

// --------- Окончание модульных тестов поисковой системы -----------