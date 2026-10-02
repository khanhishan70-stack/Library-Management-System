// server.cpp - starts the web server and waits for requests.

#include "http.h"
#include "digital.h"
#include "library.h"

#include <iostream>
#include <cstring>

#ifdef _WIN32
    #pragma comment(lib, "Ws2_32.lib")
    #define CLOSE_SOCKET closesocket
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
#else
    #include <unistd.h>
    #define CLOSE_SOCKET close
    #define INVALID_SOCKET_VALUE -1
#endif

const int DEFAULT_PORT = 8080;

int main(int argc, char* argv[]) {
    // the folder holding "frontend" and "backend". With no argument we take the
    // parent of the folder the .exe sits in, which is the backend folder.
    std::string projectFolder = "..";

    if (argc >= 2) {
        projectFolder = argv[1];
    } else {
        std::string programFolder = getCurrentFolder();
        std::size_t lastSlash = programFolder.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            projectFolder = programFolder.substr(0, lastSlash);
        }
    }

    int port = DEFAULT_PORT;
    if (argc >= 3) {
        port = std::atoi(argv[2]);
    }

    std::string dataFolder = projectFolder + "/backend/data";
    std::string frontendFolder = projectFolder + "/frontend";

    std::cout << "===============================================\n";
    std::cout << "   LIBRARY MANAGEMENT SYSTEM - C++ BACKEND\n";
    std::cout << "===============================================\n";
    std::cout << "Project folder : " << projectFolder << "\n";
    std::cout << "Data folder    : " << dataFolder << "\n";
    std::cout << "Frontend folder: " << frontendFolder << "\n";

    Library library(dataFolder);
    library.loadFromFiles();

    std::cout << "Data loaded successfully from the text files.\n";

    // the bit level routines are checked against known answers on every start
    runDigitalSelfTest();

    // say it straight away if the frontend folder is wrong
    std::string checkFile;
    if (!readFrontendFile(frontendFolder + "/index.html", checkFile)) {
        std::cout << "WARNING: index.html was not found in the frontend folder.\n";
        std::cout << "         Start the program from the backend folder, or give\n";
        std::cout << "         the project folder like this:\n";
        std::cout << "         library_server.exe C:/MyProject\n";
    }

    // Windows needs this before any socket is used
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "ERROR: Could not start the socket library.\n";
        return 1;
    }
#endif

    SocketType serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET_VALUE) {
        std::cout << "ERROR: Could not create the socket.\n";
        return 1;
    }

    // lets us restart quickly without waiting for the old socket to clear
    int reuse = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in addressInfo;
    std::memset(&addressInfo, 0, sizeof(addressInfo));
    addressInfo.sin_family = AF_INET;
    addressInfo.sin_addr.s_addr = INADDR_ANY;      // every computer can connect
    addressInfo.sin_port = htons(static_cast<unsigned short>(port));

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&addressInfo),
             sizeof(addressInfo)) != 0) {
        std::cout << "ERROR: Port " << port << " is already in use.\n";
        CLOSE_SOCKET(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 20) != 0) {
        std::cout << "ERROR: Could not start listening.\n";
        CLOSE_SOCKET(serverSocket);
        return 1;
    }

    std::cout << "\nServer started successfully.\n";
    std::cout << "Open this address in Chrome / Edge / Firefox:\n";
    std::cout << "        http://localhost:" << port << "\n";
    std::cout << "\nPress Ctrl + C in this black window to stop the server.\n";
    std::cout << "-----------------------------------------------\n";

    while (true) {
        std::cout << "Waiting for a request..." << std::endl;

        sockaddr_in clientInfo;
        std::memset(&clientInfo, 0, sizeof(clientInfo));

#ifdef _WIN32
        int clientSize = sizeof(clientInfo);
#else
        socklen_t clientSize = sizeof(clientInfo);
#endif

        SocketType clientSocket = accept(serverSocket,
                                         reinterpret_cast<sockaddr*>(&clientInfo),
                                         &clientSize);

        if (clientSocket == INVALID_SOCKET_VALUE) {
            continue;
        }

        std::string request = receiveRequest(clientSocket);

        if (!request.empty()) {
            // the first line looks like:  GET /api/books HTTP/1.1
            std::size_t firstSpace = request.find(' ');
            std::size_t secondSpace = request.find(' ', firstSpace + 1);

            std::string method = "GET";
            std::string path   = "/";

            if (firstSpace != std::string::npos && secondSpace != std::string::npos) {
                method = request.substr(0, firstSpace);
                path   = request.substr(firstSpace + 1, secondSpace - firstSpace - 1);
            }

            std::string body = "";
            std::size_t headerEnd = request.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                body = request.substr(headerEnd + 4);
            }

            std::cout << "Request received: " << method << " " << path << std::endl;

            std::string answer;
            try {
                answer = handleApiRequest(method, path, body, library, projectFolder);
            } catch (...) {
                answer = buildJsonResponse(false, "Server error while handling the request.");
            }

            send(clientSocket, answer.c_str(), static_cast<int>(answer.size()), 0);
        }

        // the answer said "Connection: close", so this socket is finished
        CLOSE_SOCKET(clientSocket);
    }

    // the loop above never ends, so this is never reached
    CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
