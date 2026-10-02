// members.cpp - everything about the library members.

#include "library.h"

#include <cctype>

int Library::findMemberIndex(const std::string& memberId) const {
    for (std::size_t i = 0; i < members.size(); i++) {
        if (members[i].id == memberId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::string Library::getMemberName(const std::string& memberId) const {
    int index = findMemberIndex(memberId);
    if (index < 0) {
        return "";
    }
    return members[index].name;
}

// same idea as getNextBookId but for members, counting only M ids
std::string Library::getNextMemberId() const {
    int highestNumber = 0;

    for (std::size_t i = 0; i < members.size(); i++) {
        if (members[i].id.empty() || members[i].id[0] != 'M') {
            continue;
        }

        int number = readNumberFromId(members[i].id);

        if (number > highestNumber) {
            highestNumber = number;
        }
    }

    int number = highestNumber + 1;
    std::string newId = makeId('M', number);

    while (findMemberIndex(newId) >= 0) {
        number++;
        newId = makeId('M', number);
    }

    return newId;
}

OperationResult Library::addMember(const Member& member) {
    OperationResult result;
    result.ok = false;

    if (trimText(member.name).empty()) {
        result.message = "Student name cannot be empty.";
        return result;
    }
    if (trimText(member.course).empty()) {
        result.message = "Course cannot be empty.";
        return result;
    }

    std::string phone = trimText(member.phone);
    if (phone.empty()) {
        result.message = "Phone number cannot be empty.";
        return result;
    }
    if (phone.size() != 10) {
        result.message = "Phone number must be exactly 10 digits.";
        return result;
    }
    for (std::size_t i = 0; i < phone.size(); i++) {
        // every character must sit between the ASCII codes of '0' and '9'
        if (!isDigitCharacter(phone[i])) {
            result.message = "Phone number must contain digits only.";
            return result;
        }
    }

    Member newMember;
    newMember.id     = getNextMemberId();
    newMember.name   = trimText(member.name);
    newMember.course = trimText(member.course);
    newMember.phone  = phone;

    members.push_back(newMember);
    saveToFiles();

    result.ok = true;
    result.message = "Member \"" + newMember.name + "\" added successfully with Member ID "
                   + newMember.id + ".";
    return result;
}

// only the name and the phone can change. the id and the course stay as they
// are, so the old issue records keep pointing at the right member.
OperationResult Library::editMember(const std::string& memberId,
                                    const std::string& newName,
                                    const std::string& newPhone) {
    OperationResult result;
    result.ok = false;

    std::string id = trimText(memberId);
    if (id.empty()) {
        result.message = "Member ID cannot be empty.";
        return result;
    }

    int index = findMemberIndex(id);
    if (index < 0) {
        result.message = "Member ID not found.";
        return result;
    }

    std::string name = trimText(newName);
    if (name.empty()) {
        result.message = "Student name cannot be empty.";
        return result;
    }

    std::string phone = trimText(newPhone);
    if (phone.empty()) {
        result.message = "Phone number cannot be empty.";
        return result;
    }
    if (phone.size() != 10) {
        result.message = "Phone number must be exactly 10 digits.";
        return result;
    }
    for (std::size_t i = 0; i < phone.size(); i++) {
        // same ASCII check as when the member is added
        if (!isDigitCharacter(phone[i])) {
            result.message = "Phone number must contain digits only.";
            return result;
        }
    }

    members[index].name  = name;
    members[index].phone = phone;

    saveToFiles();

    result.ok = true;
    result.message = "Member \"" + members[index].name + "\" updated successfully.";
    return result;
}

OperationResult Library::deleteMember(const std::string& memberId) {
    OperationResult result;
    result.ok = false;

    std::string id = trimText(memberId);
    if (id.empty()) {
        result.message = "Member ID cannot be empty.";
        return result;
    }

    int index = findMemberIndex(id);
    if (index < 0) {
        result.message = "Member ID not found.";
        return result;
    }

    // not while the member still has a book out
    for (std::size_t i = 0; i < records.size(); i++) {
        if (records[i].memberId == id && records[i].status == "Issued") {
            result.message = "This member still has an issued book. Please return it first.";
            return result;
        }
    }

    std::string deletedName = members[index].name;

    members.erase(members.begin() + index);
    saveToFiles();

    result.ok = true;
    result.message = "Member \"" + deletedName + "\" deleted successfully.";
    return result;
}

std::string Library::displayMembers() {
    std::string json = "[";

    for (std::size_t i = 0; i < members.size(); i++) {
        if (i > 0) {
            json += ",";
        }

        // how many books this member has right now
        int booksCount = 0;
        for (std::size_t k = 0; k < records.size(); k++) {
            if (records[k].memberId == members[i].id && records[k].status == "Issued") {
                booksCount++;
            }
        }

        json += "{\"id\":" + toJsonString(members[i].id) +
                ",\"name\":" + toJsonString(members[i].name) +
                ",\"course\":" + toJsonString(members[i].course) +
                ",\"phone\":" + toJsonString(members[i].phone) +
                ",\"booksCount\":" + std::to_string(booksCount) + "}";
    }

    json += "]";
    return json;
}
