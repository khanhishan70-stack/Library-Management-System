// helpers.cpp - the small text, JSON and date helpers used everywhere.

#include "library.h"

#include <sstream>
#include <ctime>
#include <cctype>
#include <cstdlib>
#include <iomanip>

// Removes extra spaces from both sides of a text.
std::string trimText(const std::string& text) {
    std::size_t first = 0;
    while (first < text.size() && std::isspace(static_cast<unsigned char>(text[first]))) {
        first++;
    }

    std::size_t last = text.size();
    while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1]))) {
        last--;
    }

    return text.substr(first, last - first);
}

// Puts quotes around a text and escapes the characters that would otherwise
// break the JSON, so a name like C++ Programming becomes "C++ Programming".
std::string toJsonString(const std::string& text) {
    std::string result = "\"";

    for (std::size_t i = 0; i < text.size(); i++) {
        char current = text[i];

        if (current == '"' || current == '\\') {
            result += '\\';
            result += current;
        } else if (current == '\n') {
            result += "\\n";
        } else {
            result += current;
        }
    }

    result += "\"";
    return result;
}

// Pulls one value out of a small JSON text. It looks for the field name and
// then reads what comes after the colon, up to the next comma or bracket.
std::string getJsonField(const std::string& json, const std::string& fieldName) {
    std::string searchText = "\"" + fieldName + "\"";

    std::size_t fieldPosition = json.find(searchText);
    if (fieldPosition == std::string::npos) {
        return "";
    }

    std::size_t cursor = json.find(':', fieldPosition + searchText.size());
    if (cursor == std::string::npos) {
        return "";
    }
    cursor++;

    while (cursor < json.size() && std::isspace(static_cast<unsigned char>(json[cursor]))) {
        cursor++;
    }

    if (cursor >= json.size()) {
        return "";
    }

    // the value is a text, so it sits between two quote marks
    if (json[cursor] == '"') {
        cursor++;

        std::string value;

        while (cursor < json.size()) {
            if (json[cursor] == '\\' && cursor + 1 < json.size()) {
                cursor++;
                if (json[cursor] == 'n') {
                    value += ' ';
                } else {
                    value += json[cursor];
                }
            } else if (json[cursor] == '"') {
                break;
            } else {
                value += json[cursor];
            }
            cursor++;
        }

        return value;
    }

    // the value is a number, so there are no quote marks to stop at
    std::string value;

    while (cursor < json.size() && json[cursor] != ',' &&
           json[cursor] != '}' && json[cursor] != ']') {
        value += json[cursor];
        cursor++;
    }

    return trimText(value);
}

std::string makeResultJson(bool success, const std::string& message) {
    std::string json = "{\"ok\":";
    json += (success ? "true" : "false");
    json += ",\"message\":";
    json += toJsonString(message);
    json += "}";
    return json;
}

// Today's date in the DD-MM-YYYY form the data files use.
std::string getTodayDate() {
    std::time_t now = std::time(0);
    char buffer[32] = "";

#ifdef _WIN32
    std::tm localParts;
    localtime_s(&localParts, &now);
    std::strftime(buffer, sizeof(buffer), "%d-%m-%Y", &localParts);
#else
    std::tm* local = std::localtime(&now);
    std::strftime(buffer, sizeof(buffer), "%d-%m-%Y", local);
#endif

    return std::string(buffer);
}

// the number part of an id: "B001" -> 1, "R012" -> 12
int readNumberFromId(const std::string& id) {
    std::size_t position = 0;

    // walk over the letters until an ASCII digit starts the number
    while (position < id.size() && digitValue(id[position]) < 0) {
        position++;
    }

    if (position >= id.size()) {
        return 0;
    }

    return std::atoi(id.substr(position).c_str());
}

// one letter plus the number padded to three digits: 'B', 4 -> "B004"
std::string makeId(char letter, int number) {
    std::ostringstream text;
    text << letter << std::setfill('0') << std::setw(3) << number;
    return text.str();
}

// How many whole days have gone by since a DD-MM-YYYY date. This is what makes
// a reminder popup able to say "12 days late". The math is a simple subtract:
// today is turned into one number of seconds, the old date is turned into
// another, and the gap is divided by the seconds in a day. Anything that is
// empty or not a real date comes back as -1 so the caller can print a dash.
int daysSinceDate(const std::string& date) {
    if (date.empty() || date.size() < 10) {
        return -1;
    }

    int day = std::atoi(date.substr(0, 2).c_str());
    int month = std::atoi(date.substr(3, 2).c_str());
    int year = std::atoi(date.substr(6, 4).c_str());

    if (month < 1 || month > 12 || day < 1 || year < 1970) {
        return -1;
    }

    // tm_year counts from 1900, so 2026 becomes 126
    std::tm parts;
    parts.tm_sec = 0;
    parts.tm_min = 0;
    parts.tm_hour = 0;
    parts.tm_mday = day;
    parts.tm_mon = month - 1;
    parts.tm_year = year - 1900;
    parts.tm_isdst = -1;

    std::time_t then = std::mktime(&parts);
    if (then == static_cast<std::time_t>(-1)) {
        return -1;
    }

    std::time_t now = std::time(0);

    long seconds = static_cast<long>(std::difftime(now, then));
    if (seconds < 0) {
        return -1;
    }

    return static_cast<int>(seconds / 86400);
}
