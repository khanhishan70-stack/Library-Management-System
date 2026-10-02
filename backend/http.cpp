// http.cpp - building the answers and reading the requests.

#include "http.h"
#include "digital.h"
#include "library.h"

#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>       // for GetCurrentDirectoryA
#else
    #include <unistd.h>        // for getcwd
#endif

// hexDigitValue() now lives in digital.cpp, with the rest of the bit helpers.

// "+" becomes a space and "%20" becomes a real character, because a search
// text like "c++ programming" travels inside the web address.
//
// %2B holds two hex digits: the first one is shifted left four bits to become
// the high nibble and the second one fills the low nibble. 0x2B is the ASCII
// code of '+', which is why searching for "C++" works.
std::string urlDecode(const std::string& text) {
    std::string result;

    for (std::size_t i = 0; i < text.size(); i++) {
        if (text[i] == '+') {
            result += ' ';
        }
        else if (text[i] == '%' && i + 2 < text.size()) {
            unsigned int value = static_cast<unsigned int>(
                (hexDigitValue(text[i + 1]) << 4) | hexDigitValue(text[i + 2]));
            result += static_cast<char>(value);
            i += 2;
        }
        else {
            result += text[i];
        }
    }

    return result;
}

// everything after the '?', for example "/api/search?q=c++" gives "q=c++"
std::string getQueryPart(const std::string& path) {
    std::size_t position = path.find('?');
    if (position == std::string::npos) {
        return "";
    }
    return path.substr(position + 1);
}

// one value out of the query part: "id=B001" with "id" gives "B001"
std::string getQueryValue(const std::string& query, const std::string& name) {
    std::string searchText = name + "=";
    std::size_t position = query.find(searchText);

    if (position == std::string::npos) {
        return "";
    }

    position += searchText.size();
    std::string value;

    while (position < query.size() && query[position] != '&') {
        value += query[position];
        position++;
    }

    return urlDecode(value);
}

// folder of the running program, with the last "\" removed, so the project
// can be started from anywhere
std::string getCurrentFolder() {
    char buffer[1024];
#ifdef _WIN32
    if (GetCurrentDirectoryA(sizeof(buffer), buffer) == 0) {
        return ".";
    }
#else
    if (getcwd(buffer, sizeof(buffer)) == 0) {
        return ".";
    }
#endif
    std::string path(buffer);
    std::size_t lastSlash = path.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        path = path.substr(0, lastSlash);
    }
    return path;
}

// status line + headers + body
std::string buildHttpResponse(const std::string& statusText,
                              const std::string& contentType,
                              const std::string& body) {
    std::ostringstream response;

    response << "HTTP/1.1 " << statusText << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    // so the page is allowed to call the API
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";                 // empty line between headers and body
    response << body;

    return response.str();
}

std::string buildJsonResponse(bool success, const std::string& message) {
    return buildHttpResponse(success ? "200 OK" : "400 Bad Request",
                             "application/json",
                             makeResultJson(success, message));
}

// reads any frontend file (index.html, css/style.css, js/common.js ...)
bool readFrontendFile(const std::string& filePath, std::string& content) {
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    file.close();

    return true;
}

std::string getContentType(const std::string& filePath) {
    std::size_t dotPosition = filePath.find_last_of('.');

    std::string extension = "";
    if (dotPosition != std::string::npos) {
        extension = filePath.substr(dotPosition);
    }

    if (extension == ".html") {
        return "text/html; charset=utf-8";
    }
    if (extension == ".css") {
        return "text/css; charset=utf-8";
    }
    if (extension == ".js") {
        return "application/javascript; charset=utf-8";
    }
    if (extension == ".txt") {
        return "text/plain; charset=utf-8";
    }
    if (extension == ".json") {
        return "application/json";
    }
    if (extension == ".svg") {
        return "image/svg+xml";
    }
    if (extension == ".png") {
        return "image/png";
    }

    return "application/octet-stream";
}

std::string receiveRequest(SocketType clientSocket) {
    std::string request;
    char buffer[4096];

    // first the header, which ends at the first empty line
    std::size_t headerEnd = std::string::npos;

    while (true) {
        int received = static_cast<int>(recv(clientSocket, buffer, sizeof(buffer), 0));
        if (received <= 0) {
            break;
        }

        request.append(buffer, received);

        headerEnd = request.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            break;
        }
    }

    if (headerEnd == std::string::npos) {
        return "";
    }

    // then the body of a POST, whose size Content-Length gives us
    std::size_t lengthPosition = request.find("Content-Length:");
    if (lengthPosition == std::string::npos) {
        return request;
    }

    int contentLength = std::atoi(request.c_str() + lengthPosition + 15);
    std::size_t bodyStart = headerEnd + 4;      // length of "\r\n\r\n"
    std::size_t bodySoFar = request.size() - bodyStart;

    while (bodySoFar < static_cast<std::size_t>(contentLength)) {
        int received = static_cast<int>(recv(clientSocket, buffer, sizeof(buffer), 0));
        if (received <= 0) {
            break;
        }

        request.append(buffer, received);
        bodySoFar = request.size() - bodyStart;
    }

    return request;
}
