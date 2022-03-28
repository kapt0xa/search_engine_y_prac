#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;


const bool do_debug = 0;
inline void debug(string message)
{
    if (do_debug)
    {
        cout << message << endl;
    }
}
template <typename T>
void debug(string message, T val)
{
    if (do_debug)
    {
        cout << message << ' ' << val << endl;
    }
}

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
    bool word_began = false;
    unsigned the_beginning = 0;
    for (int i = 0; i < text.size(); ++i) {
        if (text[i] == ' ')
        {
            if (word_began)
            {
                word_began = false;
                words.push_back(text.substr(the_beginning, i - the_beginning));
            }
        }
        else
        {
            if (!word_began)
            {
                word_began = true;
                the_beginning = i;
            }
        }
    }
    if (word_began)
    {
        words.push_back(text.substr(the_beginning, text.size() - the_beginning));
    }

    return words;
}


struct DOC_ID
{
    DOC_ID() { doc_id = 0; }
    DOC_ID(int Val)
    {
        doc_id = Val;
    }
    int doc_id;
    operator int() const
    {
        return doc_id;
    }
};
struct RELEVANCE
{
    RELEVANCE() { relevance = 0; }
    RELEVANCE(double Val)
    {
        relevance = Val;
    }
    double relevance;
    operator double() const
    {
        return relevance;
    }
};
struct Document {
    Document()
    {
        relevance = 0;
    }
    Document(const pair<int, int>& Pair)
    {
        id = Pair.first;
        relevance = Pair.second;
    }
    DOC_ID id;
    RELEVANCE relevance;
    operator pair<int, int>()
    {
        return { id.doc_id, relevance.relevance };
    }
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(DOC_ID document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        for (const string& word : words)
        {
            words_to_documents[word].insert(document_id);
            documents_to_words[document_id].insert(word);
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return rhs.relevance < lhs.relevance;
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }
private:

    struct Query
    {
    public:
        Query(const string& raw_query, const SearchServer& server)
        {
            for (const auto& word : SplitIntoWords(raw_query))
            {
                if (word[0] == '-')
                {
                    ban.insert(word.substr(1));
                }
                else
                {
                    if (!server.IsStopWord(word))
                        search.insert(word);
                }
            }
        }
        set<string> search;
        set<string> ban;
    };

    map<string, set<DOC_ID>> words_to_documents;
    map<DOC_ID, set<string>> documents_to_words;

    set<string> stop_words_;

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

    inline const Query ParseQuery(string raw_query) const
    {
        return Query(raw_query, *this);
    }

    set<string> ParseQuery_old(const string& text) const {
        set<string> query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            query_words.insert(word);
        }
        return query_words;
    }

    vector<Document> FindAllDocuments(const Query& query) const {
        const auto words_to_doc_end = words_to_documents.end();
        map<DOC_ID, RELEVANCE> matched_documents;
        if (query.search.empty())
        {
            return vector<Document>();
        }
        for (const auto& word : query.search)
        {
            auto found = words_to_documents.find(word);
            if (found != words_to_doc_end)
            {
                for (const auto& doc_id : found->second)
                {
                    ++matched_documents[doc_id].relevance;
                }
            }
        }
        for (const auto& banword : query.ban)
        {
            auto found = words_to_documents.find(banword);
            if (found != words_to_doc_end)
                for (const auto& ban_id : words_to_documents.at(banword))
                {
                    matched_documents.erase(ban_id);
                }
        }
        vector<Document> all_found_documents(matched_documents.begin(), matched_documents.end());
        return all_found_documents;
    }
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
        cout << "{ document_id = "s << document_id.doc_id << ", "
            << "relevance = "s << relevance.relevance << " }"s << endl;
    }
}