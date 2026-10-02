// dashboard.cpp - the four numbers shown on the first page.

#include "library.h"

// counted fresh on every call, so the dashboard can never show a stale number
std::string Library::getDashboardStats() {
    // the book numbers count copies, not titles: two titles with 3 and 1 copies
    // make 4 books
    int totalBooks = 0;
    int availableBooks = 0;
    int issuedBooks = 0;

    for (std::size_t i = 0; i < books.size(); i++) {
        totalBooks += books[i].totalCopies;
        availableBooks += books[i].availableCopies();
        issuedBooks += books[i].issuedCopies;
    }

    int totalMembers = static_cast<int>(members.size());

    std::string json = "{\"totalBooks\":" + std::to_string(totalBooks) +
                       ",\"availableBooks\":" + std::to_string(availableBooks) +
                       ",\"issuedBooks\":" + std::to_string(issuedBooks) +
                       ",\"totalMembers\":" + std::to_string(totalMembers) +
                       "}";

    return json;
}
