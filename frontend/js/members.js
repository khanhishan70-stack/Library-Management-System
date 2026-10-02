// members.js - add a member, edit a name or phone, delete a member.

document.addEventListener("DOMContentLoaded", function () {
    setupLayout();
    setupPanelClosing();

    let members = [];

    async function loadMembers(filterText) {
        const body = byId("membersBody");

        try {
            members = await apiGet("/api/members");
        } catch (error) {
            body.innerHTML =
                '<tr><td colspan="6" class="empty">Cannot reach the backend.</td></tr>';
            return;
        }

        // keep the unfiltered list for the next-id preview and the edit popup
        byId("memberIdPreview").value = nextId(members, "M", 3);

        const filter = (filterText || "").toLowerCase();
        const list = filter === ""
            ? members
            : members.filter(function (member) {
                return (member.id + " " + member.name + " " + member.course)
                    .toLowerCase().indexOf(filter) !== -1;
            });

        body.innerHTML = list.length === 0
            ? '<tr><td colspan="6" class="empty">No member found.</td></tr>'
            : list.map(function (member) {
                return "<tr>"
                     + "<td><strong>" + escapeHtml(member.id) + "</strong></td>"
                     + "<td>" + escapeHtml(member.name) + "</td>"
                     + "<td>" + escapeHtml(member.course) + "</td>"
                     + "<td>" + escapeHtml(member.phone) + "</td>"
                     + "<td>" + escapeHtml(member.booksCount) + "</td>"
                     + '<td><button class="btn btn-sm btn-ghost" data-edit="'
                     + escapeHtml(member.id) + '">Edit</button>'
                     + '<button class="btn btn-sm btn-danger" data-del="'
                     + escapeHtml(member.id) + '">Delete</button></td>'
                     + "</tr>";
            }).join("");
    }

    // the phone rule is used twice, so it is worth having once
    function checkPhone(phone, errorId) {
        if (phone.length !== 10) {
            showFieldError(errorId, "Phone number must be exactly 10 digits.");
            return false;
        }
        if (!/^\d+$/.test(phone)) {
            showFieldError(errorId, "Phone number must contain digits only.");
            return false;
        }
        return true;
    }

    // ---------------------------------------------------------------- add

    byId("memberForm").addEventListener("submit", async function (event) {
        event.preventDefault();

        const name = readField("memberName");
        const course = readField("memberCourse");
        const phone = readField("memberPhone");

        showFieldError("errorMemberName", "");
        showFieldError("errorMemberCourse", "");
        showFieldError("errorMemberPhone", "");

        let valid = true;

        if (name === "") {
            showFieldError("errorMemberName", "Student name cannot be empty.");
            valid = false;
        }

        if (course === "") {
            showFieldError("errorMemberCourse", "Course cannot be empty.");
            valid = false;
        }

        if (!checkPhone(phone, "errorMemberPhone")) valid = false;

        if (!valid) {
            showToast("Please correct the highlighted fields.", "error");
            return;
        }

        const result = await apiPost("/api/members", {
            name: name,
            course: course,
            phone: phone
        }, "memberSubmit");

        if (result.ok) {
            byId("memberForm").reset();
            loadMembers("");
        }
    });

    byId("memberForm").addEventListener("reset", function () {
        showFieldError("errorMemberName", "");
        showFieldError("errorMemberCourse", "");
        showFieldError("errorMemberPhone", "");
    });

    // the course box uses the same available-books panel as the other pages
    setupPanel("memberCoursePanel", "memberCourse", function (id, name) {
        byId("memberCourse").value = name;
        showFieldError("errorMemberCourse", "");
    });

    // --------------------------------------------------------------- edit
    // Only the name and the phone can change. The id and the course stay
    // locked, otherwise the saved issue records would point at nothing.

    let editingId = "";

    function openEdit(id) {
        const member = members.filter(function (item) { return item.id === id; })[0];

        if (!member) {
            showToast("Member not found. Please refresh the page.", "error");
            return;
        }

        editingId = id;

        byId("editMemberIdText").textContent = member.id;
        byId("editMemberCourseText").textContent = member.course;
        byId("editMemberName").value = member.name;
        byId("editMemberPhone").value = member.phone;

        showFieldError("errorEditMemberName", "");
        showFieldError("errorEditMemberPhone", "");

        byId("editMemberModal").classList.add("open");
        byId("editMemberName").focus();
    }

    function closeEdit() {
        byId("editMemberModal").classList.remove("open");
        editingId = "";
    }

    byId("editMemberSave").addEventListener("click", async function () {
        const name = readField("editMemberName");
        const phone = readField("editMemberPhone");

        showFieldError("errorEditMemberName", "");
        showFieldError("errorEditMemberPhone", "");

        let valid = true;

        if (name === "") {
            showFieldError("errorEditMemberName", "Student name cannot be empty.");
            valid = false;
        }

        if (!checkPhone(phone, "errorEditMemberPhone")) valid = false;

        if (!valid) {
            showToast("Please correct the highlighted fields.", "error");
            return;
        }

        const result = await apiPost("/api/members/update", {
            id: editingId,
            name: name,
            phone: phone
        }, "editMemberSave");

        if (result.ok) {
            closeEdit();
            loadMembers(readField("memberSearch"));
        }
    });

    byId("editMemberCancel").addEventListener("click", closeEdit);
    byId("editMemberBackdrop").addEventListener("click", closeEdit);

    document.addEventListener("keydown", function (event) {
        if (event.key === "Escape") closeEdit();
    });

    // ------------------------------------------------------- edit / delete

    byId("membersBody").addEventListener("click", async function (event) {
        const editButton = event.target.closest("[data-edit]");
        if (editButton) {
            openEdit(editButton.getAttribute("data-edit"));
            return;
        }

        const deleteButton = event.target.closest("[data-del]");
        if (!deleteButton) return;

        const id = deleteButton.getAttribute("data-del");

        if (!confirm("Delete the member " + id + "? This cannot be undone.")) return;

        const result = await apiDelete("/api/members?id=" + encodeURIComponent(id));
        if (result.ok) loadMembers(readField("memberSearch"));
    });

    // ----------------------------------------------------------- filtering

    byId("memberSearch").addEventListener("input", function () {
        loadMembers(readField("memberSearch"));
    });

    loadMembers("");
});
