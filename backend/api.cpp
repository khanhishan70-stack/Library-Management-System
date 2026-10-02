// api.cpp - the routes. This is the bridge between the JavaScript of the
// pages and the library logic.

#include "http.h"
#include "library.h"

#include <cstdlib>

// every route is one small if
std::string handleApiRequest(const std::string& method,
                             const std::string& fullPath,
                             const std::string& body,
                             Library& library,
                             const std::string& projectFolder) {
    // the path can carry a question mark, like /api/search?q=tanenbaum, and we
    // need both halves: the real path and the values after the '?'
    std::size_t questionMark = fullPath.find('?');
    std::string path = (questionMark == std::string::npos)
                     ? fullPath
                     : fullPath.substr(0, questionMark);
    std::string query = getQueryPart(fullPath);

    if (path == "/api/dashboard" && method == "GET") {
        return buildHttpResponse("200 OK", "application/json", library.getDashboardStats());
    }

    if (path == "/api/books" && method == "GET") {
        return buildHttpResponse("200 OK", "application/json", library.displayBooks());
    }

    if (path == "/api/books" && method == "POST") {
        Book book;
        book.id       = trimText(getJsonField(body, "id"));
        book.name     = trimText(getJsonField(body, "name"));
        book.author   = trimText(getJsonField(body, "author"));
        book.category = trimText(getJsonField(body, "category"));
        book.year     = std::atoi(trimText(getJsonField(body, "year")).c_str());

        // how many copies the library owns, the frontend sends it as "copies"
        book.totalCopies = std::atoi(trimText(getJsonField(body, "copies")).c_str());

        OperationResult result = library.addBook(book);
        return buildJsonResponse(result.ok, result.message);
    }

    if (path == "/api/books" && method == "DELETE") {
        std::string bookId = getQueryValue(query, "id");

        OperationResult result = library.deleteBook(bookId);
        return buildJsonResponse(result.ok, result.message);
    }

    if (path == "/api/search" && method == "GET") {
        std::string text = getQueryValue(query, "q");
        return buildHttpResponse("200 OK", "application/json", library.searchBooks(text));
    }

    if (path == "/api/members" && method == "GET") {
        return buildHttpResponse("200 OK", "application/json", library.displayMembers());
    }

    if (path == "/api/members" && method == "POST") {
        Member member;
        member.id     = trimText(getJsonField(body, "id"));
        member.name   = trimText(getJsonField(body, "name"));
        member.course = trimText(getJsonField(body, "course"));
        member.phone  = trimText(getJsonField(body, "phone"));

        OperationResult result = library.addMember(member);
        return buildJsonResponse(result.ok, result.message);
    }

    if (path == "/api/members/update" && method == "POST") {
        std::string memberId = trimText(getJsonField(body, "id"));
        std::string name     = trimText(getJsonField(body, "name"));
        std::string phone    = trimText(getJsonField(body, "phone"));

        OperationResult result = library.editMember(memberId, name, phone);
        return buildJsonResponse(result.ok, result.message);
    }

    if (path == "/api/members" && method == "DELETE") {
        std::string memberId = getQueryValue(query, "id");

        OperationResult result = library.deleteMember(memberId);
        return buildJsonResponse(result.ok, result.message);
    }

    if (path == "/api/records" && method == "GET") {
        return buildHttpResponse("200 OK", "application/json", library.displayRecords());
    }

    // the still-issued books, for the reminder popup
    if (path == "/api/pending" && method == "GET") {
        return buildHttpResponse("200 OK", "application/json", library.displayPendingReturns());
    }

    if (path == "/api/issue" && method == "POST") {
        std::string bookId   = trimText(getJsonField(body, "bookId"));
        std::string memberId = trimText(getJsonField(body, "memberId"));
        std::string date     = trimText(getJsonField(body, "issueDate"));
        std::string time     = trimText(getJsonField(body, "issueTime"));

        OperationResult result = library.issueBook(bookId, memberId, date, time);
        return buildJsonResponse(result.ok, result.message);
    }

    if (path == "/api/return" && method == "POST") {
        std::string bookId = trimText(getJsonField(body, "bookId"));
        std::string date   = trimText(getJsonField(body, "returnDate"));

        OperationResult result = library.returnBook(bookId, date);
        return buildJsonResponse(result.ok, result.message);
    }

    if (path.compare(0, 5, "/api/") == 0) {
        return buildJsonResponse(false, "Unknown request: " + fullPath);
    }

    // anything that is not /api/... is a file inside the frontend folder
    std::string fileName = path;
    if (fileName == "/" || fileName.empty()) {
        fileName = "/index.html";
    }

    // keeps the file name inside the frontend folder
    if (fileName.find("..") != std::string::npos) {
        return buildHttpResponse("403 Forbidden", "text/plain", "Access denied.");
    }

    std::string content;
    std::string fileToOpen = projectFolder + "/frontend" + fileName;

    if (!readFrontendFile(fileToOpen, content)) {
        return buildHttpResponse("404 Not Found", "text/plain",
                                 "File not found: " + fileName +
                                 "\n\nFolder used: " + projectFolder + "/frontend");
    }

    return buildHttpResponse("200 OK", getContentType(fileName), content);
}
