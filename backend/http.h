// http.h - the small helpers that build and read the web answers.

#ifndef HTTP_H
#define HTTP_H

#include <string>

// Windows needs winsock, Linux and macOS use the normal socket headers.
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET SocketType;
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    typedef int SocketType;
#endif

std::string urlDecode(const std::string& text);
std::string getQueryPart(const std::string& path);
std::string getQueryValue(const std::string& query, const std::string& name);
std::string getCurrentFolder();
void openWebPage(const std::string& url);

std::string buildHttpResponse(const std::string& statusText,
                              const std::string& contentType,
                              const std::string& body);
std::string buildJsonResponse(bool success, const std::string& message);

bool readFrontendFile(const std::string& filePath, std::string& content);
std::string getContentType(const std::string& filePath);

std::string receiveRequest(SocketType clientSocket);

class Library;

// the routes, defined in api.cpp
std::string handleApiRequest(const std::string& method,
                             const std::string& fullPath,
                             const std::string& body,
                             Library& library,
                             const std::string& projectFolder);

#endif  // HTTP_H
