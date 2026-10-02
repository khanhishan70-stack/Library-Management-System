// storage.cpp - reading and writing the three text files.

#include "library.h"

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <filesystem>

// Cuts one line of a text file into parts. "B001 | C++ | 2024" split by '|'
// gives ["B001", "C++", "2024"], because every part is trimmed.
static std::vector<std::string> splitText(const std::string& line, char separator) {
    std::vector<std::string> parts;
    std::string currentPart;
    std::stringstream stream(line);

    while (std::getline(stream, currentPart, separator)) {
        parts.push_back(trimText(currentPart));
    }

    return parts;
}

Library::Library(const std::string& folder)
    : dataFolder(folder) {
    std::error_code error;
    if (!std::filesystem::exists(dataFolder, error)) {
        std::filesystem::create_directories(dataFolder, error);
    }
}

void Library::loadFromFiles() {
    loadBooks();
    loadMembers();
    loadRecords();
}

void Library::loadBooks() {
    books.clear();

    std::ifstream file(dataFolder + "/books.txt");
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trimText(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::vector<std::string> parts = splitText(line, '|');
        if (parts.size() < 6) {
            continue;
        }

        Book book;
        book.id       = parts[0];
        book.name     = parts[1];
        book.author   = parts[2];
        book.category = parts[3];
        book.year     = std::atoi(parts[4].c_str());

        // the file stores a plain 1 or 0, and that is exactly bit 0 of the
        // status byte, so old books.txt files load without any change
        book.flags = (parts[5] == "1")
                     ? static_cast<unsigned char>(setMask(0u, BOOK_ON_SHELF))
                     : 0;

        // the copy counts came later, so older files have 6 fields only and
        // are treated as a single copy
        if (parts.size() >= 8) {
            book.totalCopies  = std::atoi(parts[6].c_str());
            book.issuedCopies = std::atoi(parts[7].c_str());
        } else {
            book.totalCopies  = 1;
            book.issuedCopies = flagIsSet(book.flags, BOOK_ON_SHELF) ? 0 : 1;
        }

        // keep the numbers inside the range that makes sense
        if (book.totalCopies < 1) {
            book.totalCopies = 1;
        }
        if (book.issuedCopies < 0) {
            book.issuedCopies = 0;
        }
        if (book.issuedCopies > book.totalCopies) {
            book.issuedCopies = book.totalCopies;
        }

        // the counters are the truth, so the three status bits follow them
        book.refreshFlags();

        books.push_back(book);
    }

    file.close();
}

void Library::loadMembers() {
    members.clear();

    std::ifstream file(dataFolder + "/members.txt");
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trimText(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::vector<std::string> parts = splitText(line, '|');
        if (parts.size() < 4) {
            continue;
        }

        Member member;
        member.id     = parts[0];
        member.name   = parts[1];
        member.course = parts[2];
        member.phone  = parts[3];

        members.push_back(member);
    }

    file.close();
}

void Library::loadRecords() {
    records.clear();

    std::ifstream file(dataFolder + "/records.txt");
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trimText(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::vector<std::string> parts = splitText(line, '|');
        if (parts.size() < 6) {
            continue;
        }

        IssueRecord record;
        record.recordId   = parts[0];
        record.bookId     = parts[1];
        record.memberId   = parts[2];
        record.issueDate  = parts[3];

        // the time was added later, so older records have 6 fields only
        if (parts.size() >= 7) {
            record.issueTime  = parts[4];
            record.returnDate = parts[5];
            record.status     = parts[6];
        } else {
            record.issueTime  = "-";
            record.returnDate = parts[4];
            record.status     = parts[5];
        }

        records.push_back(record);
    }

    file.close();
}

void Library::saveToFiles() {
    saveBooks();
    saveMembers();
    saveRecords();
}

// std::ios::trunc wipes the old content, so the file only ever holds the
// current data and never leftovers from before.
void Library::saveBooks() const {
    std::ofstream file(dataFolder + "/books.txt", std::ios::trunc);
    if (!file.is_open()) {
        return;
    }

    file << "# books.txt - BookID | BookName | Author | Category | Year"
            " | Available | TotalCopies | IssuedCopies\n";

    for (std::size_t i = 0; i < books.size(); i++) {
        file << books[i].id << " | "
             << books[i].name << " | "
             << books[i].author << " | "
             << books[i].category << " | "
             << books[i].year << " | "
             << (flagIsSet(books[i].flags, BOOK_ON_SHELF) ? 1 : 0) << " | "
             << books[i].totalCopies << " | "
             << books[i].issuedCopies << "\n";
    }

    file.close();
}

void Library::saveMembers() const {
    std::ofstream file(dataFolder + "/members.txt", std::ios::trunc);
    if (!file.is_open()) {
        return;
    }

    file << "# members.txt - MemberID | MemberName | Course | PhoneNumber\n";

    for (std::size_t i = 0; i < members.size(); i++) {
        file << members[i].id << " | "
             << members[i].name << " | "
             << members[i].course << " | "
             << members[i].phone << "\n";
    }

    file.close();
}

void Library::saveRecords() const {
    std::ofstream file(dataFolder + "/records.txt", std::ios::trunc);
    if (!file.is_open()) {
        return;
    }

    file << "# records.txt - RecordID | BookID | MemberID | IssueDate | IssueTime"
            " | ReturnDate | Status\n";

    for (std::size_t i = 0; i < records.size(); i++) {
        std::string time = records[i].issueTime;
        if (time.empty()) {
            time = "-";
        }

        file << records[i].recordId << " | "
             << records[i].bookId << " | "
             << records[i].memberId << " | "
             << records[i].issueDate << " | "
             << time << " | "
             << records[i].returnDate << " | "
             << records[i].status << "\n";
    }

    file.close();
}
