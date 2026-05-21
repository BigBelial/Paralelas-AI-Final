// Cliente web del scaffold de Mancala/Kalah(6,4).
// Conoce el orden canónico del tablero: 14 enteros, índices 0..5 son los
// hoyos del jugador 0, índice 6 su kalaha; índices 7..12 los hoyos del
// jugador 1 y 13 su kalaha.

const API_BASE = window.MANCALA_API_BASE || "http://localhost:8000";

const initialBoard = () => [4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4, 4, 4, 0];

let board = initialBoard();

function render() {
    document.querySelectorAll("[data-idx]").forEach((el) => {
        const idx = Number(el.dataset.idx);
        el.textContent = String(board[idx]);
    });
}

async function pedirJugada() {
    const algo = document.getElementById("algo").value;
    const budget = Number(document.getElementById("budget").value);
    const threads = Number(document.getElementById("threads").value);

    const payload = {
        board,
        side: 0,
        algo,
        threads,
    };
    if (algo === "alphabeta") payload.depth = budget;
    else payload.simulations = budget;

    const out = document.getElementById("result-json");
    out.textContent = "Pidiendo jugada...";

    try {
        const resp = await fetch(`${API_BASE}/move`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(payload),
        });
        const json = await resp.json();
        out.textContent = `HTTP ${resp.status}\n` + JSON.stringify(json, null, 2);
    } catch (err) {
        out.textContent = `Error de red: ${err}`;
    }
}

document.getElementById("btn-move").addEventListener("click", pedirJugada);
document.getElementById("btn-reset").addEventListener("click", () => {
    board = initialBoard();
    render();
    document.getElementById("result-json").textContent = "(sin datos aún)";
});

render();
