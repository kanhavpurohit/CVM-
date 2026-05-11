// CVM++ visualizer frontend.
// Editor → POST /api/analyze → render tokens, AST (D3), bytecode, output.

const EXAMPLES = {
    hello:
`// Arithmetic + variables
let x = 10;
let y = 32;
print x + y;

let a = true;
print a;
print 5 < 10;
`,
    loop:
`// Sum of 1..10
let i = 1;
let sum = 0;
while (i <= 10) {
    sum = sum + i;
    i = i + 1;
}
print sum;
`,
    fizzbuzz:
`// FizzBuzz with numeric codes (-1 Fizz, -2 Buzz, -3 FizzBuzz)
let i = 1;
while (i <= 15) {
    if ((i % 15) == 0) {
        print 0 - 3;
    } else if ((i % 3) == 0) {
        print 0 - 1;
    } else if ((i % 5) == 0) {
        print 0 - 2;
    } else {
        print i;
    }
    i = i + 1;
}
`,
    input:
`// Reads two ints from stdin, prints the larger.
// Put numbers separated by spaces in the stdin box.
input a;
input b;
if (a > b) {
    print a;
} else {
    print b;
}
`
};

const editor = CodeMirror.fromTextArea(document.getElementById("code"), {
    lineNumbers: true,
    mode: "text/x-csrc",
    theme: "monokai",
    indentUnit: 4,
    tabSize: 4
});
editor.setValue(EXAMPLES.loop);

// Example buttons
document.querySelectorAll(".example-buttons button").forEach(btn => {
    btn.addEventListener("click", () => {
        editor.setValue(EXAMPLES[btn.dataset.example] || "");
    });
});

// Tabs
document.querySelectorAll(".tab").forEach(t => {
    t.addEventListener("click", () => {
        document.querySelectorAll(".tab").forEach(x => x.classList.remove("active"));
        document.querySelectorAll(".tab-pane").forEach(x => x.classList.remove("active"));
        t.classList.add("active");
        document.querySelector(`.tab-pane[data-pane="${t.dataset.tab}"]`).classList.add("active");
        // Re-render AST on tab switch so it fits the now-visible container
        if (t.dataset.tab === "ast" && window.__lastAst) renderAst(window.__lastAst);
    });
});

// Run button
document.getElementById("run").addEventListener("click", runAnalysis);
document.addEventListener("keydown", e => {
    if ((e.ctrlKey || e.metaKey) && e.key === "Enter") runAnalysis();
});

async function runAnalysis() {
    const code = editor.getValue();
    const stdin = document.getElementById("stdin").value;
    const errBox = document.getElementById("error");
    errBox.classList.add("hidden");

    try {
        const res = await fetch("/api/analyze", {
            method: "POST",
            headers: {"Content-Type": "application/json"},
            body: JSON.stringify({ code, stdin })
        });
        const data = await res.json();
        render(data);
    } catch (e) {
        errBox.textContent = "Network error: " + e.message;
        errBox.classList.remove("hidden");
    }
}

function render(data) {
    const errBox = document.getElementById("error");
    if (data.error) {
        errBox.textContent = `[${data.error.stage}] ${data.error.message}`;
        errBox.classList.remove("hidden");
    } else {
        errBox.classList.add("hidden");
    }

    renderTokens(data.tokens || []);
    window.__lastAst = data.ast;
    renderAst(data.ast);
    renderBytecode(data.bytecode);
    document.getElementById("output").textContent = data.output || "(no output)";
}

function renderTokens(tokens) {
    const tbody = document.querySelector("#tokens-table tbody");
    tbody.innerHTML = "";
    tokens.forEach((tok, i) => {
        const tr = document.createElement("tr");
        tr.innerHTML = `<td>${i}</td><td>${escapeHtml(tok.type)}</td>` +
                       `<td>${escapeHtml(tok.value)}</td><td>${tok.line}</td>`;
        tbody.appendChild(tr);
    });
}

function renderBytecode(bc) {
    const varsDiv = document.getElementById("bytecode-vars");
    const tbody = document.querySelector("#bytecode-table tbody");
    tbody.innerHTML = "";
    if (!bc) { varsDiv.textContent = ""; return; }

    varsDiv.textContent = `vars: [${bc.vars.map((v, i) => `${i}:${v}`).join(", ")}]`;

    bc.code.forEach(ins => {
        const tr = document.createElement("tr");
        let operand = "";
        if ("operand" in ins) operand = String(ins.operand);
        else if ("slot" in ins) operand = `slot ${ins.slot}` + (ins.var ? ` (${ins.var})` : "");
        else if ("target" in ins) operand = `→ ${ins.target}`;
        const addr = String(ins.address).padStart(4, "0");
        tr.innerHTML = `<td>${addr}</td><td>${escapeHtml(ins.opcode)}</td>` +
                       `<td>${escapeHtml(operand)}</td>`;
        tbody.appendChild(tr);
    });
}

// ── AST rendering with D3 ──────────────────────────
function nodeLabel(n) {
    switch (n.type) {
        case "NumberLit":  return `Number ${n.value}`;
        case "BoolLit":    return `Bool ${n.value}`;
        case "Identifier": return `Ident ${n.name}`;
        case "BinaryOp":   return `BinaryOp (${n.op})`;
        case "UnaryOp":    return `UnaryOp (${n.op})`;
        case "VarDecl":    return `VarDecl ${n.name}`;
        case "Assign":     return `Assign ${n.name}`;
        case "Print":      return "Print";
        case "Input":      return `Input ${n.name}`;
        case "If":         return "If";
        case "While":      return "While";
        case "Block":      return "Block";
        case "ExprStmt":   return "ExprStmt";
        default:           return n.type || "?";
    }
}

function renderAst(astRoot) {
    const svg = d3.select("#ast-svg");
    svg.selectAll("*").remove();
    if (!astRoot) return;

    const container = document.getElementById("ast-container");
    const width  = container.clientWidth;
    const height = container.clientHeight;

    const root = d3.hierarchy(astRoot, n => n.children || []);
    const dx = 28;  // vertical spacing per node
    const dy = 170; // horizontal spacing per depth
    d3.tree().nodeSize([dx, dy])(root);

    // Bounding box of the laid-out tree (root.x is vertical because nodeSize is [dx, dy])
    let x0 = Infinity, x1 = -Infinity, y0 = Infinity, y1 = -Infinity;
    root.each(d => {
        if (d.x < x0) x0 = d.x;
        if (d.x > x1) x1 = d.x;
        if (d.y < y0) y0 = d.y;
        if (d.y > y1) y1 = d.y;
    });

    const treeW = (y1 - y0) + 200;
    const treeH = (x1 - x0) + 80;

    const g = svg
        .attr("viewBox", [0, 0, width, height])
        .append("g")
        .attr("transform", `translate(40, ${height/2 - (x0 + x1)/2})`);

    // Zoom + pan
    const zoom = d3.zoom().scaleExtent([0.2, 3]).on("zoom", e => {
        g.attr("transform", e.transform);
    });
    svg.call(zoom);
    svg.on("dblclick.zoom", null);
    svg.on("dblclick", () => svg.transition().duration(300)
        .call(zoom.transform, d3.zoomIdentity.translate(40, height/2 - (x0 + x1)/2)));

    // Links
    g.append("g")
        .selectAll("path")
        .data(root.links())
        .join("path")
        .attr("class", "link")
        .attr("d", d3.linkHorizontal().x(d => d.y).y(d => d.x));

    // Nodes
    const node = g.append("g")
        .selectAll("g")
        .data(root.descendants())
        .join("g")
        .attr("class", d => "node" + (d.children ? "" : " leaf"))
        .attr("transform", d => `translate(${d.y},${d.x})`);

    node.append("circle").attr("r", 5);

    node.append("text")
        .attr("dy", "0.32em")
        .attr("x", d => d.children ? -10 : 10)
        .attr("text-anchor", d => d.children ? "end" : "start")
        .text(d => nodeLabel(d.data));
}

function escapeHtml(s) {
    if (s === null || s === undefined) return "";
    return String(s).replace(/[&<>"]/g, c => (
        {"&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;"}[c]
    ));
}

// Auto-run on load
runAnalysis();
