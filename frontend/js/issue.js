// issue.js - hand a book to a member.

document.addEventListener("DOMContentLoaded", function () {
    setupLayout();
    setupPanelClosing();

    byId("issueDate").value = todayForInput();
    byId("issueTime").value = nowForInput();

    // The time box starts with the current clock time, so normally the
    // librarian never touches it. Clicking an empty one fills it again.
    byId("issueTime").addEventListener("focus", function () {
        if (byId("issueTime").value === "") {
            byId("issueTime").value = nowForInput();
        }
    });

    byId("issueDate").addEventListener("focus", function () {
        if (byId("issueDate").value === "") {
            byId("issueDate").value = todayForInput();
        }
    });

    // the member id box suggests the ids that exist
    async function loadMemberIds() {
        try {
            fillMemberIds(await apiGet("/api/members"));
        } catch (error) {
            // suggestions are not important enough to shout about
        }
    }

    setupPanel("issueBookPanel", "issueBookId", function (id) {
        byId("issueBookId").value = id;
    });

    byId("issueBookToggle").addEventListener("click", function () {
        openPanel("issueBookPanel");
    });

    byId("issueForm").addEventListener("submit", async function (event) {
        event.preventDefault();

        const bookId = readField("issueBookId");
        const memberId = readField("issueMemberId");
        const date = readField("issueDate");
        const time = readField("issueTime");

        showFieldError("errorIssueBookId", "");
        showFieldError("errorIssueMemberId", "");
        showFieldError("errorIssueDate", "");
        showFieldError("errorIssueTime", "");

        let valid = true;

        if (bookId === "") {
            showFieldError("errorIssueBookId", "Book ID cannot be empty.");
            valid = false;
        }

        if (memberId === "") {
            showFieldError("errorIssueMemberId", "Member ID cannot be empty.");
            valid = false;
        }

        if (date === "") {
            showFieldError("errorIssueDate", "Please choose the issue date.");
            valid = false;
        }

        if (time !== "" && !/^\d{2}:\d{2}$/.test(time)) {
            showFieldError("errorIssueTime", "Please choose a valid time.");
            valid = false;
        }

        if (!valid) {
            showToast("Please fill all the fields.", "error");
            return;
        }

        const result = await apiPost("/api/issue", {
            bookId: bookId,
            memberId: memberId,
            issueDate: formatDate(date),
            issueTime: time === "" ? nowForInput() : time
        }, "issueSubmit");

        if (result.ok) {
            byId("issueForm").reset();
            byId("issueDate").value = todayForInput();
            byId("issueTime").value = nowForInput();
            loadMemberIds();
        }
    });

    byId("issueForm").addEventListener("reset", function () {
        showFieldError("errorIssueBookId", "");
        showFieldError("errorIssueMemberId", "");
        showFieldError("errorIssueDate", "");
        showFieldError("errorIssueTime", "");
    });

    loadMemberIds();
});
