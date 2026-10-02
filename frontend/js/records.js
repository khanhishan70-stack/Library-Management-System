// records.js - the full history of every issue and return.

document.addEventListener("DOMContentLoaded", function () {
    setupLayout();

    async function loadRecords() {
        const body = byId("recordsBody");
        let records = [];

        try {
            records = await apiGet("/api/records");
        } catch (error) {
            body.innerHTML =
                '<tr><td colspan="9" class="empty">Cannot reach the backend.</td></tr>';
            return;
        }

        body.innerHTML = records.length === 0
            ? '<tr><td colspan="9" class="empty">No record found.</td></tr>'
            : records.map(function (record) {
                return "<tr>"
                     + "<td><strong>" + escapeHtml(record.recordId) + "</strong></td>"
                     + "<td>" + escapeHtml(record.bookId) + "</td>"
                     + "<td>" + escapeHtml(record.bookName) + "</td>"
                     + "<td>" + escapeHtml(record.memberId) + "</td>"
                     + "<td>" + escapeHtml(record.memberName) + "</td>"
                     + "<td>" + escapeHtml(record.issueDate) + "</td>"
                     + "<td>" + formatTime(record.issueTime) + "</td>"
                     + "<td>" + escapeHtml(record.returnDate) + "</td>"
                     + "<td>" + statusBadge(record.status) + "</td>"
                     + "</tr>";
            }).join("");
    }

    byId("refreshRecordsBtn").addEventListener("click", function () {
        loadRecords();
        showToast("Records refreshed.", "info");
    });

    loadRecords();
});
