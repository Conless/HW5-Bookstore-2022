#include "BookSystem.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <utility>

#include "Utils/Exception.h"
#include "Utils/TokenScanner.h"

namespace bookstore {

namespace book {

BookInfo::BookInfo()
    : isbn(), name(), author(), keyword_cnt(0), quantity(0), price(0.0) {}

BookInfo::BookInfo(const char *_isbn, const char *_name, const char *_author,
                   const std::vector<BookStr> &_keyword, const double _price)
    : isbn(_isbn), name(_name), author(_author), price(_price) {
    keyword_cnt = _keyword.size();
    for (int i = 0; i < _keyword.size(); i++)
        keyword[i] = _keyword[i];
}

void BookInfo::PrintInfo() const {
    std::cout << isbn.str << '\t' << name.str << '\t' << author.str << '\t';
    for (int i = 0; i < keyword_cnt; i++) {
        if (i)
            std::cout << '|';
        std::cout << keyword[i].str;
    }
    std::cout << '\t' << price << '\t' << quantity << '\n';
}

BookFileSystem::BookFileSystem()
    : BaseFileSystem("book"), isbn_table("isbn"), name_table("name"),
      author_table("author"), key_table("key"), siz(0) {}

std::pair<int, bool> BookFileSystem::insert(const IsbnStr &isbn,
                                            const BookInfo &data) {
    try {
        isbn_table.insert(isbn, siz + 1);
        siz++;
        name_table.insert(data.name, siz);
        author_table.insert(data.author, siz);
        for (int i = 0; i < data.keyword_cnt; i++)
            key_table.insert(data.keyword[i], siz);
        BaseFileSystem::insert(siz, data);
        return std::make_pair(siz, true);
    } catch (const NormalException &x) {
        if (x.what() == ULL_INSERTED)
            return std::make_pair(isbn_table.find(isbn), false);
        else {
            x.error();
            exit(-1);
        }
    }
}

std::pair<int, bool> BookFileSystem::erase(const IsbnStr &isbn) {
    try {
        int pos = isbn_table.erase(isbn);
        BookInfo tmp = BaseFileSystem::find(pos);
        name_table.erase(tmp.name, pos);
        author_table.erase(tmp.author, pos);
        for (int i = 0; i < tmp.keyword_cnt; i++)
            key_table.erase(tmp.keyword[i], siz);
        BaseFileSystem::erase(pos);
        return std::make_pair(pos, true);
    } catch (const NormalException &x) {
        if (x.what() == ULL_ERASE_NOT_FOUND)
            return std::make_pair(0, false);
        else {
            x.error();
            exit(-1);
        }
    }
}

std::pair<int, bool> BookFileSystem::edit(const int pos, BookInfo data) {
    BookInfo tmp = BaseFileSystem::find(pos);
    if (!data.isbn.empty()) {
        try {
            isbn_table.find(data.isbn);
            return std::make_pair(pos, false);
        } catch (const NormalException &x) {
            if (x.what() == ULL_NOT_FOUND) {
                isbn_table.erase(tmp.isbn);
                isbn_table.insert(data.isbn, pos);
                tmp.isbn = data.isbn;
            } else {
                x.error();
                exit(-1);
            }
        }
    }
    if (!data.name.empty()) {
        name_table.erase(tmp.name, pos);
        name_table.insert(data.name, pos);
        tmp.name = data.name;
    }
    if (!data.author.empty()) {
        author_table.erase(tmp.author, pos);
        author_table.insert(data.author, pos);
        tmp.author = data.author;
    }
    if (data.keyword_cnt) {
        for (int i = 0; i < tmp.keyword_cnt; i++)
            key_table.erase(tmp.keyword[i], pos);
        for (int i = 0; i < data.keyword_cnt; i++)
            key_table.insert(data.keyword[i], pos);
        memcpy(tmp.keyword, data.keyword, sizeof(data.keyword));
        tmp.keyword_cnt = data.keyword_cnt;
    }
    if (data.price != -1)
        tmp.price = data.price;
    BaseFileSystem::erase(pos);
    BaseFileSystem::insert(pos, tmp);
    return std::make_pair(pos, true);
}

std::pair<double, bool>
BookFileSystem::import(const int pos, const int quantity, const double cost) {
    if (!pos)
        throw InvalidException("Import a book before select it");
    BookInfo tmp = BaseFileSystem::find(pos);
    tmp.quantity += quantity;
    BaseFileSystem::erase(pos);
    BaseFileSystem::insert(pos, tmp);
    return std::make_pair(cost, true);
}

std::pair<double, bool> BookFileSystem::buy(const IsbnStr &isbn,
                                            const int quantity) {
    try {
        int pos = isbn_table.find(isbn);
        BookInfo tmp = BaseFileSystem::find(pos);
        if (tmp.quantity < quantity)
            return std::make_pair(0.0, false);
        tmp.quantity -= quantity;
        BaseFileSystem::erase(pos);
        BaseFileSystem::insert(pos, tmp);
        return std::make_pair(tmp.price, true);
    } catch (const NormalException &x) {
        if (x.what() == ULL_NOT_FOUND)
            return std::make_pair(0.0, false);
        else {
            x.error();
            exit(-1);
        }
    }
}

BookInfo BookFileSystem::FileSearchByISBN(const IsbnStr &isbn) {
    try {
        int pos = isbn_table.find(isbn);
        BookInfo ret = BaseFileSystem::find(pos);
        ret.pos = pos;
        return ret;
    } catch (const NormalException &x) {
        if (x.what() == ULL_NOT_FOUND)
            return BookInfo();
        else {
            x.error();
            exit(-1);
        }
    }
}

std::vector<BookInfo> BookFileSystem::FileSearchByName(const BookStr &name) {
    std::vector<int> pos = name_table.find(name);
    std::vector<BookInfo> ret;
    ret.clear();
    for (auto p : pos)
        ret.push_back(BaseFileSystem::find(p));
    std::sort(ret.begin(), ret.end());
    return ret;
}

std::vector<BookInfo>
BookFileSystem::FileSearchByAuthor(const BookStr &author) {
    std::vector<int> pos = author_table.find(author);
    std::vector<BookInfo> ret;
    ret.clear();
    for (auto p : pos)
        ret.push_back(BaseFileSystem::find(p));
    std::sort(ret.begin(), ret.end());
    return ret;
}

std::vector<BookInfo>
BookFileSystem::FileSearchByKeyword(const BookStr &keyword) {
    std::vector<int> pos = key_table.find(keyword);
    std::vector<BookInfo> ret;
    ret.clear();
    for (auto p : pos)
        ret.push_back(BaseFileSystem::find(p));
    std::sort(ret.begin(), ret.end());
    return ret;
}

void BookFileSystem::output() {
    std::cout << "Book status:\n";
    for (int i = 1; i <= siz; i++) {
        BookInfo tmp = BaseFileSystem::find(i);
        tmp.PrintInfo();
    }
    std::cout << '\n';
}

BookSystem::BookSystem() : book_table() {
    std::ifstream fin("./data/book.log");
    if (fin.good()) {
        int len;
        fin >> book_table.siz >> len;
        total_earn.resize(len);
        total_cost.resize(len);
        for (int i = 0; i < len; i++)
            fin >> total_earn[i] >> total_cost[i];
    } else {
        total_earn.push_back(0);
        total_cost.push_back(0);
    }
}
BookSystem::~BookSystem() {
    std::ofstream fout("./data/book.log", std::ios::out | std::ios::trunc);
    int len = total_earn.size();
    fout << book_table.siz << ' ' << len << '\n';
    for (int i = 0; i < len; i++)
        fout << total_earn[i] << ' ' << total_cost[i] << '\n';
}

int BookSystem::SelectBook(const char *isbn) {
    BookInfo tmp = book_table.FileSearchByISBN(IsbnStr(isbn));
    if (tmp.empty()) {
        tmp.isbn = isbn;
        return book_table.insert(IsbnStr(isbn), tmp).first;
    } else
        return tmp.pos;
}
void BookSystem::ModifyBook(const int book_pos, const char *_isbn,
                            const char *_name, const char *_author,
                            const std::vector<BookStr> &_key,
                            const double _price) {
    if (!book_pos)
        throw InvalidException("Modify a book before selecting it");
    if (!book_table
             .edit(book_pos, BookInfo(_isbn, _name, _author, _key, _price))
             .second)
        throw InvalidException("ISBN exists.");
}

void BookSystem::SearchByISBN(const char *isbn) {
    BookInfo tmp = book_table.FileSearchByISBN(IsbnStr(isbn));
    if (tmp.empty()) {
        std::cout << '\n';
        return;
    }
    tmp.PrintInfo();
}

void BookSystem::SearchByName(const char *name) {
    std::vector<BookInfo> tmp = book_table.FileSearchByName(BookStr(name));
    if (tmp.empty()) {
        std::cout << '\n';
        return;
    }
    for (const auto &tmp_book : tmp)
        tmp_book.PrintInfo();
    return;
}

void BookSystem::SearchByAuthor(const char *author) {
    std::vector<BookInfo> tmp = book_table.FileSearchByAuthor(BookStr(author));
    if (tmp.empty()) {
        std::cout << '\n';
        return;
    }
    for (const auto &tmp_book : tmp)
        tmp_book.PrintInfo();
    return;
}

void BookSystem::SearchByKeyword(const char *keyword) {
    std::vector<BookInfo> tmp =
        book_table.FileSearchByKeyword(BookStr(keyword));
    if (tmp.empty()) {
        std::cout << '\n';
        return;
    }
    for (const auto &tmp_book : tmp)
        tmp_book.PrintInfo();
    return;
}

void BookSystem::SearchAll() {
    std::set<BookInfo> tmp = book_table.search();
    if (tmp.empty()) {
        std::cout << '\n';
        return;
    }
    for (const BookInfo &tmp_book : tmp)
        tmp_book.PrintInfo();
    return;
}

void BookSystem::output() { book_table.output(); }

void BookSystem::AddBook(const char *isbn, const BookInfo &data) {
    if (!book_table.insert(IsbnStr(isbn), data).second)
        throw InvalidException("Insert a book that already exists");
    return;
}

void BookSystem::BuyBook(const char *isbn, const int quantity) {
    auto res = book_table.buy(IsbnStr(isbn), quantity);
    if (!res.second)
        throw InvalidException("Not found the book or no enough book!");
    res.first *= quantity;
    std::cout << res.first << '\n';
    total_earn.push_back(total_earn.back() + res.first);
    total_cost.push_back(total_cost.back());
}

void BookSystem::ImportBook(const int book_pos, const int quantity,
                            const double cost) {

    if (!book_table.import(book_pos, quantity, cost).second)
        throw InvalidException("Not found the book to import");
    total_earn.push_back(total_earn.back());
    total_cost.push_back(total_cost.back() + cost);
}

void BookSystem::ShowFinance(const int rev) {
    if (rev == -1)
        std::cout << "+ " << total_earn.back() << " - " << total_cost.back()
                  << '\n';
    else if (rev == 0)
        std::cout << '\n';
    else {
        int cnt = total_earn.size() - 1;
        if (cnt - rev < 0)
            throw InvalidException("Show finance out of range");
        double earn_dif = total_earn[cnt] - total_earn[cnt - rev];
        double cost_dif = total_cost[cnt] - total_cost[cnt - rev];
        std::cout << "+ " << earn_dif << " - " << cost_dif << '\n';
    }
}

} // namespace book

} // namespace bookstore
