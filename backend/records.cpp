// records.cpp - issuing a book, taking it back and reading the history.

#include "library.h"

#include <cctype>
#include <cstdlib>
#include <algorithm>
#include <vector>

// same idea for records: R001 and R003 exist, so the next one is R004
std::string Library::getNextRecordId() const {
    int highestNumber = 0;

    for (std::size_t i = 0; i < records.size(); i++) {
        int number = readNumberFromId(records[i].recordId);

        if (number > highestNumber) {
            highestNumber = number;
        }
    }

    return makeId('R', highestNumber + 1);
}

// Anything that is not a real HH:MM time comes back as "-", so a bad value
// can never be written into records.txt.
std::string Library::cleanTimeText(const std::string& time) {
    std::string text = trimText(time);

    if (text.empty()) {
        return "-";
    }

    if (text.size() != 5 || text[2] != ':') {
        return "-";
    }

    // both halves must be two ASCII digits, the colon in the middle is skipped
    if (!isAllDigits(text, 0, 2) || !isAllDigits(text, 3, 2)) {
        return "-";
    }

    int hours   = digitValue(text[0]) * 10 + digitValue(text[1]);
    int minutes = digitValue(text[3]) * 10 + digitValue(text[4]);

    // Both halves are packed as BCD into a single word, hours in the high byte
    // and minutes in the low byte, which is how a microcontroller holds a time.
    // Reading the two halves back out of that word does the range check.
    unsigned int packed = packTime(hours, minutes);

    if (hoursFromTime(packed) > 23 || minutesFromTime(packed) > 59) {
        return "-";
    }

    return text;
}

OperationResult Library::issueBook(const std::string& bookId,
                                   const std::string& memberId,
                                   const std::string& issueDate,
                                   const std::string& issueTime) {
    OperationResult result;
    result.ok = false;

    std::string book   = trimText(bookId);
    std::string member = trimText(memberId);

    if (book.empty()) {
        result.message = "Book ID cannot be empty.";
        return result;
    }
    if (member.empty()) {
        result.message = "Member ID cannot be empty.";
        return result;
    }

    int bookIndex = findBookIndex(book);
    if (bookIndex < 0) {
        result.message = "Book ID not found. Please check the Book ID.";
        return result;
    }

    int memberIndex = findMemberIndex(member);
    if (memberIndex < 0) {
        result.message = "Member ID not found. Please check the Member ID.";
        return result;
    }

    // This is the stock check. If the library owns 3 copies and 3 are already
    // out, bit 2 (ALL_OUT) is set in the status byte and nothing can be issued.
    if (flagIsSet(books[bookIndex].flags, BOOK_ALL_OUT)) {
        result.message = "No copy of \"" + books[bookIndex].name
                       + "\" is available. All " + std::to_string(books[bookIndex].totalCopies)
                       + (books[bookIndex].totalCopies == 1 ? " copy is" : " copies are")
                       + " already issued.";
        return result;
    }

    // one student cannot take the same title twice at the same time
    for (std::size_t i = 0; i < records.size(); i++) {
        if (records[i].bookId == book && records[i].memberId == member
                                     && records[i].status == "Issued") {
            result.message = "This member already has this book. Please return it first.";
            return result;
        }
    }

    // one copy leaves the shelf, so the issued count goes up by one and the
    // status bits are worked out again
    books[bookIndex].issuedCopies++;
    books[bookIndex].refreshFlags();

    IssueRecord record;
    record.recordId   = getNextRecordId();
    record.bookId     = book;
    record.memberId   = member;
    record.issueDate  = trimText(issueDate).empty() ? getTodayDate() : trimText(issueDate);
    record.issueTime  = cleanTimeText(issueTime);
    record.returnDate = "-";
    record.status     = "Issued";

    records.push_back(record);

    saveToFiles();

    result.ok = true;
    result.message = "Book \"" + books[bookIndex].name + "\" issued to " +
                     members[memberIndex].name + " (" + record.recordId + "). "
                     + std::to_string(books[bookIndex].availableCopies())
                     + (books[bookIndex].availableCopies() == 1
                        ? " copy left." : " copies left.");
    return result;
}

OperationResult Library::returnBook(const std::string& bookId,
                                    const std::string& returnDate) {
    OperationResult result;
    result.ok = false;

    std::string book = trimText(bookId);

    if (book.empty()) {
        result.message = "Book ID cannot be empty.";
        return result;
    }

    int bookIndex = findBookIndex(book);
    if (bookIndex < 0) {
        result.message = "Book ID not found. Please check the Book ID.";
        return result;
    }

    // the mirror image of the issue check: a return is only possible while
    // something is actually out, which is what bit 1 (ON_LOAN) records
    if (!flagIsSet(books[bookIndex].flags, BOOK_ON_LOAN)) {
        result.message = "This book is already available. Nothing to return.";
        return result;
    }

    books[bookIndex].issuedCopies--;

    // the counter must never go below zero, whatever a flag says
    if (books[bookIndex].issuedCopies < 0) {
        books[bookIndex].issuedCopies = 0;
    }

    books[bookIndex].refreshFlags();

    // close the oldest open record for this title
    std::string date = trimText(returnDate).empty() ? getTodayDate() : trimText(returnDate);
    std::string updatedRecord = "-";

    for (std::size_t i = 0; i < records.size(); i++) {
        if (records[i].bookId == book && records[i].status == "Issued") {
            records[i].returnDate = date;
            records[i].status     = "Returned";
            updatedRecord = records[i].recordId;
            break;
        }
    }

    saveToFiles();

    result.ok = true;
    result.message = "Book \"" + books[bookIndex].name + "\" returned successfully (" +
                     updatedRecord + "). "
                     + std::to_string(books[bookIndex].availableCopies())
                     + (books[bookIndex].availableCopies() == 1
                        ? " copy left." : " copies left.");
    return result;
}

// the full history. book and member names are looked up here so the page does
// not have to do that work again.
std::string Library::displayRecords() {
    std::string json = "[";

    for (std::size_t i = 0; i < records.size(); i++) {
        if (i > 0) {
            json += ",";
        }

        std::string returnDate = records[i].returnDate;
        if (returnDate.empty()) {
            returnDate = "-";
        }

        std::string issueTime = records[i].issueTime;
        if (issueTime.empty()) {
            issueTime = "-";
        }

        json += "{\"recordId\":" + toJsonString(records[i].recordId) +
                ",\"bookId\":" + toJsonString(records[i].bookId) +
                ",\"bookName\":" + toJsonString(getBookName(records[i].bookId)) +
                ",\"memberId\":" + toJsonString(records[i].memberId) +
                ",\"memberName\":" + toJsonString(getMemberName(records[i].memberId)) +
                ",\"issueDate\":" + toJsonString(records[i].issueDate) +
                ",\"issueTime\":" + toJsonString(issueTime) +
                ",\"returnDate\":" + toJsonString(returnDate) +
                ",\"status\":" + toJsonString(records[i].status) + "}";
    }

    json += "]";
    return json;
}

// the phone number of a member, or a dash when the member is not in the file
// any more (an old record can point at an id that has since been deleted)
std::string Library::getMemberPhone(const std::string& memberId) const {
    int index = findMemberIndex(memberId);

    if (index < 0) {
        return "-";
    }

    std::string phone = members[index].phone;
    if (phone.empty()) {
        return "-";
    }

    return phone;
}

// Everything the reminder popup needs, and nothing that is already returned.
// Each row carries the member's name, id and phone number plus the number of
// days the book has been out, so the librarian can call the right person.
std::string Library::displayPendingReturns() {
    // gather first, because the rows are sorted by how late they are and the
    // records vector itself must not be touched
    std::vector<std::size_t> pending;

    for (std::size_t i = 0; i < records.size(); i++) {
        if (records[i].status == "Issued") {
            pending.push_back(i);
        }
    }

    // the longest missing book goes first. std::stable_sort keeps the order of
    // two rows that are equally late, so the list does not jump about.
    std::stable_sort(pending.begin(), pending.end(),
                     [this](std::size_t left, std::size_t right) {
                         return daysSinceDate(records[left].issueDate)
                              > daysSinceDate(records[right].issueDate);
                     });

    std::string json = "[";

    for (std::size_t i = 0; i < pending.size(); i++) {
        const IssueRecord& record = records[pending[i]];

        std::string issueTime = record.issueTime;
        if (issueTime.empty()) {
            issueTime = "-";
        }

        json += "{\"recordId\":" + toJsonString(record.recordId) +
                ",\"bookId\":" + toJsonString(record.bookId) +
                ",\"bookName\":" + toJsonString(getBookName(record.bookId)) +
                ",\"memberId\":" + toJsonString(record.memberId) +
                ",\"memberName\":" + toJsonString(getMemberName(record.memberId)) +
                ",\"phone\":" + toJsonString(getMemberPhone(record.memberId)) +
                ",\"issueDate\":" + toJsonString(record.issueDate) +
                ",\"issueTime\":" + toJsonString(issueTime) +
                ",\"daysOut\":" + std::to_string(daysSinceDate(record.issueDate)) + "}";

        if (i + 1 < pending.size()) {
            json += ",";
        }
    }

    json += "]";
    return json;
}
