# Digital Techniques & Microprocessors in this project

This page maps the theory of the subject onto the actual lines of this
project, so it is easy to answer a viva question by opening one file.

All of the bit level work sits in two files:

| File | What it holds |
|------|----------------|
| `backend/digital.h` | the names of the bit level routines and the `BookFlag` bits |
| `backend/digital.cpp` | the working code of those routines + a self test |

The rest of the backend *uses* those routines for real work: it decides the
stock rules with flag bits, validates the issue time as packed BCD, reads base
16 out of the web address and checks digits by their ASCII codes.

---

## 1. Number systems and representation

### Binary, hex and nibbles

`%2B` inside a search address is two hex digits. The first digit is shifted
left four bits to become the high nibble and the second fills the low nibble:

```
   '2' -> 2,  shifted left 4 bits -> 0010 0000
   'B' -> 11, filled into the low nibble -> 0000 1011
                                        OR them -> 0010 1011 = 0x2B = '+'
```

* `hexDigitValue()` — `digital.cpp:107`
* the shift and OR — `http.cpp:35`
* `0x2B` is the ASCII code of `+`, which is why searching for **C++** works

This is also why the old `digit - 'a' + 10` version could not be used for a
capital `B`; the routine now accepts `A-F` and `a-f` (`digital.cpp:111-121`).

### ASCII codes instead of a library call

`'0'` is ASCII 48 and `'9'` is ASCII 57, so every digit character sits between
them. Two comparisons are enough:

```cpp
bool isDigitCharacter(char character) {
    return character >= '0' && character <= '9';
}
```

* `isDigitCharacter()` — `digital.cpp:159`
* used for the 10 digit phone check — `members.cpp:75` and `members.cpp:133`
* `digitValue()` returns the number, or `-1` for a non digit —
  `digital.cpp:164`
* `isAllDigits()` checks a fixed run of characters —
  `digital.cpp:173`, used by the time validator `records.cpp:37`
* `readNumberFromId()` finds the first digit of `B001` with `digitValue()` —
  `helpers.cpp:140`

### BCD - binary coded decimal

BCD keeps every decimal digit inside its own four bits, so 59 becomes
`0101 1001` instead of the binary `0011 1011`. A 8051 uses this layout because
its `DA` (decimal adjust) instruction expects exactly it.

```
  toBcd(59)   = ((59 / 10) << 4) | (59 % 10) = 0101 1001
  fromBcd(..) = ((packed >> 4) & 0x0F) * 10 + (packed & 0x0F)
```

* `toBcd()` / `fromBcd()` — `digital.cpp:126` and `digital.cpp:136`
* the issue time `HH:MM` is packed into **one word**, BCD hours in the high
  byte and BCD minutes in the low byte — `packTime()`, `digital.cpp:145`
* unpacked again for the range check — `records.cpp:47-51`

```
  14:35  ->  BCD 0001 0100 0000 0011 0111  ->  one word, two bytes
```

So `cleanTimeText()` accepts only a real clock time and anything else becomes
`-`. Tested values:

| Sent | Stored |
|------|--------|
| `14:35`, `00:00`, `23:59`, `09:05` | kept as typed |
| `24:00`, `12:60`, `99:99` | `-` (out of range) |
| `9:35`, `ab:cd`, `12:5`, empty | `-` (wrong shape) |

---

## 2. Boolean logic, flags and bit manipulation

### The status byte of a book

Instead of three separate yes/no variables, the state of a book lives in one
byte, one bit per question (`digital.h:20-25`):

```
   bit 2      bit 1       bit 0
   ALL_OUT    ON_LOAN     ON_SHELF
   0000 0100  0000 0010   0000 0001
```

This is the same idea as the flag register a microcontroller keeps in RAM.

`Book::refreshFlags()` (`books.cpp:11`) rebuilds it from the two copy counters:
the three bits are switched off together with one mask, then the bits that are
true are set again.

| Copies | ON_SHELF | ON_LOAN | ALL_OUT | Status shown |
|--------|----------|---------|---------|--------------|
| 1 total, 0 out | 1 | 0 | 0 | Available |
| 3 total, 1 out | 1 | 1 | 0 | Partly Issued |
| 3 total, 3 out | 0 | 1 | 1 | Issued |
| 1 total, 1 out | 0 | 1 | 1 | Issued |

### The routines and where they are used

| Routine | Does | Used at |
|---------|------|---------|
| `bitIsSet(value, bit)` | one AND with a shifted mask reads one bit | self test only |
| `bitSet(value, bit)` | OR switches one bit on | self test only |
| `bitClear(value, bit)` | AND with `~mask` switches one bit off | self test only |
| `bitFlip(value, bit)` | one XOR flips one bit | self test only |
| `setMask(value, mask)` | OR with a **group** of bits | `books.cpp:16,19,22`, `storage.cpp:69` |
| `clearMask(value, mask)` | AND with `~mask` for a group | `books.cpp:13` |
| `flagIsSet(value, mask)` | asks about a flag using its own mask | `books.cpp:158,181,185`, `records.cpp:89,153`, `storage.cpp:79,202` |
| `keepLowBits(value, n)` | keeps only the lowest `n` bits | self test |
| `countBits(value)` | population count | self test |
| `firstBitPosition(value)` | place of the lowest 1 | self test |

### A bit index is not the same as a bit value

This is the mistake worth remembering, because the project hit it twice.

`bitIsSet()` wants a bit **number**. A flag such as `BOOK_ON_LOAN` is the
**value** 2, and `1 << 2` is bit 2. Passing the flag to `bitIsSet()` therefore
asks about bit 2 (which is `ALL_OUT`) and silently answers the wrong question.
The same trap applies to `bitClear(flags, A | B | C)`, where `A | B | C`
cleared bit 7 instead of bits 0, 1 and 2.

Both mistakes are avoided on purpose:

* every flag test goes through `flagIsSet()`, which uses the flag as the mask
  it already is, so no bit number is ever involved;
* every multi bit change goes through `setMask()` / `clearMask()`, which take a
  group of bits.

### The stock rules are now flag tests

The three business rules are decided by testing one bit each:

* a book can be deleted only when `ON_LOAN` is clear — `books.cpp:158`
* a book can be issued only when `ALL_OUT` is clear — `records.cpp:89`
* a book can be returned only when `ON_LOAN` is set — `records.cpp:153`

The copy counters stay the single source of truth: `refreshFlags()` is called
after every change (`books.cpp:128`, `records.cpp:109`, `records.cpp:165`) and
after loading a file (`storage.cpp:94`). The return path also refuses to let
the counter go below zero (`records.cpp:161-164`).

### Old data files still load

`books.txt` stores the `Available` column as a plain `0` or `1`, and that is
exactly bit 0 of the status byte, so every existing file loads unchanged
(`storage.cpp:68-70`) and is written back in exactly the same shape
(`storage.cpp:202`). A file saved by the old version needs no conversion.

---

## 3. Combinational logic - the three way choice

`bookStatusText()` (`books.cpp:178-189`) is a multiplexer: two bit tests pick
one of three answers.

```
   ALL_OUT = 1  ->  "Issued"
   ON_LOAN = 1  ->  "Partly Issued"
   otherwise    ->  "Available"
```

The same pattern appears in `refreshFlags()`, where three comparisons drive
three independent bits, which is a set of AND gates with an OR.

---

## 4. Counters, registers and memory

| Idea | Where |
|------|-------|
| up counter, "find the highest id and add one" | `books.cpp:47-70`, `records.cpp:9-21` |
| down counter, copies leaving the shelf | `books.cpp` add / `records.cpp` issue and return |
| shift register, walking to the lowest set bit | `firstBitPosition()`, `digital.cpp:95` |
| fixed buffer holding a request | `char buffer[4096]` at `http.cpp` receive |
| heap storage for the records | `std::vector` inside the `Library` class |
| `value & (value - 1)` clears the lowest bit | `countBits()`, `digital.cpp:83` |

---

## 5. I/O, ports and the polling loop

This project is a PC program, not bare metal, but the ideas transfer directly:

| Microprocessor idea | Where it appears here |
|---------------------|------------------------|
| port number | 8080, put into `sin_port` with `htons()` (`server.cpp`) |
| byte swapping / endianness | `htons()` swaps the byte order of the port number |
| parallel vs serial | the socket is a **serial** byte stream, so `recv()` reads a request bit by bit, byte by byte, into a buffer |
| read, process, write | `recv()` -> route -> `send()` |
| polling loop | `while (true) { accept(); recv(); send(); }` - the same idea as a polling main loop, checked over and over |
| demultiplexer | the `if / else if` chain in `api.cpp` sends one path to one handler |
| timer / real time clock | `std::time()` and `strftime()` fill the issue date |

---

## 6. Verifying the routines

`runDigitalSelfTest()` (`digital.cpp:188`, called from `server.cpp:58`) checks
every routine against a known answer, and the black window prints the result on
every start:

```
Digital self-test : all bit routines passed
```

The self test earned its keep: it caught a wrong expected value
(`countBits(0xAB)` is 5, not 4) and a wrong assertion about masks before either
could be shown to an examiner.

---

## 7. Viva questions and short answers

**Q. Why did you use a bitmask instead of three booleans?**
One byte carries all three answers, so a book needs one field instead of three,
and the checks are single AND operations instead of several comparisons.

**Q. What is a mask?**
A value whose 1 bits mark the positions we care about. `value & mask` reads
those bits, `value | mask` sets them, `value & ~mask` clears them.

**Q. Why is BCD used for the issue time?**
Every decimal digit sits in its own four bits, so it is easy to read and to
change one digit, and it is the layout a 8051 expects for a clock time.

**Q. What is the difference between a bit number and a bit value?**
`BOOK_ON_SHELF` is the value 1, which is `1 << 0`, so its bit number is 0.
Passing the value where a bit number is expected asks about the wrong bit,
which is why `flagIsSet()` was added.

**Q. What does `%2B` mean and how is it read?**
Percent encoding: the hex bytes of a character that cannot travel in a web
address. `2` shifted left four bits OR `B` gives `0x2B`, the ASCII code of `+`.

**Q. How does the program know a book is fully issued?**
`ALL_OUT` is bit 2 of the status byte, set when the copies on the shelf reach
zero. The issue route tests that one bit.

**Q. Why is the counter the truth and not the bits?**
The bits are worked out from the counters after every change
(`refreshFlags()`), so the numbers in the file can never disagree with the
status shown on the screen.
