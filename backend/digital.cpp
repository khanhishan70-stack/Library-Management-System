// digital.cpp - the bit level toolbox, written out.

#include "digital.h"

#include <iostream>

// "Is bit number 'bit' a 1?" A mask with only that one bit switched on is
// built by shifting 1 to the left, and one AND then reads that bit alone.
// 0000 1000 AND 0000 1000 = 0000 1000  -> bit 3 is a 1
// 0000 1000 AND 0000 0001 = 0000 0000  -> bit 0 is a 0
bool bitIsSet(unsigned int value, int bit) {
    if (bit < 0 || bit > 31) {
        return false;
    }

    unsigned int mask = 1u << bit;
    return (value & mask) != 0u;
}

// sets one bit to 1 and leaves every other bit alone
unsigned int bitSet(unsigned int value, int bit) {
    if (bit < 0 || bit > 31) {
        return value;
    }

    return value | (1u << bit);
}

// sets one bit to 0. AND with a mask that has only that bit OFF (written with
// ~) is the standard way to clear a bit.
unsigned int bitClear(unsigned int value, int bit) {
    if (bit < 0 || bit > 31) {
        return value;
    }

    return value & ~(1u << bit);
}

// one XOR flips exactly one bit
unsigned int bitFlip(unsigned int value, int bit) {
    if (bit < 0 || bit > 31) {
        return value;
    }

    return value ^ (1u << bit);
}

// (1 << bitCount) - 1 is a row of ones, for example 0000 1111 for 4 bits, and
// AND-ing with it throws away everything above that width
unsigned int keepLowBits(unsigned int value, int bitCount) {
    if (bitCount <= 0) {
        return 0u;
    }
    if (bitCount >= 32) {
        return value;
    }

    return value & ((1u << bitCount) - 1u);
}

// A mask holds several bits, not one bit number, so OR switches every bit of
// the mask on. Passing BOOK_ON_SHELF | BOOK_ALL_OUT switches bits 0 and 2 on
// together.
unsigned int setMask(unsigned int value, unsigned int mask) {
    return value | mask;
}

// AND with ~mask, where the mask is flipped first, so every bit of the mask
// becomes 0 and all the other bits stay as they were.
unsigned int clearMask(unsigned int value, unsigned int mask) {
    return value & ~mask;
}

// The flag is already a mask, so it is used directly. Every flag test in the
// project goes through here, which keeps a flag value from ever being read as
// if it were a bit number.
bool flagIsSet(unsigned int value, unsigned int mask) {
    return (value & mask) != 0u;
}

// The population count. value & (value - 1) clears the lowest set bit, so the
// loop runs once per 1 that was in the number and never touches the 0s.
int countBits(unsigned int value) {
    int ones = 0;

    while (value != 0u) {
        value &= value - 1u;
        ones++;
    }

    return ones;
}

// walks a shift register right until the lowest bit becomes a 1
int firstBitPosition(unsigned int value) {
    int position = 0;

    while (value != 0u && (value & 1u) == 0u) {
        value >>= 1;
        position++;
    }

    return (value == 0u) ? -1 : position;
}

// one hex digit as its real number 0-15: "2" in %2B and "B" in %2B
int hexDigitValue(char digit) {
    if (digit >= '0' && digit <= '9') {
        return digit - '0';
    }

    if (digit >= 'A' && digit <= 'F') {
        return digit - 'A' + 10;
    }

    if (digit >= 'a' && digit <= 'f') {
        return digit - 'a' + 10;
    }

    return 0;
}

// BCD: each decimal digit is stored in its own four bits, so 59 becomes
// 0101 1001 instead of the binary 0011 1011. A microcontroller uses this
// because its decimal adjust instruction expects exactly this layout.
int toBcd(int number) {
    if (number < 0 || number > 99) {
        return 0;
    }

    return ((number / 10) << 4) | (number % 10);
}

// reads the two nibbles back: (packed >> 4) & 0x0F is the tens digit and
// packed & 0x0F is the units digit
int fromBcd(int packed) {
    int tens   = (packed >> 4) & 0x0F;
    int units  = packed & 0x0F;

    return tens * 10 + units;
}

// both halves of a time in one word: BCD hours in the high byte, BCD minutes
// in the low byte
unsigned int packTime(int hours, int minutes) {
    return (static_cast<unsigned int>(toBcd(hours)) << 8)
           | static_cast<unsigned int>(toBcd(minutes));
}

int hoursFromTime(unsigned int packed) {
    return fromBcd(static_cast<int>((packed >> 8) & 0xFFu));
}

int minutesFromTime(unsigned int packed) {
    return fromBcd(static_cast<int>(packed & 0xFFu));
}

// '0' is ASCII 48 and '9' is ASCII 57, so every digit sits between them
bool isDigitCharacter(char character) {
    return character >= '0' && character <= '9';
}

// -1 means "this character is not a digit", which is a useful answer on its own
int digitValue(char character) {
    if (!isDigitCharacter(character)) {
        return -1;
    }

    return character - '0';
}

// checks a fixed run of characters, used for the HH:MM halves of a time
bool isAllDigits(const std::string& text, std::size_t from, std::size_t count) {
    if (from + count > text.size()) {
        return false;
    }

    for (std::size_t i = 0; i < count; i++) {
        if (!isDigitCharacter(text[from + i])) {
            return false;
        }
    }

    return true;
}

// known answers, checked once when the program starts
void runDigitalSelfTest() {
    bool ok = true;

    // 0000 1000: bit 3 is on, bit 0 is off, and clearing bit 3 empties it
    unsigned int mask = bitSet(0u, 3);
    ok = ok && bitIsSet(mask, 3) && !bitIsSet(mask, 0);
    ok = ok && bitClear(mask, 3) == 0u;
    ok = ok && bitFlip(0u, 5) == keepLowBits(32u, 6);

    // a mask is a group of bits: bits 0 and 2 together switch off at once,
    // while bit 1 survives untouched
    unsigned int group = bitSet(bitSet(0u, 0), 1);
    group = bitSet(group, 2);
    ok = ok && clearMask(group, BOOK_ON_SHELF | BOOK_ALL_OUT) == BOOK_ON_LOAN;
    ok = ok && setMask(0u, BOOK_ON_LOAN | BOOK_ALL_OUT)
              == (BOOK_ON_LOAN | BOOK_ALL_OUT);

    // a flag test asks about the mask itself, so it cannot drift onto a
    // neighbouring bit the way a bit number would
    unsigned int shelfOnly = setMask(0u, BOOK_ON_SHELF);
    ok = ok && flagIsSet(shelfOnly, BOOK_ON_SHELF);
    ok = ok && !flagIsSet(shelfOnly, BOOK_ON_LOAN);
    ok = ok && !flagIsSet(shelfOnly, BOOK_ALL_OUT);
    ok = ok && flagIsSet(setMask(shelfOnly, BOOK_ON_LOAN), BOOK_ON_LOAN);

    // 1010 1011 holds five ones, and its lowest 1 sits at bit 0
    ok = ok && countBits(0xAB) == 5;
    ok = ok && firstBitPosition(0x80) == 7;
    ok = ok && firstBitPosition(0u) == -1;

    // base 16: "2" shifted left four bits plus "B" gives 0x2B, the "+" of %2B
    ok = ok && hexDigitValue('2') == 2 && hexDigitValue('B') == 11;
    ok = ok && ((hexDigitValue('2') << 4) | hexDigitValue('B')) == 0x2B;

    // BCD round trip, and a time packed into a single word
    ok = ok && fromBcd(toBcd(59)) == 59;
    ok = ok && hoursFromTime(packTime(14, 35)) == 14;
    ok = ok && minutesFromTime(packTime(14, 35)) == 35;

    std::cout << "Digital self-test : "
              << (ok ? "all bit routines passed" : "SOME BIT ROUTINES FAILED")
              << "\n";
}
