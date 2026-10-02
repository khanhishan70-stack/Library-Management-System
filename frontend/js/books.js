// books.js - add a book, search the list, delete a book.

document.addEventListener("DOMContentLoaded", function () {
    setupLayout();
    setupPanelClosing();

    let books = [];

    // "Copies Left" is the number the librarian actually cares about.
    function copiesCell(book) {
        const left = book.availableCopies;

        if (left === 0) {
            return '<td><span class="copies-cell copies-none">0 (none left)</span></td>';
        }
        if (left === 1) {
            return '<td><span class="copies-cell copies-low">1</span></td>';
        }
        return '<td><span class="copies-cell">' + left + "</span></td>";
    }

    async function loadBooks(searchText) {
        const body = byId("booksBody");
        const url = searchText
            ? "/api/search?q=" + encodeURIComponent(searchText)
            : "/api/books";

        try {
            books = await apiGet(url);
        } catch (error) {
            body.innerHTML =
                '<tr><td colspan="8" class="empty">Cannot reach the backend.</td></tr>';
            return;
        }

        // the next-id preview needs the unfiltered list, but the table shows
        // whatever the search found
        if (!searchText) {
            loadAllForPreview();
        }

        body.innerHTML = books.length === 0
            ? '<tr><td colspan="8" class="empty">No book found.</td></tr>'
            : books.map(function (book) {
                return "<tr>"
                     + "<td><strong>" + escapeHtml(book.id) + "</strong></td>"
                     + "<td>" + escapeHtml(book.name) + "</td>"
                     + "<td>" + escapeHtml(book.category) + "</td>"
                     + "<td>" + escapeHtml(book.year) + "</td>"
                     + copiesCell(book)
                     + "<td>" + escapeHtml(book.totalCopies) + "</td>"
                     + "<td>" + statusBadge(book.status) + "</td>"
                     + '<td><button class="btn btn-sm btn-danger" data-id="'
                     + escapeHtml(book.id) + '">Delete</button></td>'
                     + "</tr>";
            }).join("");
    }

    // the unfiltered list, used for the next book id and for the panel
    async function loadAllForPreview() {
        try {
            const all = await apiGet("/api/books");
            byId("bookIdPreview").value = nextId(all, "B", 3);
        } catch (error) {
            // leave the preview as it is
        }
    }

    function refresh() {
        loadBooks(readField("bookSearch"));
    }

    // ---------------------------------------------------------------- add

    byId("bookForm").addEventListener("submit", async function (event) {
        event.preventDefault();

        const name = readField("bookName");
        const year = readField("bookYear");
        const copies = readField("bookCopies");

        showFieldError("errorBookName", "");
        showFieldError("errorBookYear", "");
        showFieldError("errorBookCopies", "");

        const yearNumber = Number(year);
        const copiesNumber = Number(copies);

        let valid = true;

        if (name === "") {
            showFieldError("errorBookName", "Book name cannot be empty.");
            valid = false;
        }

        if (year === "" || isNaN(yearNumber) || yearNumber < 1000 || yearNumber > 2100) {
            showFieldError("errorBookYear", "Please enter a valid year, like 2024.");
            valid = false;
        }

        if (copies === "" || isNaN(copiesNumber)
            || copiesNumber < 1 || copiesNumber > 999
            || Math.floor(copiesNumber) !== copiesNumber) {
            showFieldError("errorBookCopies", "Copies must be a whole number, 1 to 999.");
            valid = false;
        }

        if (!valid) {
            showToast("Please correct the highlighted fields.", "error");
            return;
        }

        // the id and the category are not sent, the backend makes them
        const result = await apiPost("/api/books", {
            name: name,
            year: yearNumber,
            copies: copiesNumber
        }, "bookSubmit");

        if (result.ok) {
            byId("bookForm").reset();
            byId("bookCopies").value = 1;
            refresh();
            loadAllForPreview();
        }
    });

    byId("bookForm").addEventListener("reset", function () {
        showFieldError("errorBookName", "");
        showFieldError("errorBookYear", "");
        showFieldError("errorBookCopies", "");
    });

    // ------------------------------------------------------------- search

    function runSearch() {
        closePanel();
        loadBooks(readField("bookSearch"));
    }

    byId("bookSearchBtn").addEventListener("click", runSearch);

    byId("bookSearch").addEventListener("keydown", function (event) {
        if (event.key === "Enter") {
            event.preventDefault();
            runSearch();
        }
    });

    // clicking the search box opens the list of available books
    setupPanel("bookPanel", "bookSearch", function (id, name) {
        byId("bookSearch").value = name;
        loadBooks(name);
    });

    // ------------------------------------------------------------- delete

    byId("booksBody").addEventListener("click", async function (event) {
        const button = event.target.closest("[data-id]");
        if (!button || !button.classList.contains("btn-danger")) return;

        const id = button.getAttribute("data-id");
        const book = books.filter(function (item) { return item.id === id; })[0];
        const title = book ? book.name : id;

        if (!confirm('Delete "' + title + '"? This cannot be undone.')) return;

        const result = await apiDelete("/api/books?id=" + encodeURIComponent(id));
        if (result.ok) refresh();
    });

    // --------------------------------------------------------------- start

    loadBooks("");
    loadAllForPreview();
});
