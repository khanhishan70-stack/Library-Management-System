// dashboard.js - the four numbers and the two small tables under them.

document.addEventListener("DOMContentLoaded", function () {
    setupLayout();

    async function loadStats() {
        try {
            const stats = await apiGet("/api/dashboard");

            byId("statTotalBooks").textContent = stats.totalBooks;
            byId("statAvailableBooks").textContent = stats.availableBooks;
            byId("statIssuedBooks").textContent = stats.issuedBooks;
            byId("statTotalMembers").textContent = stats.totalMembers;
        } catch (error) {
            showToast("Cannot reach library_server.exe. Is it running?", "error");
        }
    }

    async function loadTables() {
        const issuedBody = byId("issuedBooksBody");
        const activityBody = byId("recentActivityBody");

        let records = [];

        try {
            records = await apiGet("/api/records");
        } catch (error) {
            issuedBody.innerHTML =
                '<tr><td colspan="4" class="empty">Server not reachable.</td></tr>';
            activityBody.innerHTML =
                '<tr><td colspan="3" class="empty">Server not reachable.</td></tr>';
            return;
        }

        const stillOut = records.filter(function (record) {
            return record.status === "Issued";
        });

        issuedBody.innerHTML = stillOut.length === 0
            ? '<tr><td colspan="4" class="empty">No book is issued right now.</td></tr>'
            : stillOut.map(function (record) {
                return "<tr>"
                     + "<td>" + escapeHtml(record.bookName) + " ("
                     + escapeHtml(record.bookId) + ")</td>"
                     + "<td>" + escapeHtml(record.memberName) + "</td>"
                     + "<td>" + escapeHtml(record.issueDate) + "</td>"
                     + "<td>" + formatTime(record.issueTime) + "</td>"
                     + "</tr>";
            }).join("");

        // newest first, only the last five
        const recent = records.slice(-5).reverse();

        activityBody.innerHTML = recent.length === 0
            ? '<tr><td colspan="3" class="empty">No activity yet.</td></tr>'
            : recent.map(function (record) {
                return "<tr>"
                     + "<td>" + escapeHtml(record.recordId) + "</td>"
                     + "<td>" + escapeHtml(record.bookName) + "</td>"
                     + "<td>" + statusBadge(record.status) + "</td>"
                     + "</tr>";
            }).join("");
    }

    loadStats();
    loadTables();
});
