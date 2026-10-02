// digital.h - the bit level toolbox of the project.
//
// Everything here works on single bits of a byte or a word, which is how a
// microprocessor handles flags, masks, shifted values and packed decimals.
// A flag register in an 8051 or a status word in a 8086 program is built the
// same way, so the routines below are the digital-techniques part of the code.

#ifndef DIGITAL_H              // ignore this file if it is included twice
#define DIGITAL_H

#include <cstddef>
#include <string>

/* The status of a book is kept in one byte, with one bit per meaning. Instead
   of three separate "yes/no" variables, the answer to each question is a single
   bit that is either 0 or 1, and several bits travel together in the same
   byte.

        bit 2   bit 1   bit 0
        ALL_OUT ON_LOAN ON_SHELF                                      */
enum BookFlag {
    BOOK_ON_SHELF = 1 << 0,   // 0000 0001 - at least one copy is on the shelf
    BOOK_ON_LOAN  = 1 << 1,   // 0000 0010 - at least one copy is with a member
    BOOK_ALL_OUT  = 1 << 2    // 0000 0100 - every single copy is out
};

// single bit work: turn a bit on, turn it off, flip it, or simply look at it
bool bitIsSet(unsigned int value, int bit);
unsigned int bitSet(unsigned int value, int bit);
unsigned int bitClear(unsigned int value, int bit);
unsigned int bitFlip(unsigned int value, int bit);

// whole groups of bits. The mask is a set of bits, not a bit number, so
// BOOK_ON_SHELF | BOOK_ALL_OUT switches off both of those bits at once.
unsigned int setMask(unsigned int value, unsigned int mask);
unsigned int clearMask(unsigned int value, unsigned int mask);

// The safe way to ask about a flag: the flag is used as the mask it already is.
// bitIsSet() wants a bit NUMBER, and a flag such as BOOK_ON_LOAN is the value
// 2, which would be read as "bit 2" and answer the wrong question.
bool flagIsSet(unsigned int value, unsigned int mask);

// keeps only the lowest bits of a value, which is how a wider number is
// squeezed into a narrower field
unsigned int keepLowBits(unsigned int value, int bitCount);

// counting and finding bits
int countBits(unsigned int value);           // how many bits are 1
int firstBitPosition(unsigned int value);    // place of the lowest 1, or -1

// base 16, needed to read a %2B inside a web address
int hexDigitValue(char digit);

// BCD (binary coded decimal) - every decimal digit sits inside its own four
// bits, which is how a classic 8051 keeps a clock time in two registers
int toBcd(int number);
int fromBcd(int packed);
unsigned int packTime(int hours, int minutes);   // hours in the high byte
int hoursFromTime(unsigned int packed);
int minutesFromTime(unsigned int packed);

// ASCII digits, checked by comparing the codes instead of calling a library
bool isDigitCharacter(char character);
int digitValue(char character);
bool isAllDigits(const std::string& text, std::size_t from, std::size_t count);

// checks the routines above with known answers and prints the result, so the
// startup window proves they really work
void runDigitalSelfTest();

#endif  // DIGITAL_H
