// books.cpp - everything about the book titles and their copies.

#include "library.h"

#include <cctype>
#include <cstdlib>

// Rebuilds the status byte from the two copy counters. The three bits are
// switched off together with one mask, then the ones that are true are set
// again, which is exactly how a program updates its flag register.
void Book::refreshFlags() {
    flags = static_cast<unsigned char>(
        clearMask(flags, BOOK_ON_SHELF | BOOK_ON_LOAN | BOOK_ALL_OUT));

    if (availableCopies() > 0) {
        flags = static_cast<unsigned char>(setMask(flags, BOOK_ON_SHELF));
    }
    if (issuedCopies > 0) {
        flags = static_cast<unsigned char>(setMask(flags, BOOK_ON_LOAN));
    }
    if (availableCopies() <= 0) {
        flags = static_cast<unsigned char>(setMask(flags, BOOK_ALL_OUT));
    }
}

// gives back the position in the vector, or -1 when the id is not there
int Library::findBookIndex(const std::string& bookId) const {
    for (std::size_t i = 0; i < books.size(); i++) {
        if (books[i].id == bookId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::string Library::getBookName(const std::string& bookId) const {
    int index = findBookIndex(bookId);
    if (index < 0) {
        return "";
    }
    return books[index].name;
}

// The next free book id. Only ids starting with 'B' are counted, so a book
// that was typed into the file by hand with another id (K001 say) is ignored.
// If the number we pick is somehow taken, we keep going until it is free.
std::string Library::getNextBookId() const {
    int highestNumber = 0;

    for (std::size_t i = 0; i < books.size(); i++) {
        if (books[i].id.empty() || books[i].id[0] != 'B') {
            continue;
        }

        int number = readNumberFromId(books[i].id);

        if (number > highestNumber) {
            highestNumber = number;
        }
    }

    int number = highestNumber + 1;
    std::string newId = makeId('B', number);

    while (findBookIndex(newId) >= 0) {
        number++;
        newId = makeId('B', number);
    }

    return newId;
}

/* The librarian only types the name, the year and the copy count. The id and
   the category are decided here, so there is no way to type them wrong. */
OperationResult Library::addBook(const Book& book) {
    OperationResult result;
    result.ok = false;

    if (trimText(book.name).empty()) {
        result.message = "Book name cannot be empty.";
        return result;
    }
    if (book.year < 1000 || book.year > 2100) {
        result.message = "Please enter a valid year (for example 2024).";
        return result;
    }

    int copies = book.totalCopies;

    if (copies < 1) {
        copies = 1;
    }
    if (copies > 999) {
        result.message = "Copies cannot be more than 999.";
        return result;
    }

    Book newBook;
    newBook.id = trimText(book.id);

    if (newBook.id.empty()) {
        newBook.id = getNextBookId();
    }

    newBook.name = trimText(book.name);
    newBook.year = book.year;

    newBook.category = trimText(book.category);
    if (newBook.category.empty()) {
        newBook.category = "General";
    }

    newBook.author = trimText(book.author);
    if (newBook.author.empty()) {
        newBook.author = "-";   // kept in the file only, never shown
    }

    // A duplicate id is still not allowed (in case an id was sent by mistake).
    if (findBookIndex(newBook.id) >= 0) {
        result.message = "A book with this Book ID already exists.";
        return result;
    }

    // a new book starts with every copy on the shelf
    newBook.totalCopies  = copies;
    newBook.issuedCopies = 0;
    newBook.flags        = 0;
    newBook.refreshFlags();

    books.push_back(newBook);
    saveToFiles();

    result.ok = true;
    result.message = "Book \"" + newBook.name + "\" added successfully with Book ID "
                     + newBook.id + " (" + std::to_string(copies)
                     + (copies == 1 ? " copy)" : " copies)");
    return result;
}

OperationResult Library::deleteBook(const std::string& bookId) {
    OperationResult result;
    result.ok = false;

    std::string id = trimText(bookId);
    if (id.empty()) {
        result.message = "Book ID cannot be empty.";
        return result;
    }

    int index = findBookIndex(id);
    if (index < 0) {
        result.message = "Book ID not found.";
        return result;
    }

    // blocked while a copy is out, otherwise the history would point at a
    // book that is no longer in the list. bit 1 of the status byte tells us.
    if (flagIsSet(books[index].flags, BOOK_ON_LOAN)) {
        result.message = "This book still has " + std::to_string(books[index].issuedCopies)
                       + (books[index].issuedCopies == 1
                          ? " copy" : " copies")
                       + " issued. Please return "
                       + (books[index].issuedCopies == 1 ? "it" : "them") + " first.";
        return result;
    }

    std::string deletedName = books[index].name;

    books.erase(books.begin() + index);
    saveToFiles();

    result.ok = true;
    result.message = "Book \"" + deletedName + "\" deleted successfully.";
    return result;
}

// The word shown in the Status column. Nothing out -> Available, some out ->
// Partly Issued, all out -> Issued. Each answer is picked by testing one bit,
// so the three cases travel in a single byte instead of three variables.
std::string Library::bookStatusText(const Book& book) {
    if (flagIsSet(book.flags, BOOK_ALL_OUT)) {
        return "Issued";
    }

    if (flagIsSet(book.flags, BOOK_ON_LOAN)) {
        return "Partly Issued";
    }

    return "Available";
}

// one book as JSON. the author stays in the data file but is not sent to the
// page, because it is not shown any more.
std::string Library::bookToJson(const Book& book) {
    return "{\"id\":" + toJsonString(book.id) +
           ",\"name\":" + toJsonString(book.name) +
           ",\"category\":" + toJsonString(book.category) +
           ",\"year\":" + std::to_string(book.year) +
           ",\"totalCopies\":" + std::to_string(book.totalCopies) +
           ",\"issuedCopies\":" + std::to_string(book.issuedCopies) +
           ",\"availableCopies\":" + std::to_string(book.availableCopies()) +
           ",\"status\":" + toJsonString(bookStatusText(book)) + "}";
}

// lower case copy of a text, used so the search is not case sensitive
static std::string toLowerText(const std::string& text) {
    std::string small = text;

    for (std::size_t i = 0; i < small.size(); i++) {
        small[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(small[i])));
    }

    return small;
}

std::string Library::searchBooks(const std::string& query) {
    std::string smallText = toLowerText(trimText(query));
    std::string json = "[";

    for (std::size_t i = 0; i < books.size(); i++) {
        bool found = (toLowerText(books[i].id).find(smallText) != std::string::npos) ||
                     (toLowerText(books[i].name).find(smallText) != std::string::npos) ||
                     (toLowerText(books[i].category).find(smallText) != std::string::npos);

        if (found) {
            if (json != "[") {
                json += ",";
            }
            json += bookToJson(books[i]);
        }
    }

    json += "]";
    return json;
}

std::string Library::displayBooks() {
    std::string json = "[";

    for (std::size_t i = 0; i < books.size(); i++) {
        if (i > 0) {
            json += ",";
        }

        json += bookToJson(books[i]);
    }

    json += "]";
    return json;
}
