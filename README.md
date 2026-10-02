# Library Management System

A simple **Library Management System** made for a diploma / DTM college project.

The frontend is built with **HTML, CSS and vanilla JavaScript**.
The backend and all the real logic are written in **C++**, and the data is saved in
**plain text files** (no MySQL, no MongoDB, no Firebase, no framework).

---

## 1. Project Introduction

This project is a computer program that helps a librarian to manage a library.
The librarian can add books, add members, give (issue) a book to a student,
take a book back (return) and see the full history of every issue and return.

The user interface looks like a modern dashboard in the browser, but the
program that actually does the work is written in C++.

---

## 2. Project Objectives

1. To replace the manual register book of a library with a computer program.
2. To learn how a web page (HTML/CSS/JS) talks to a program written in C++.
3. To understand how data can be stored in simple text files.
4. To show the complete working of a small software project: frontend,
   backend, data storage, validation and documentation.

---

## 3. Technologies Used

| Part | Technology | Why |
|------|-----------|-----|
| Page structure | **HTML** | Gives the structure of the page (buttons, forms, tables). |
| Page design | **CSS** | Gives the colours, layout, cards, animations, dark/light mode. |
| User interaction | **Vanilla JavaScript** | Sends requests, fills tables, validates forms, shows messages. |
| Backend logic | **C++ (C++17)** | Does all the real work using `struct` and a `class`. |
| Web server | **C++ sockets** | A small built-in HTTP server, so no extra software is needed. |
| Data storage | **Text files** (`ifstream` / `ofstream`) | Simple, and easy to open and check in Notepad. |

**Not used:** React, Node.js, Python, PHP, Java, Firebase, MongoDB, MySQL.

---

## 4. Features

* **Dashboard** with four live counters: Total Books, Available Books,
  Issued Books and Total Members. They update automatically.
* **Books**: add, view, search and delete books.
  Search works on Book ID, Book Name and Category (not case sensitive).
* **Stock / copies of a book**: the librarian types how many **copies** the
  library owns of a title, for example 3 copies of *C++ Programming*.
  * The **Books** list then shows a **Copies Left** column and a **Total** column.
  * Every time a student takes the book, **Copies Left** goes down by one and the
    status becomes **Partly Issued**.
  * When the last copy is taken, **Copies Left** shows `0 (none left)`, the status
    becomes **Issued**, and the title disappears from every Available Books panel.
  * When a book comes back, **Copies Left** goes up by one and the title appears
    in the panels again.
* **Easy book entry**: the librarian types the **Book Name**, the **Year** and the
  **number of copies**. The **Book ID** and the **Category** are made by the C++
  backend by itself (ID = next free `B001`, `B002`... ; Category = `General`).
* **Small book panel**: clicking the Books search box opens a drop-down that
  lists the names of all the **Available** books. The panel has its own small
  search box, and clicking one name fills the main search box automatically.
  Issued books never appear in the panel.
* **Pending Return Reminder (bell icon in the top bar)**: a book that has not come
  back yet is a book the librarian has to chase, so the project warns about it.
  * A **bell button** appears next to the theme button on **every** page, with a
    red badge showing how many books are still out.
  * Clicking the bell opens a **popup** that lists, for each book that is missing:
    the **Book Name** and ID, the **Member Name**, the **Member ID**, the
    **Phone Number** (as a `tel:` link, so it can be dialled straight away),
    the **Issue Date** with the time, and the **number of days the book has been out**.
  * The days column turns **orange** after a day and **red** after 14 days, and the
    longest missing book is shown first.
  * The popup makes a short **ring tone** when it opens. The sound is made with the
    Web Audio API (no audio file needed) and the 🔊 / 🔇 button in the popup turns
    it off; the choice is remembered.
  * On the **Dashboard** the popup opens by itself **once a day** when something is
    still missing. It does not come back on later refreshes of the same day, and
    the other pages only show it when the bell is clicked.
  * If a book was given to a member who has since been deleted, the row still
    shows and the member is marked `deleted member` instead of leaving a blank cell.
* **Members**: add, view, filter, edit and delete members.
  * Clicking the **Course** box opens the same small panel with the **Available**
    books (with its own search box), so the staff can pick a name from the list
    instead of typing it. The box can still be typed into by hand as well.
  * The **Member ID** is made by the C++ backend by itself (`M001`, `M002`, ...),
    so the librarian types only the Student Name, the Course and the Phone Number.
  * Every member row has an **Edit** button that opens a small popup where the
    **Student Name** and the **Phone Number** can be changed. The Member ID and
    the Course are locked, so the saved issue records stay correct.
* **Issue Book**: the Book box opens the same small panel with the **Available**
  books, so the staff never has to type or remember a Book ID. Clicking a book
  fills the box. An Issued book is never listed.
  The book is then given to an existing member and a record is created.
* **Return Book**: changes the book back to Available and completes the record.
* **Records**: shows the full history with Record ID, Book, Member, Issue Date,
  Return Date and Status.
* **Dark / Light theme** button (the choice is remembered in the browser).
* **Success and error messages** shown as small pop-ups (toasts).
* **Simple, fast, case-insensitive search**.
* **Validation** on both sides: in the browser *and* in the C++ code.
* **Responsive design**, so it also works on a phone screen.

---

## 5. Project Folder Structure

```
libarary managment/
│
├── frontend/                     <-- THE USER INTERFACE (what we see)
│   ├── index.html                 the Dashboard page
│   ├── books.html                 the Books page
│   ├── members.html               the Members page
│   ├── issue.html                 the Issue Book page
│   ├── return.html                the Return Book page
│   ├── records.html               the Records page
│   │
│   ├── css/
│   │   └── style.css              all the design (colours, cards, animations)
│   │
│   └── js/
│       ├── common.js              helpers, sidebar, theme, search panels
│       ├── dashboard.js           the numbers and the two small tables
│       ├── books.js               add, search and delete books
│       ├── members.js             add, filter, edit and delete members
│       ├── issue.js               issue a book (with the issue time)
│       ├── return.js              take a book back
│       └── records.js             the history table
│
├── backend/                      <-- THE PROGRAM (what really works)
│   ├── server.cpp                 starts the web server and waits for requests
│   ├── api.cpp                    the /api routes (one small if per route)
│   ├── http.cpp                   building and reading the web answers
│   ├── http.h                     the names of those web helpers
│   ├── storage.cpp                reads and writes the three text files
│   ├── books.cpp                  the book logic
│   ├── members.cpp                the member logic
│   ├── records.cpp                issue, return and the history
│   ├── dashboard.cpp              the four dashboard numbers
│   ├── helpers.cpp                text, JSON and date helpers
│   ├── digital.h                  the bit level toolbox (flag bits, BCD, hex)
│   ├── digital.cpp                the working code of those bit routines
│   ├── library.h                  the structures and the class declaration
│   ├── build.bat                  compiles the C++ code into an .exe file
│   ├── run.bat                    starts the program and opens the browser
│   │
│   └── data/                      <-- THE DATABASE OF THIS PROJECT
│       ├── books.txt              all books
│       ├── members.txt            all members
│       └── records.txt            all issue / return records
│
├── docs/
│   └── digital_techniques.md      how the digital techniques are used here
│
├── .gitignore                     tells git to ignore the compiled files
└── README.md                      this file
```

> **Note:** `library_server.exe` and the `.obj` files are created after
> compiling. They are not kept in the project folder in git because they are
> just build results.

---

## 6. How the Frontend Works

Every part of the system has **its own HTML page**, so there is one file for one
screen:

| Page | File | JavaScript that runs on it |
|------|------|----------------------------|
| Dashboard | `index.html` | `js/dashboard.js` |
| Books | `books.html` | `js/books.js` |
| Members | `members.html` | `js/members.js` |
| Issue Book | `issue.html` | `js/issue.js` |
| Return Book | `return.html` | `js/return.js` |
| Records | `records.html` | `js/records.js` |

The sidebar moves between them with normal links:

```html
<a class="nav-link" href="books.html">Books</a>
```

`js/common.js` is loaded on every page and holds the parts that are the same
everywhere: the sidebar, the theme button, the clock, the toast messages, the
search panels and the small helper functions. Each page then loads only its own
`js` file, so the code of one screen stays inside that screen.

---

## 7. How the JavaScript Works

`js/common.js` holds the shared parts, and each page file holds only its own
work:

| File | What it does |
|------|--------------|
| `js/common.js` | `apiGet()`, `apiPost()`, `apiDelete()`, `showToast()`, `showFieldError()`, `setupLayout()`, `setupPanel()`, `formatDate()`, `formatTime()`, plus the whole pending-return reminder (`openReminderPopup()`, `playReminderSound()`, `refreshReminderBadge()`) |
| `js/dashboard.js` | fills the four numbers and the two small tables |
| `js/books.js` | add, search and delete books, plus the next-id preview |
| `js/members.js` | add, filter, edit (modal) and delete members |
| `js/issue.js` | issue a book, with the issue time |
| `js/return.js` | take a book back |
| `js/records.js` | the history table |

**The bridge to the backend**

JavaScript uses the browser's built-in `fetch()` function:

```javascript
// get data
const books = await fetch("/api/books");

// send new data - note that ONLY name, year and copies are sent
await fetch("/api/books", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ name: "Java Basics", year: 2025, copies: 3 })
});
```

The Book ID and the Category are **not** sent by the browser. The C++ code
creates them by itself, which is why they can never clash or be typed wrongly.

The C++ program answers with JSON text, and JavaScript reads it using
`response.json()`. For example:

```json
{ "ok": true, "message": "Book added successfully." }
```

`ok` tells us if it worked, and `message` is shown to the user.

**Two levels of validation**

* First the browser checks the form quickly (empty box, wrong year, wrong phone).
* Then the C++ backend checks again (duplicate ID, book not found, already issued).

This is good practice, because the backend must never trust the frontend.

---

## 8. How the C++ Backend Works

The backend has three files:

### `library.h` — the "blueprint"

It contains the three structures used in the project:

```cpp
struct Book {
    std::string id, name, author, category;
    int year;
    int totalCopies;      // how many copies the library owns
    int issuedCopies;     // how many are given out
    bool available;       // true when at least one copy is left

    // Copies still on the shelf. Example: 3 owned - 2 issued = 1 left.
    int availableCopies() const {
        int left = totalCopies - issuedCopies;
        return left < 0 ? 0 : left;
    }
};
struct Member { std::string id, name, course, phone; };
struct IssueRecord {
    std::string recordId, bookId, memberId, issueDate;
    std::string issueTime;   // "14:35" (24 hours), or "-" if not known
    std::string returnDate, status;
};
```

> The `author` field is still kept inside the struct and inside `books.txt`, so
> old data files keep working. It is simply **never shown** on the screen and
> never sent to the browser any more.

and it declares the `Library` class with all the functions:

```cpp
addBook()      deleteBook()     searchBooks()    displayBooks()
addMember()    editMember()     deleteMember()   displayMembers()
issueBook()    returnBook()     displayRecords()
loadFromFiles() saveToFiles()   getDashboardStats()
getNextBookId() getNextMemberId() getNextRecordId()
```

### The `.cpp` files — one part of the project each

Every part of the backend lives in its own file, so it is easy to find:

| File | What lives there |
|------|------------------|
| `server.cpp` | creates the socket and waits for requests |
| `api.cpp` | the `/api` routes, one small `if` per route |
| `http.cpp` | building the answers, reading a request, serving the files |
| `storage.cpp` | `loadFromFiles()` and `saveToFiles()` |
| `books.cpp` | `addBook()`, `deleteBook()`, `searchBooks()`, `displayBooks()` |
| `members.cpp` | `addMember()`, `editMember()`, `deleteMember()`, `displayMembers()` |
| `records.cpp` | `issueBook()`, `returnBook()`, `displayRecords()`, `displayPendingReturns()`, `getMemberPhone()` |
| `dashboard.cpp` | `getDashboardStats()` |
| `helpers.cpp` | `trimText()`, `toJsonString()`, `getJsonField()`, `getTodayDate()`, `makeId()`, `readNumberFromId()`, `daysSinceDate()` |

### The pending-return reminder, in the backend

`displayPendingReturns()` in `records.cpp` builds the list the popup shows:

1. It walks `records` and keeps only the ones whose status is `Issued`.
   A returned book never reaches the reminder.
2. It sorts those rows with `std::stable_sort` so the **longest missing book is
   first**. `stable_sort` is used instead of `sort` because two books taken on the
   same day keep the order they were issued in, so the list does not jump about
   between two refreshes.
3. For every row it calls `getMemberPhone()` and `daysSinceDate()`.

`daysSinceDate()` in `helpers.cpp` turns a stored `DD-MM-YYYY` date into the number
of whole days that have gone by, by turning both dates into a number of seconds
and dividing the gap by `86400` (the seconds in a day):

```cpp
long seconds = static_cast<long>(std::difftime(now, then));
return static_cast<int>(seconds / 86400);
```

It returns `-1` for anything it cannot use, and the popup prints a dash for that:
an empty date, a text that is not a date, a month above 12, or a date that is still
in the future. That is why a book issued today shows `today` and not `0 days`.

All of them are small functions, and the logic is easy to follow.

**How the copies (stock) work**

A library owns several copies of the same title, so every book remembers two
numbers:

* `totalCopies` — how many copies the library owns (typed by the librarian).
* `issuedCopies` — how many copies are currently given out.

The number that matters on the screen is worked out from them:

```cpp
int availableCopies() const {
    int left = totalCopies - issuedCopies;
    return left < 0 ? 0 : left;
}
```

**Status is worked out from the numbers**, so it can never be wrong:

| Situation | Status shown |
|-----------|--------------|
| `issuedCopies == 0` | Available |
| `0 < issuedCopies < totalCopies` | Partly Issued |
| `issuedCopies == totalCopies` | Issued (0 copies left) |

* **Issuing** a book is refused when `availableCopies() == 0` with the message
  *"No copy of "..." is available. All 3 copies are already issued."*
  Otherwise `issuedCopies` is increased by one.
* **Returning** a book is refused when `issuedCopies == 0`
  (*"This book is already available. Nothing to return."*).
  Otherwise `issuedCopies` is decreased by one.
* **Deleting** a title is refused while `issuedCopies > 0`, so no record can ever
  point at a book that no longer exists.
* One member cannot take the same title twice at the same time.

The three dashboard numbers (**Total Books**, **Available Books**, **Issued
Books**) are counted in **copies**, not in titles, because that is what a real
library total means.

**How the Book ID is made automatically**

```cpp
std::string Library::getNextBookId() const {
    int highestNumber = 0;

    // only the ids that start with 'B' are counted
    for (std::size_t i = 0; i < books.size(); i++) {
        if (books[i].id.empty() || books[i].id[0] != 'B') { continue; }
        int number = readNumberFromId(books[i].id);
        if (number > highestNumber) { highestNumber = number; }
    }
    // B001, B002, ... - and we try the next number if it is already used
    ...
}
```

Only ids that begin with `B` are counted, so a book that was saved by hand with
another id (for example `K001`) does not disturb the numbering, and the safety
loop makes sure a new id can never clash with an existing one.

**How the Category is decided**

The browser never sends a category. `addBook()` simply stores `General` for
every new book, so the value can never be misspelled.

**How the Member ID is made automatically**

`getNextMemberId()` works exactly like `getNextBookId()`, but with the letter
`M`: it counts only the ids that start with `M`, adds one, and keeps trying the
next number if that one is already used. So a member saved by hand with another
id (for example `KOO1`) does not disturb the numbering.

**Why can a member not change their ID or course?**

The saved issue records (`records.txt`) store the Member ID. If the ID could be
edited, the old records would point at a member who no longer exists. So
`editMember()` is allowed to change **only the Student Name and the Phone
Number**, and the C++ code refuses anything else.

**`issueBook()`** (in `records.cpp`) follows the steps directly:

1. Check the Book ID and Member ID are not empty.
2. Find the book → if not found, stop.
3. Find the member → if not found, stop.
4. If no copy is left, stop with *"No copy of "..." is available."*
5. Increase `issuedCopies` by one, so one copy leaves the shelf.
6. Create a new record with the next Record ID (R001, R002, ...).
7. Save everything into the text files.
8. Send back the success message.

### `server.cpp` and `api.cpp` — the web server

The browser cannot read files by itself, so these files make a **small web
server**:

* `server.cpp` creates a socket and waits on port `8080`.
* When the browser asks for something, `api.cpp` reads the request and answers:
  * an address that starts with `/api/` calls the matching function of the
    `Library` class;
  * anything else sends the matching file of the `frontend` folder, so
    `books.html`, `css/style.css` and `js/common.js` are served the same way.

The complete list of requests (this is the API):

| Request | Result |
|---------|--------|
| `GET /api/dashboard` | the four dashboard numbers |
| `GET /api/books` | all books |
| `GET /api/search?q=...` | search books |
| `POST /api/books` | add a book (body: `name`, `year`, `copies`) |
| `DELETE /api/books?id=B001` | delete a book |
| `GET /api/members` | all members |
| `POST /api/members` | add a member (body: only `name`, `course`, `phone`) |
| `POST /api/members/update` | edit the name + phone of a member |
| `DELETE /api/members?id=M001` | delete a member |
| `GET /api/records` | all issue / return records |
| `GET /api/pending` | only the books still out, with the member's phone number and the days late (the reminder popup) |
| `POST /api/issue` | issue a book (body: `bookId`, `memberId`, `issueDate`, `issueTime`) |
| `POST /api/return` | return a book |

**Why is the server written by hand?**
Because the project is not allowed to use frameworks. Writing it with the
`socket` functions keeps everything inside our own C++ code, and it is still
small enough to explain in a viva.

---

## 9. How File Storage Works

This is the important part of the project. There is **no database server** —
the data is stored in three normal text files.

### The format of the files

`data/books.txt`

```
B007 | Java Basics | - | General | 2025 | 1 | 3 | 1
```

Fields: `BookID | BookName | Author | Category | Year | Available | TotalCopies | IssuedCopies`

In this example the library owns **3** copies of *Java Basics* and **1** is
issued, so **2** are still on the shelf.

* `Available` is `1` when at least one copy is on the shelf, `0` when not.
* The **BookID** is written by the program, not by you.
* The **Category** is always `General` for a new book.
* The **Author** column is kept only so that old data files still open
  correctly. It is not shown anywhere in the program, and the program puts `-`
  there for a new book.
* **Old data files still work.** A line that has only the first 6 fields is read
  as *one copy*, issued if the old `Available` flag was `0`. The next time the
  file is saved it is rewritten with all 8 fields.

`data/members.txt`

```
M001 | Hishan Khan | Computer Engineering | 9876543210
```

Fields: `MemberID | MemberName | Course | PhoneNumber`

`data/records.txt`

```
R001 | B001 | M001 | 01-10-2026 | 14:35 | 05-10-2026 | Returned
```

Fields: `RecordID | BookID | MemberID | IssueDate | IssueTime | ReturnDate | Status`
(`-` in ReturnDate means "not returned yet")

* **IssueTime** is the clock time of the issue in the 24 hour form `HH:MM`, for
  example `14:35` for 2:35 in the afternoon. The file keeps `14:35`, but the
  records table shows it in the friendlier 12 hour form `2:35 PM`.
* `-` in IssueTime means "the time is not known". That happens for a record that
  was created before the time box was added, so **old record files still open
  correctly**.
* The backend checks the time once more before saving. If it is not a correct
  time (for example `25:99` or `abc`) it is stored as `-`, so a wrong value can
  never reach the file.
* In the Issue form the time box is **optional**. If it is left empty, the current
  clock time is used, so a record always has a sensible time on it.

Lines starting with `#` are comments. The program skips them while reading.

### The four steps

1. **Program starts** → `loadFromFiles()` opens the three files with `ifstream`
   and fills three vectors (`books`, `members`, `records`) in memory.
2. **User works** → the user adds, deletes, issues or returns. Only the memory is changed.
3. **After every change** → `saveToFiles()` writes everything back with
   `ofstream ... std::ios::trunc` (trunc = delete the old content and write again).
4. **Program is closed and opened again** → step 1 repeats, so all the old data is back.

### Why save after every change?

It is the simplest and safest method. If the program closes suddenly, nothing is lost,
because the text files are already up to date.

---

## 10. How to Compile the C++ Backend

You only need to do this **once**.

### Easiest way (recommended)

1. Open the `backend` folder.
2. Double click **`build.bat`**.

It checks which compiler you have and builds the program:

* Microsoft Visual C++ → `cl /EHsc /std:c++17 *.cpp`
* MinGW-w64 → `g++ -std=c++17 -O2 *.cpp -lws2_32`

Because it compiles `*.cpp`, every `.cpp` file in the folder is included, so a
new file never has to be added by hand anywhere.

After a successful build you will see `library_server.exe` inside `backend`.

### Manual way (command prompt)

**With Visual Studio** — open *Developer Command Prompt* and run:

```bat
cl /nologo /EHsc /std:c++17 /Fe:library_server.exe *.cpp
```

**With MinGW (g++)**:

```bat
g++ -std=c++17 -O2 -o library_server.exe *.cpp -lws2_32
```

`-lws2_32` is the Windows socket library and is needed on MinGW.
On Linux/macOS, compile without it: `g++ -std=c++17 -O2 -o library_server *.cpp`.

> The code uses `std::filesystem`, so a **C++17** compiler is needed.

---

## 11. How to Run the Complete Project

1. Open the `backend` folder.
2. Double click **`build.bat`** (only the first time).
3. Double click **`run.bat`**.

`run.bat` starts the C++ program and your browser opens the page automatically.

If you prefer to do it by hand:

```bat
cd backend
library_server.exe ".." 8080
```

Then open this address in Chrome / Edge / Firefox:

```
http://localhost:8080
```

**To stop the program:** press `Ctrl + C` in the black window, or just close it.

**The black window must stay open** while you use the project, because it is the server.

### What if it does not work?

| Problem | Solution |
|---------|----------|
| "Port 8080 is already in use" | Another program is using the port. Try another port: `library_server.exe ".." 8081` and open `http://localhost:8081` |
| Page shows "Cannot reach the C++ backend" | The `.exe` is not running. Start `run.bat`. |
| Page is blank / 404 | The project folder was not found. Start the program from inside the `backend` folder, or give the folder: `library_server.exe C:/MyProject` |
| "Build failed" | Install Visual Studio Build Tools or MinGW-w64 |

---

## 12. Example Workflow (for the demonstration)

1. **Start** `run.bat`. The dashboard shows **7 books, 7 available, 0 issued, 4 members**.
2. Go to **Books**. The form has three boxes you can type in: **Book Name**,
   **Year** and **How Many Copies**. The **Next Book ID** box shows `B007` and the
   **Category** box shows `General`.
3. Type `C++ Programming`, `2024` and `3` copies, click **Add Book**.
   * Success message: *Book "C++ Programming" added successfully with Book ID B007 (3 copies).*
   * The row shows **Copies Left `3`**, **Total `3`**, status **Available**.
   * The dashboard **Total Books** goes from 7 to 10, because it counts copies.
4. Press **Add Book** with `0` copies → red message:
   *"Copies must be a number between 1 and 999."*
5. Go to **Issue Book**, click the **Book** box → the Available Books panel opens and
   shows `C++ Programming (3 left)`. Click it → the Book ID box is filled with `B007`.
   Enter `M001`, pick `02-10-2026` in the **Issue Date** box and set the **Issue Time**
   box to `14:35`, then click **Issue Book**.
   * Success message: *... issued to Ravi Kumar (R004). 2 copies left.*
   * The books list now shows **Copies Left `2`** and status **Partly Issued**.
   * The **Records** page now has a **Time** column, and it shows `2:35 PM`
     (the file keeps `14:35`). The dashboard's *Currently Issued Books* box shows
     the time as well.
   * The **Issue Time** box is already filled with the current clock time, so in
     normal use you do not need to touch it. If you clear it and press the box
     again, the current time comes back.
6. Issue `B007` to `M002` → **Copies Left `1`**.
7. Issue `B007` to `M003` → **Copies Left `0 (none left)`**, status **Issued**,
   and the dashboard shows 3 issued / 7 available.
8. Click the Book box again → `B007` is **not** listed anywhere any more, because no
   copy is left. Trying to issue it gives:
   *"No copy of "C++ Programming" is available. All 3 copies are already issued."*
9. Go to **Return Book**, enter `B007`, date `03-10-2026`.
   * Success message: *... returned successfully (R004). 1 copy left.*
   * **Copies Left** goes back to `1`, and the title appears in the panels again.
10. Press **Delete** on `B007` → refused: *"This book still has 2 copies issued.
    Please return them first."* Return the rest and then it can be deleted.
11. **Close the program** and start it again → all the copy counts are still correct,
    because everything was saved in the text files.
12. Press the 🌙 button to switch to the light theme.

---

## 13. Limitations

* Only one user can use it at a time (the C++ server handles one request after another).
* It runs on one computer only; there is no user login or password.
* There are no fine / due dates, so there is no fine calculation and no overdue reminder.
* The JSON reading code is a small simple one; it works for this project but is not a full JSON library.
* The text files grow forever; old records are never cleared automatically.
* It works on port 8080, so if that port is busy the program must be given another port.
* No printing of a bill or a due-date slip.

---

## 14. Future Improvements

1. Add a **login page** so only the librarian can use it.
2. Add **due date and fine calculation** (for example 1 rupee per day).
3. Add a **print bill** when a book is issued or returned.
4. Add **barcode scanning** instead of typing the Book ID.
5. Show **charts** on the dashboard (most issued books, most active students).
6. Add **email / SMS alerts** to the member when a book is due.
7. Add a **date filter and a record export** to CSV / PDF.
8. Convert the text file storage to **MySQL or SQLite** when the project grows.

---

## 15. Simple Viva Explanation

Keep these answers short, in your own words.

**What is a Library Management System?**
It is a computer program that manages a library: it stores the books and the members,
gives books to students and takes them back, instead of writing everything in a register.

**Why did we create this project?**
To replace the manual register, to save time and mistakes, and to learn how a
frontend and a C++ program work together in a real project.

**Why HTML?**
HTML gives the structure of the page — the menu, the forms, the buttons and the
tables. It is the simplest language for showing content.

**Why CSS?**
CSS gives the look — the colours, the cards, the rounded buttons, the dark and
light theme, and the responsive layout. It is separated from HTML so the design
can be changed without touching the structure.

**Why JavaScript?**
JavaScript makes the page react. It sends the data to the C++ program, fills the
tables with the answer, checks the form and shows the success / error messages.

**Why C++?**
Because C++ is fast, uses `struct` and `class` clearly, has good file handling,
and it was our subject for the project. It also shows real logic, not just page design.

**Why file-based storage?**
It is the simplest way to save data — no MySQL or MongoDB has to be installed,
so the project runs on any computer. It is also easy to open the data file in Notepad
and show the examiner the real saved data.

**What is the role of the C++ backend?**
The backend is the part that really works. It holds all the data, checks all the
conditions, saves the data in the files and sends the answers back to the page.

**Why does the librarian type only the book name, the year and the copies?**
Because the rest is decided by the program, not by the person. The C++ code
finds the highest `B` number that is already used and adds one, so the Book ID
can never be repeated and can never be typed wrongly. The Category is simply
fixed to `General`, so it also cannot be misspelled. This removes two chances
of mistakes and saves time while entering books.

**Why is there a time box in the Issue form, and why is the time optional?**
A date alone only says *which day*, so two books taken on the same day cannot be
told apart. The **Issue Time** box records the exact moment, in the 24 hour form
`HH:MM`, and the record keeps it next to the date: `02-10-2026 | 14:35`. The
screen then shows it in the friendly 12 hour form `2:35 PM`, which is easier to
read for a human, while the file keeps the simple form that is easy for the
program to work with.

The box is **optional on purpose**. It is already filled with the current clock
time, so in normal use the librarian does nothing. But if it is left empty the
program fills in the current time instead, which means a record never has an
empty time. Old records that were created before this box existed simply show a
`-`, because at that moment nobody wrote the time down. This is also why the
backend checks the value one more time with `cleanTimeText()`: a value such as
`25:99` or `abc` is replaced by `-`, so the data file can never end up with a
broken time in it.

**How does the system know how many books are left?**
Every book title remembers two numbers: how many copies the library owns
(`totalCopies`) and how many are given out (`issuedCopies`). The number shown on
the screen is simply the difference, `totalCopies - issuedCopies`. When a
student takes a book, `issuedCopies` goes up by one, so the Copies Left column
goes down by one. When the book comes back, it goes down by one, so Copies Left
goes up again. Because the status word is also worked out from these two
numbers, the screen can never show something wrong.

**Why does a finished book disappear from the panel?**
The Available Books panel is meant for books that can really be taken right now.
A title with `0` copies left cannot be issued, so showing it would only make the
staff try and get an error. The panel therefore keeps only the books where
`availableCopies > 0`, and the moment the last copy is taken the title vanishes
from all three panels. When one copy is returned it comes back automatically,
because the panel asks the backend for the fresh list every time it opens.

**What is the small panel that opens under the search box?**
It is a drop-down list of the books that are Available at that moment. When it
opens, JavaScript asks the backend for the current list, keeps only the
Available books and shows their names. The panel has its own small search box to
shorten the list, and clicking one name fills the main search box. An Issued
book is never shown there, because it cannot be issued again.

**Why does the same panel open in three places?**
The panel is one small reusable piece, and it is used wherever the staff has to
pick a book: on the **Books** search box, on the **Course** box of the member
form, and on the **Book** box of the Issue Book form. All three call the same
JavaScript function (`renderBookPanel()`), so there is only one copy of the
logic to look after and all three lists always show the same Available books.

**Why can a member's ID and course not be edited?**
The issue records in `records.txt` store the Member ID. If the ID could be
changed, the old records would point to a member that no longer exists. So the
Edit popup allows only the Student Name and the Phone Number, and the ID and
the course are shown there just as information, locked.

**How does book issuing work?**
The librarian enters the Book ID and the Member ID. The backend checks that the book
exists, the member exists and the book is available. If all is fine, the book status
becomes Issued, a new record is created, the file is saved and a success message appears.

**How does book returning work?**
The librarian enters the Book ID. The backend checks that the book is currently issued.
Then the status becomes Available, the matching record gets the return date and the
status Returned, the file is saved and a success message appears.

**How is data stored?**
In three text files inside `backend/data` — `books.txt`, `members.txt` and `records.txt`.
The fields are separated by the `|` symbol, and each line is one record.

**What happens when the application is closed?**
When we close the program, `saveToFiles()` has already written everything to the
text files, so no data is lost.

**What happens when the application is opened again?**
`loadFromFiles()` reads the three files again and fills the memory, so the old data
appears on the screen exactly as it was.

**Which files do what?**
Frontend: one HTML page per screen (`index.html`, `books.html`, `members.html`,
`issue.html`, `return.html`, `records.html`), one CSS file (`css/style.css`) and
one JS file per page plus the shared `js/common.js`.

Backend: `library.h` is the blueprint, and every job has its own file —
`server.cpp` (the web server), `api.cpp` (the routes), `http.cpp` (the web
answers), `storage.cpp` (the text files), `books.cpp`, `members.cpp`,
`records.cpp`, `dashboard.cpp` and `helpers.cpp`.

**Where the digital techniques are?**
The bit level work sits in `digital.h` and `digital.cpp`: the status of a book
is one byte of flag bits instead of three booleans, the issue time is checked
as packed BCD, `%2B` in a web address is decoded with a shift and an OR, and
the digits are tested by comparing ASCII codes. The startup window prints a
self test of those routines. See `docs/digital_techniques.md`.

**Why did you write your own server?**
Because the project cannot use frameworks, so we used the C++ socket functions
to create a small HTTP server that serves the page and answers the requests.

**What is an API?**
It is a fixed list of addresses, like `/api/books`, that the frontend uses to ask
the backend to do a job. Here it is the list of the 11 requests written in `api.cpp`.