// library.h - the structures and the function names the compiler needs to know.
// the working code of those functions is in the matching .cpp files.

#ifndef LIBRARY_H          // ignore this file if it is included twice
#define LIBRARY_H

#include "digital.h"

#include <string>
#include <vector>

/* A library owns copies of a title, not just the title itself. 3 copies of
   C++ Programming with 2 given out still leaves 1 on the shelf, and once all 3
   are out the title vanishes from the Available Books list.

   The librarian types the name, the year and the copy count. The id and the
   category are filled in by the program, and the author only stays in the data
   file because it is not shown on the screen any more.

   The status is one byte of bits (see BookFlag in digital.h) instead of a pile
   of yes/no variables, so the three questions about a book are asked with a
   single bit test each. */
struct Book {
    std::string id;         // automatic. Example: B001
    std::string name;       // typed by the librarian
    std::string author;     // kept in the file only
    std::string category;   // automatic, "General"
    int year;               // Example: 2024
    int totalCopies;        // how many copies the library owns
    int issuedCopies;       // how many are out right now
    unsigned char flags;    // bit 0 on shelf, bit 1 on loan, bit 2 all out

    int availableCopies() const {
        int left = totalCopies - issuedCopies;
        return left < 0 ? 0 : left;
    }

    // works out the three bits again from the copy counters
    void refreshFlags();
};

struct Member {
    std::string id;         // Example: M001
    std::string name;
    std::string course;
    std::string phone;
};

struct IssueRecord {
    std::string recordId;   // Example: R001
    std::string bookId;
    std::string memberId;
    std::string issueDate;  // DD-MM-YYYY
    std::string issueTime;  // HH:MM, or "-" when it was not noted
    std::string returnDate; // DD-MM-YYYY, or "-" while still out
    std::string status;     // "Issued" or "Returned"
};

// how an add / issue / return went, and the sentence to show on the screen
struct OperationResult {
    bool ok;
    std::string message;
};

// small text and JSON helpers, working code in helpers.cpp
std::string toJsonString(const std::string& text);
std::string getJsonField(const std::string& json, const std::string& fieldName);
std::string makeResultJson(bool success, const std::string& message);
std::string getTodayDate();
std::string trimText(const std::string& text);

// id helpers: "B001" -> 1, and ('B', 4) -> "B004"
int readNumberFromId(const std::string& id);
std::string makeId(char letter, int number);
int daysSinceDate(const std::string& date);   // -1 when the date is not usable

// holds the data and all the library work
class Library {
public:
    Library(const std::string& dataFolder);

    void loadFromFiles();   // reads books.txt, members.txt, records.txt
    void saveToFiles();     // writes the same three files back

    // books
    OperationResult addBook(const Book& book);
    OperationResult deleteBook(const std::string& bookId);
    std::string searchBooks(const std::string& query);
    std::string displayBooks();
    int findBookIndex(const std::string& bookId) const;   // -1 if not found

    // members
    OperationResult addMember(const Member& member);
    OperationResult editMember(const std::string& memberId,
                               const std::string& newName,
                               const std::string& newPhone);
    OperationResult deleteMember(const std::string& memberId);
    std::string displayMembers();
    int findMemberIndex(const std::string& memberId) const;   // -1 if not found

    // issue and return
    OperationResult issueBook(const std::string& bookId,
                              const std::string& memberId,
                              const std::string& issueDate,
                              const std::string& issueTime);
    OperationResult returnBook(const std::string& bookId,
                               const std::string& returnDate);
    std::string displayRecords();

    // Only the books that are still out, with the member's phone number and
    // the number of days late. This is what the reminder popup shows.
    std::string displayPendingReturns();

    std::string getDashboardStats();   // the four dashboard numbers as JSON

private:
    std::vector<Book> books;
    std::vector<Member> members;
    std::vector<IssueRecord> records;

    std::string dataFolder;   // where the three data files live

    // next free id of each series: B004, M008, R007 ...
    std::string getNextBookId() const;
    std::string getNextMemberId() const;
    std::string getNextRecordId() const;

    std::string getBookName(const std::string& bookId) const;
    std::string getMemberName(const std::string& memberId) const;
    std::string getMemberPhone(const std::string& memberId) const;

    static std::string bookStatusText(const Book& book);
    static std::string bookToJson(const Book& book);

    // only a real HH:MM time survives, so a wrong value can never reach the file
    static std::string cleanTimeText(const std::string& time);

    void loadBooks();
    void loadMembers();
    void loadRecords();
    void saveBooks() const;
    void saveMembers() const;
    void saveRecords() const;
};

#endif  // LIBRARY_H