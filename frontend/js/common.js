// common.js - the parts every page needs: talking to the server, small helpers,
// the sidebar, the theme button and the "available books" panel.

// ---------------------------------------------------------------- server side

function byId(id) {
    return document.getElementById(id);
}

async function apiGet(url) {
    const response = await fetch(url);
    if (!response.ok) throw new Error("bad response");
    return response.json();
}

async function apiPost(url, data, buttonId) {
    const button = buttonId ? byId(buttonId) : null;
    const label = button ? button.textContent : "";

    if (button) {
        button.disabled = true;
        button.textContent = "Please wait...";
    }

    try {
        const response = await fetch(url, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(data)
        });

        const result = await response.json();
        showToast(result.message, result.ok ? "success" : "error");

        // issuing or taking back a book changes how many are pending
        if (result.ok) refreshReminderBadge();

        return result;
    } catch (error) {
        showToast("Cannot reach library_server.exe. Is it running?", "error");
        return { ok: false };
    } finally {
        if (button) {
            button.disabled = false;
            button.textContent = label;
        }
    }
}

async function apiDelete(url) {
    try {
        const response = await fetch(url, { method: "DELETE" });
        const result = await response.json();
        showToast(result.message, result.ok ? "success" : "error");
        if (result.ok) refreshReminderBadge();
        return result;
    } catch (error) {
        showToast("Cannot reach library_server.exe. Is it running?", "error");
        return { ok: false };
    }
}

// ------------------------------------------------------------------- messages

function showToast(message, type) {
    const box = byId("toastContainer");
    const toast = document.createElement("div");
    toast.className = "toast " + type;
    toast.textContent = message;

    const close = document.createElement("button");
    close.className = "toast-close";
    close.textContent = "×";
    close.onclick = function () {
        box.removeChild(toast);
    };

    toast.appendChild(close);
    box.appendChild(toast);

    setTimeout(function () {
        if (toast.parentNode) toast.parentNode.removeChild(toast);
    }, 3500);
}

function showFieldError(id, message) {
    byId(id).textContent = message;
}

// ---------------------------------------------------------------- text helpers

// Text coming from a text file goes into innerHTML, so anything that looks
// like a tag has to be neutralised first.
function escapeHtml(text) {
    return String(text)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;");
}

function statusBadge(status) {
    const classes = {
        "Available": "available",
        "Partly Issued": "partly",
        "Issued": "issued",
        "Returned": "returned"
    };

    const style = classes[status] || "returned";
    return '<span class="badge badge-' + style + '">' + escapeHtml(status) + "</span>";
}

function readField(id) {
    return byId(id).value.trim();
}

// --------------------------------------------------------------- dates & times

function todayForInput() {
    const now = new Date();
    const month = String(now.getMonth() + 1).padStart(2, "0");
    const day = String(now.getDate()).padStart(2, "0");
    return now.getFullYear() + "-" + month + "-" + day;
}

function nowForInput() {
    const now = new Date();
    return String(now.getHours()).padStart(2, "0") + ":"
         + String(now.getMinutes()).padStart(2, "0");
}

// the date input gives YYYY-MM-DD, the files use DD-MM-YYYY
function formatDate(value) {
    if (!value) return "-";
    const parts = value.split("-");
    if (parts.length !== 3) return value;
    return parts[2] + "-" + parts[1] + "-" + parts[0];
}

// 14:35 is stored like that but shown as 2:35 PM
function formatTime(value) {
    if (!value || value === "-") return "-";

    const parts = value.split(":");
    if (parts.length !== 2) return escapeHtml(value);

    let hours = Number(parts[0]);
    if (isNaN(hours) || hours < 0 || hours > 23) return escapeHtml(value);

    const ampm = hours >= 12 ? "PM" : "AM";
    hours = hours % 12;
    if (hours === 0) hours = 12;

    return hours + ":" + parts[1] + " " + ampm;
}

// ------------------------------------------------------------------- the shell

// Every page has the same sidebar and top bar, so the bits that change
// between pages are read from body data-page.
const PAGE_INFO = {
    dashboard: ["Dashboard", "Overview of the library"],
    books:     ["Books", "Add, search and delete books"],
    members:   ["Members", "Add and view library members"],
    issue:     ["Issue Book", "Give a book to a member"],
    "return":  ["Return Book", "Take a book back from a member"],
    records:   ["Records", "History of every issue and return"]
};

function setupLayout() {
    const page = document.body.getAttribute("data-page");

    const info = PAGE_INFO[page];
    if (info) {
        byId("pageTitle").textContent = info[0];
        byId("pageSubtitle").textContent = info[1];
    }

    const link = document.querySelector('.nav-link[data-page="' + page + '"]');
    if (link) link.classList.add("active");

    byId("menuBtn").addEventListener("click", function () {
        byId("sidebar").classList.toggle("open");
    });

    // dark is the default, light is remembered in localStorage
    const themeBtn = byId("themeBtn");

    if (localStorage.getItem("lmsTheme") === "light") {
        document.body.classList.add("light");
        themeBtn.textContent = "☀️";
    }

    themeBtn.addEventListener("click", function () {
        const light = document.body.classList.toggle("light");
        themeBtn.textContent = light ? "☀️" : "🌙";
        localStorage.setItem("lmsTheme", light ? "light" : "dark");
    });

    const clock = byId("clock");
    function tick() {
        const now = new Date();
        clock.textContent = [now.getHours(), now.getMinutes(), now.getSeconds()]
            .map(function (part) { return String(part).padStart(2, "0"); })
            .join(":");
    }

    tick();
    setInterval(tick, 1000);
}

// ---------------------------------------------------- the available-books panel
// Clicking a search box drops this list down. Only books that still have a
// copy on the shelf are shown, and each one says how many are left.

let panelBooks = [];

async function loadPanelBooks() {
    try {
        panelBooks = await apiGet("/api/books");
    } catch (error) {
        // keep whatever we had, an empty list is worse than a stale one
    }
}

function drawPanel(panelId, filterText) {
    const list = byId(panelId + "List");
    const filter = (filterText || "").toLowerCase();

    const available = panelBooks.filter(function (book) {
        if (book.availableCopies <= 0) return false;
        if (filter === "") return true;
        return (book.id + " " + book.name + " " + book.category)
            .toLowerCase().indexOf(filter) !== -1;
    });

    if (available.length === 0) {
        list.innerHTML = '<li class="panel-empty">No available book found.</li>';
        return;
    }

    list.innerHTML = available.map(function (book) {
        return '<li data-id="' + escapeHtml(book.id) + '" data-name="'
             + escapeHtml(book.name) + '">'
             + '<span class="panel-id">' + escapeHtml(book.id) + "</span>"
             + '<span class="panel-name">' + escapeHtml(book.name) + "</span>"
             + '<span class="panel-count">' + book.availableCopies + " left</span>"
             + "</li>";
    }).join("");
}

async function openPanel(panelId) {
    byId(panelId).classList.add("open");
    await loadPanelBooks();
    drawPanel(panelId, byId(panelId + "Search").value.trim());
}

function closePanel(panelId) {
    if (panelId) {
        byId(panelId).classList.remove("open");
        return;
    }

    document.querySelectorAll(".book-panel").forEach(function (panel) {
        panel.classList.remove("open");
    });
}

// Wires up one panel: the input that opens it, the small filter box inside
// it, and what happens when a book is clicked. Every page that uses the
// panel calls this once.
function setupPanel(panelId, inputId, onPick) {
    const input = byId(inputId);

    // mousedown so the panel is already open before the click lands
    input.addEventListener("mousedown", function () { openPanel(panelId); });
    input.addEventListener("click", function () { openPanel(panelId); });

    byId(panelId + "Search").addEventListener("input", function () {
        drawPanel(panelId, byId(panelId + "Search").value.trim());
    });

    byId(panelId + "List").addEventListener("click", function (event) {
        const item = event.target.closest("li[data-id]");
        if (!item) return;

        onPick(item.getAttribute("data-id"), item.getAttribute("data-name"));
        closePanel(panelId);
    });
}

function setupPanelClosing() {
    document.addEventListener("click", function (event) {
        if (!event.target.closest(".search-wrap")) closePanel();
    });

    document.addEventListener("keydown", function (event) {
        if (event.key === "Escape") closePanel();
    });
}

// --------------------------------------------------------- suggestion boxes
// <datalist> gives the little dropdown of suggestions while typing an id.

function fillBookIds(books) {
    byId("bookIdList").innerHTML = books.map(function (book) {
        return '<option value="' + escapeHtml(book.id) + '">'
             + escapeHtml(book.name) + "</option>";
    }).join("");
}

function fillMemberIds(members) {
    byId("memberIdList").innerHTML = members.map(function (member) {
        return '<option value="' + escapeHtml(member.id) + '">'
             + escapeHtml(member.name) + "</option>";
    }).join("");
}

// The next id the backend will hand out. It counts only ids starting with
// the letter it uses, the same way the backend does.
function nextId(list, letter, width) {
    let highest = 0;

    list.forEach(function (item) {
        if (!item.id || item.id.charAt(0) !== letter) return;

        const number = parseInt(item.id.replace(/[^0-9]/g, ""), 10);
        if (!isNaN(number) && number > highest) highest = number;
    });

    return letter + String(highest + 1).padStart(width, "0");
}

// ======================================================= the pending-return popup
// A book that has not come back yet is a book the librarian has to chase. This
// part adds a bell to the top bar and the popup it opens, showing the name,
// id and phone number of every member who is holding a book, plus how many
// days it has been with them.

const REMINDER_DAY_KEY = "lmsReminderDay";    // last day the popup opened itself
const REMINDER_SOUND_KEY = "lmsReminderSound"; // "on" or "off"

let pendingCache = [];
let reminderAudio = null;

// --------------------------------------------------------------- the sound
// The note is made with the Web Audio API instead of an audio file, so the
// project needs no extra asset and there is nothing to download.

function getReminderAudio() {
    if (reminderAudio) return reminderAudio;

    const AudioCtor = window.AudioContext || window.webkitAudioContext;
    if (!AudioCtor) return null;

    try {
        reminderAudio = new AudioCtor();
    } catch (error) {
        return null;
    }

    return reminderAudio;
}

function reminderSoundOn() {
    return localStorage.getItem(REMINDER_SOUND_KEY) !== "off";
}

// three short notes, like a telephone ringing
function ringTone(context) {
    const notes = [880, 660, 880];
    let start = context.currentTime;

    notes.forEach(function (frequency) {
        const oscillator = context.createOscillator();
        const gain = context.createGain();

        oscillator.type = "sine";
        oscillator.frequency.value = frequency;

        // fade in and out so the note does not click
        gain.gain.setValueAtTime(0.0001, start);
        gain.gain.exponentialRampToValueAtTime(0.3, start + 0.02);
        gain.gain.exponentialRampToValueAtTime(0.0001, start + 0.16);

        oscillator.connect(gain);
        gain.connect(context.destination);
        oscillator.start(start);
        oscillator.stop(start + 0.18);

        start += 0.22;
    });
}

function playReminderSound() {
    if (!reminderSoundOn()) return;

    const context = getReminderAudio();
    if (!context) return;

    // a browser keeps quiet until the visitor has touched the page, so the
    // note is played on the first click or key press instead of being lost
    if (context.state === "suspended") {
        const playWhenAllowed = function () {
            context.resume().then(function () {
                ringTone(context);
            }).catch(function () {});

            document.removeEventListener("pointerdown", playWhenAllowed, true);
            document.removeEventListener("keydown", playWhenAllowed, true);
        };

        document.addEventListener("pointerdown", playWhenAllowed, true);
        document.addEventListener("keydown", playWhenAllowed, true);
        return;
    }

    ringTone(context);
}

// ------------------------------------------------------------ the popup itself

function buildReminderMarkup() {
    if (byId("reminderModal")) return;

    const host = document.createElement("div");
    host.innerHTML =
        '<div class="modal" id="reminderModal">' +
          '<div class="modal-backdrop" id="reminderBackdrop"></div>' +
          '<div class="modal-box wide">' +
            '<div class="reminder-head">' +
              '<h3 class="modal-title">🔔 Pending Returns</h3>' +
              '<button type="button" class="sound-btn" id="reminderSoundBtn" ' +
                      'title="Turn the alert sound on or off">🔊</button>' +
            '</div>' +
            '<div id="reminderSummary"></div>' +
            '<div class="table-scroll">' +
              '<table class="reminder-table">' +
                '<thead><tr>' +
                  '<th>Book</th><th>Member</th><th>ID</th>' +
                  '<th>Phone</th><th>Issued</th><th>Days Out</th>' +
                '</tr></thead>' +
                '<tbody id="reminderBody"></tbody>' +
              '</table>' +
            '</div>' +
            '<div class="modal-buttons">' +
              '<button type="button" class="btn btn-primary" id="reminderClose">Close</button>' +
            '</div>' +
          '</div>' +
        '</div>';

    document.body.appendChild(host.firstChild);

    byId("reminderClose").addEventListener("click", closeReminderPopup);
    byId("reminderBackdrop").addEventListener("click", closeReminderPopup);

    document.addEventListener("keydown", function (event) {
        if (event.key === "Escape") closeReminderPopup();
    });

    const soundBtn = byId("reminderSoundBtn");
    soundBtn.textContent = reminderSoundOn() ? "🔊" : "🔇";

    soundBtn.addEventListener("click", function () {
        const on = !reminderSoundOn();
        localStorage.setItem(REMINDER_SOUND_KEY, on ? "on" : "off");
        soundBtn.textContent = on ? "🔊" : "🔇";
        showToast(on ? "Alert sound is on." : "Alert sound is off.", "success");
        if (on) playReminderSound();
    });
}

// one row per book that is still out
function reminderRow(item) {
    let daysText = "-";
    let daysClass = "";

    if (item.daysOut === 0) {
        daysText = "today";
    } else if (item.daysOut > 0) {
        daysText = item.daysOut + (item.daysOut === 1 ? " day" : " days");
        daysClass = item.daysOut >= 14 ? "late" : "soon";
    }

    // a dash when the member was deleted after taking the book
    const phone = (item.phone && item.phone !== "-")
        ? '<a class="phone-link" href="tel:' + escapeHtml(item.phone) + '">'
          + escapeHtml(item.phone) + "</a>"
        : "-";

    // an old record can point at an id that is no longer in members.txt
    const who = item.memberName ? escapeHtml(item.memberName) : "deleted member";

    const daysStyle = daysClass ? ' class="days-cell ' + daysClass + '"' : "";

    return "<tr>"
         + "<td>" + escapeHtml(item.bookName) + "<br><small>"
           + escapeHtml(item.bookId) + "</small></td>"
         + "<td>" + who + "</td>"
         + "<td>" + escapeHtml(item.memberId) + "</td>"
         + "<td>" + phone + "</td>"
         + "<td>" + escapeHtml(item.issueDate) + "<br><small>"
           + formatTime(item.issueTime) + "</small></td>"
         + "<td" + daysStyle + ">" + escapeHtml(daysText) + "</td>"
         + "</tr>";
}

function drawReminder(items) {
    const body = byId("reminderBody");
    const summary = byId("reminderSummary");

    if (items.length === 0) {
        summary.innerHTML = "";
        body.innerHTML =
            '<tr><td colspan="6" class="reminder-empty">'
            + '<span class="tick">✅</span>'
            + "Every book is back. Nothing to chase."
            + "</td></tr>";
        return;
    }

    const books = items.length;
    const members = {};
    let worst = 0;

    items.forEach(function (item) {
        members[item.memberId] = true;
        if (item.daysOut > worst) worst = item.daysOut;
    });

    const memberCount = Object.keys(members).length;
    const worstText = worst > 0
        ? " and " + worst + (worst === 1 ? " day" : " days") + " late"
        : "";

    summary.innerHTML = '<div class="reminder-summary">'
        + '<span class="pulse"></span><span>'
        + escapeHtml(books) + (books === 1 ? " book is" : " books are")
        + " still not returned by " + escapeHtml(memberCount)
        + (memberCount === 1 ? " member" : " members") + escapeHtml(worstText) + "."
        + "</span></div>";

    body.innerHTML = items.map(reminderRow).join("");
}

async function loadPendingReturns() {
    try {
        pendingCache = await apiGet("/api/pending");
    } catch (error) {
        pendingCache = [];
    }

    return pendingCache;
}

async function openReminderPopup(withSound) {
    if (withSound) playReminderSound();

    const items = await loadPendingReturns();
    drawReminder(items);
    byId("reminderModal").classList.add("open");

    const bell = byId("reminderBtn");
    if (bell && items.length > 0) {
        bell.classList.remove("ringing");
        void bell.offsetWidth;          // restart the shake animation
        bell.classList.add("ringing");
    }
}

function closeReminderPopup() {
    const modal = byId("reminderModal");
    if (modal) modal.classList.remove("open");
}

// keeps the red number on the bell equal to the books that are still out
async function refreshReminderBadge() {
    const items = await loadPendingReturns();

    const badge = byId("reminderCount");
    if (!badge) return;

    if (items.length > 0) {
        badge.textContent = String(items.length);
        badge.classList.remove("hidden");
    } else {
        badge.classList.add("hidden");
    }
}

function addReminderBell() {
    const bar = document.querySelector(".topbar-right");
    if (!bar || byId("reminderBtn")) return;

    const bell = document.createElement("button");
    bell.type = "button";
    bell.className = "bell-btn";
    bell.id = "reminderBtn";
    bell.title = "Books that have not come back yet";
    bell.innerHTML = '<span aria-hidden="true">🔔</span>'
                   + '<span class="bell-count hidden" id="reminderCount">0</span>';

    bell.addEventListener("click", function () {
        openReminderPopup(true);
    });

    bar.insertBefore(bell, bar.firstChild);
}

// On the dashboard the popup opens by itself once a day when something is
// still out. On the other pages the bell is waiting to be clicked.
function autoOpenReminderOnce() {
    if (document.body.getAttribute("data-page") !== "dashboard") return;

    const today = todayForInput();
    if (localStorage.getItem(REMINDER_DAY_KEY) === today) return;

    loadPendingReturns().then(function (items) {
        if (items.length === 0) return;

        localStorage.setItem(REMINDER_DAY_KEY, today);
        openReminderPopup(true);
    });
}

document.addEventListener("DOMContentLoaded", function () {
    buildReminderMarkup();
    addReminderBell();
    refreshReminderBadge();
    autoOpenReminderOnce();
});

