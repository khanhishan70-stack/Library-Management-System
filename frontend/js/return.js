// return.js - take a book back from a member.

document.addEventListener("DOMContentLoaded", function () {
    setupLayout();

    byId("returnDate").value = todayForInput();

    // suggest the book ids that exist
    async function loadBookIds() {
        try {
            fillBookIds(await apiGet("/api/books"));
        } catch (error) {
            // suggestions are not important enough to shout about
        }
    }

    byId("returnForm").addEventListener("submit", async function (event) {
        event.preventDefault();

        const bookId = readField("returnBookId");
        const date = readField("returnDate");

        showFieldError("errorReturnBookId", "");
        showFieldError("errorReturnDate", "");

        let valid = true;

        if (bookId === "") {
            showFieldError("errorReturnBookId", "Book ID cannot be empty.");
            valid = false;
        }

        if (date === "") {
            showFieldError("errorReturnDate", "Please choose the return date.");
            valid = false;
        }

        if (!valid) {
            showToast("Please fill all the fields.", "error");
            return;
        }

        const result = await apiPost("/api/return", {
            bookId: bookId,
            returnDate: formatDate(date)
        }, "returnSubmit");

        if (result.ok) {
            byId("returnForm").reset();
            byId("returnDate").value = todayForInput();
            loadBookIds();
        }
    });

    byId("returnForm").addEventListener("reset", function () {
        showFieldError("errorReturnBookId", "");
        showFieldError("errorReturnDate", "");
    });

    loadBookIds();
});
