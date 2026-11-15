const API_BASE = "http://localhost:8080/api";

const refreshBtn = document.getElementById("refreshBtn");
const searchInput = document.getElementById("searchInput");
const computerIdInput = document.getElementById("computerId");
const processTable = document.getElementById("processTable");
const totalProcesses = document.getElementById("totalProcesses");
const computerName = document.getElementById("computerName");
const apiStatus = document.getElementById("apiStatus");
const lastUpdated = document.getElementById("lastUpdated");
const recordCount = document.getElementById("recordCount");
const message = document.getElementById("message");

const averageCpu = document.getElementById("averageCpu");
const totalMemory = document.getElementById("totalMemory");
const metricsStatus = document.getElementById("metricsStatus");
const metricsUpdated = document.getElementById("metricsUpdated");

let processes = [];
let metricsByPid = new Map();

function showMessage(text, isError = false) {
    message.textContent = text;
    message.classList.toggle("error", isError);
}

function makeCell(value) {
    const cell = document.createElement("td");
    cell.textContent = value;
    return cell;
}

function renderProcesses() {
    const query = searchInput.value.trim().toLowerCase();

    const filtered = processes.filter(process =>
        String(process.pid).includes(query) ||
        process.process_name.toLowerCase().includes(query) ||
        process.computer_name.toLowerCase().includes(query)
    );

    processTable.replaceChildren();

    if (filtered.length === 0) {
        const row = document.createElement("tr");
        const cell = makeCell("No matching processes found.");
        cell.colSpan = 6;
        row.appendChild(cell);
        processTable.appendChild(row);
    } else {
        const fragment = document.createDocumentFragment();

        for (const process of filtered) {
            const row = document.createElement("tr");
            const metric = metricsByPid.get(Number(process.pid));

            row.append(
                makeCell(process.pid),
                makeCell(process.process_name),
                makeCell(process.computer_name),
                makeCell(metric
                    ? `${Number(metric.cpu_usage_percent).toFixed(2)}%`
                    : "N/A"),
                makeCell(metric
                    ? `${Number(metric.memory_usage_mb).toFixed(2)} MB`
                    : "N/A"),
                makeCell(metric?.recorded_at ?? "N/A")
            );

            fragment.appendChild(row);
        }

        processTable.appendChild(fragment);
    }

    recordCount.textContent =
        `${filtered.length} of ${processes.length} records`;
}

async function loadProcesses() {
    const computerId = Number(computerIdInput.value);

    if (!Number.isInteger(computerId) || computerId <= 0) {
        showMessage("Enter a valid positive computer ID.", true);
        return;
    }

    refreshBtn.disabled = true;

    try {
        const [processResponse, metricsResponse] = await Promise.all([
            fetch(
                `${API_BASE}/processes?computer_id=${encodeURIComponent(computerId)}`,
                { cache: "no-store" }
            ),
            fetch(
                `${API_BASE}/metrics?computer_id=${encodeURIComponent(computerId)}`,
                { cache: "no-store" }
            )
        ]);

        const processData = await processResponse.json();

        if (!processResponse.ok || processData.error) {
            throw new Error(
                processData.error || `Processes HTTP ${processResponse.status}`
            );
        }

        if (!Array.isArray(processData.processes)) {
            throw new Error("Unexpected processes API response.");
        }

        processes = processData.processes.map(process => ({
            pid: process.pid,
            process_name: String(process.process_name ?? ""),
            computer_name: String(process.computer_name ?? "")
        }));

        metricsByPid = new Map();

        if (metricsResponse.ok) {
            const metricsData = await metricsResponse.json();

            if (!metricsData.error && Array.isArray(metricsData.metrics)) {
                for (const metric of metricsData.metrics) {
                    metricsByPid.set(Number(metric.pid), metric);
                }

                const validMetrics = metricsData.metrics.filter(metric =>
                    Number.isFinite(Number(metric.cpu_usage_percent)) &&
                    Number.isFinite(Number(metric.memory_usage_mb))
                );

                if (validMetrics.length > 0) {
                    const cpuAverage = validMetrics.reduce(
                        (sum, metric) =>
                            sum + Number(metric.cpu_usage_percent), 0
                    ) / validMetrics.length;

                    const memorySum = validMetrics.reduce(
                        (sum, metric) =>
                            sum + Number(metric.memory_usage_mb), 0
                    );

                    averageCpu.textContent = `${cpuAverage.toFixed(2)}%`;
                    totalMemory.textContent = `${memorySum.toFixed(2)} MB`;
                    metricsStatus.textContent = "Online";
                    metricsUpdated.textContent =
                        `Updated ${new Date().toLocaleTimeString()}`;
                } else {
                    averageCpu.textContent = "N/A";
                    totalMemory.textContent = "N/A";
                    metricsStatus.textContent = "No data";
                    metricsUpdated.textContent = "No measurements available";
                }
            } else {
                throw new Error("Unexpected metrics API response.");
            }
        } else {
            throw new Error(`Metrics HTTP ${metricsResponse.status}`);
        }

        totalProcesses.textContent = processData.count ?? processes.length;
        computerName.textContent = processes.length > 0
            ? processes[0].computer_name
            : "No records";

        apiStatus.textContent = "Online";
        lastUpdated.textContent =
            `Updated ${new Date().toLocaleTimeString()}`;

        renderProcesses();
        showMessage("Process records and metrics loaded successfully.");
    } catch (error) {
        showMessage(
            `Could not refresh dashboard: ${error.message}. Check that the C++ API is running.`,
            true
        );

        if (apiStatus) apiStatus.textContent = "Offline / Error";
        if (metricsStatus) metricsStatus.textContent = "Error";
    } finally {
        refreshBtn.disabled = false;
    }
}

refreshBtn.addEventListener("click", loadProcesses);
searchInput.addEventListener("input", renderProcesses);
computerIdInput.addEventListener("change", loadProcesses);

loadProcesses();
setInterval(loadProcesses, 5000);

// CPU scheduling simulator
(() => {
    const form = document.getElementById("schedulingForm");
    if (!form || form.dataset.initialized === "true") return;
    form.dataset.initialized = "true";

    const algorithm = document.getElementById("scheduleAlgorithm");
    const quantumField = document.getElementById("quantumField");
    const quantumInput = document.getElementById("scheduleQuantum");
    const rows = document.getElementById("scheduleInputRows");
    const addButton = document.getElementById("addProcessButton");
    const runButton = document.getElementById("runScheduleButton");
    const status = document.getElementById("scheduleStatus");
    const resultsPanel = document.getElementById("scheduleResults");

    algorithm.addEventListener("change", () => {
        const isRR = algorithm.value === "ROUND_ROBIN";
        quantumField.hidden = !isRR;
        quantumInput.required = isRR;
    });

    addButton.addEventListener("click", () => {
        if (rows.rows.length >= 100) {
            status.textContent = "Maximum 100 processes allowed.";
            return;
        }

        const tr = document.createElement("tr");
        for (const [min, value] of [[1, ""], [0, "0"], [1, "1"]]) {
            const td = document.createElement("td");
            const input = document.createElement("input");
            input.type = "number";
            input.min = String(min);
            input.required = true;
            input.value = value;
            td.appendChild(input);
            tr.appendChild(td);
        }

        const actionCell = document.createElement("td");
        const remove = document.createElement("button");
        remove.type = "button";
        remove.className = "remove-process";
        remove.textContent = "Remove";
        actionCell.appendChild(remove);
        tr.appendChild(actionCell);
        rows.appendChild(tr);
    });

    rows.addEventListener("click", event => {
        if (!event.target.classList.contains("remove-process")) return;
        if (rows.rows.length <= 1) {
            status.textContent = "At least one process is required.";
            return;
        }
        event.target.closest("tr").remove();
    });

    form.addEventListener("submit", async event => {
        event.preventDefault();
        status.textContent = "Running scheduler and saving to Oracle...";
        runButton.disabled = true;
        resultsPanel.hidden = true;

        try {
            const processes = [];
            const seen = new Set();

            for (const row of rows.rows) {
                const inputs = [...row.querySelectorAll("input")];
                const [pid, arrival, burst] = inputs.map(input => Number(input.value));

                if (!inputs.every(input => input.value.trim() !== "") ||
                    !Number.isInteger(pid) || pid <= 0 ||
                    !Number.isInteger(arrival) || arrival < 0 ||
                    !Number.isInteger(burst) || burst <= 0) {
                    throw new Error("Enter valid integer PID, arrival and burst values.");
                }
                if (seen.has(pid)) throw new Error("Each PID must be unique.");
                seen.add(pid);
                processes.push({ pid, arrival, burst });
            }

            const data = new URLSearchParams();
            data.set("algorithm", algorithm.value);
            data.set("processes",
                processes.map(p => `${p.pid},${p.arrival},${p.burst}`).join(";"));

            if (algorithm.value === "ROUND_ROBIN") {
                const quantum = Number(quantumInput.value);
                if (!Number.isInteger(quantum) || quantum <= 0) {
                    throw new Error("Time quantum must be a positive integer.");
                }
                data.set("quantum", String(quantum));
            }

            const response = await fetch(
                "http://127.0.0.1:8080/api/scheduling/run",
                {
                    method: "POST",
                    headers: { "Content-Type": "application/x-www-form-urlencoded" },
                    body: data.toString()
                }
            );

            const result = await response.json();
            if (!response.ok) {
                throw new Error(result.error || `Request failed (${response.status})`);
            }

            document.getElementById("averageWaitingResult").textContent =
                Number(result.average_waiting_time).toFixed(2);
            document.getElementById("averageTurnaroundResult").textContent =
                Number(result.average_turnaround_time).toFixed(2);
            document.getElementById("scheduleRunId").textContent = result.run_id;

            const resultRows = document.getElementById("scheduleResultRows");
            resultRows.replaceChildren();

            for (const item of result.results) {
                const tr = document.createElement("tr");
                for (const value of [
                    item.pid, item.completion_time, item.turnaround_time,
                    item.waiting_time, item.response_time
                ]) {
                    const td = document.createElement("td");
                    td.textContent = String(value);
                    tr.appendChild(td);
                }
                resultRows.appendChild(tr);
            }

            const gantt = document.getElementById("scheduleGantt");
            gantt.replaceChildren();

            for (const segment of result.gantt) {
                const block = document.createElement("div");
                block.className = "gantt-segment";

                const process = document.createElement("strong");
                process.textContent = `P${segment.pid}`;

                const times = document.createElement("span");
                times.textContent = `${segment.start_time} – ${segment.end_time}`;

                block.append(process, times);
                gantt.appendChild(block);
            }

            resultsPanel.hidden = false;
            status.textContent =
                `${result.algorithm} completed and saved as run ${result.run_id}.`;
        } catch (error) {
            status.textContent = `Scheduling failed: ${error.message}`;
        } finally {
            runButton.disabled = false;
        }
    });
})();


// Deadlock avoidance: Banker's Algorithm API integration.
(() => {
    const form = document.getElementById("deadlockForm");
    if (!form || form.dataset.initialized === "true") return;
    form.dataset.initialized = "true";

    const status = document.getElementById("deadlockStatus");
    const button = document.getElementById("deadlockCheckButton");
    const results = document.getElementById("deadlockResults");

    function parseMatrix(text) {
        return text.trim().split(/\n+/).map(line => {
            if (!line.trim()) throw new Error("Matrix rows cannot be empty.");
            const values = line.split(",").map(value => value.trim());
            if (values.some(value => !/^\d+$/.test(value))) {
                throw new Error("Use non-negative integers separated by commas.");
            }
            return values.map(Number);
        });
    }

    form.addEventListener("submit", async event => {
        event.preventDefault();
        results.hidden = true;
        button.disabled = true;
        status.textContent = "Checking safety and saving to Oracle...";

        try {
            const availableText =
                document.getElementById("deadlockAvailable").value.trim();
            if (!/^\d+(,\d+)*$/.test(availableText)) {
                throw new Error("Available resources must be comma-separated non-negative integers.");
            }

            const available = availableText.split(",").map(Number);
            const allocation = parseMatrix(
                document.getElementById("deadlockAllocation").value
            );
            const maximum = parseMatrix(
                document.getElementById("deadlockMaximum").value
            );

            if (allocation.length !== maximum.length) {
                throw new Error("Allocation and Maximum must have the same number of rows.");
            }
            if (allocation.length < 1 || allocation.length > 100 ||
                available.length < 1 || available.length > 20) {
                throw new Error("Use 1–100 processes and 1–20 resource types.");
            }

            for (const matrix of [allocation, maximum]) {
                if (matrix.some(row => row.length !== available.length)) {
                    throw new Error("Every matrix row must match the number of available resource types.");
                }
            }

            for (let i = 0; i < allocation.length; i++) {
                for (let j = 0; j < available.length; j++) {
                    if (maximum[i][j] < allocation[i][j]) {
                        throw new Error("Maximum demand cannot be less than allocation.");
                    }
                }
            }

            const data = new URLSearchParams();
            data.set("available", available.join(","));
            data.set("allocation",
                allocation.map(row => row.join(",")).join(";"));
            data.set("maximum",
                maximum.map(row => row.join(",")).join(";"));

            const response = await fetch(
                "http://127.0.0.1:8080/api/deadlock/check",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/x-www-form-urlencoded"
                    },
                    body: data.toString()
                }
            );

            const result = await response.json();
            if (!response.ok) {
                throw new Error(result.error || `Request failed (${response.status})`);
            }

            document.getElementById("deadlockSafetyResult").textContent =
                result.safe ? "SAFE" : "UNSAFE";
            document.getElementById("deadlockRunId").textContent =
                String(result.run_id);

            document.getElementById("deadlockSequence").textContent =
                result.safe
                    ? result.safe_sequence.map(index => `P${index}`).join(" → ")
                    : "No safe sequence exists.";

            const head = document.getElementById("deadlockNeedHead");
            const body = document.getElementById("deadlockNeedBody");
            head.replaceChildren();
            body.replaceChildren();

            const headerRow = document.createElement("tr");
            const processHeader = document.createElement("th");
            processHeader.textContent = "Process";
            headerRow.appendChild(processHeader);

            available.forEach((_, j) => {
                const th = document.createElement("th");
                th.textContent = `R${j}`;
                headerRow.appendChild(th);
            });
            head.appendChild(headerRow);

            result.need.forEach((row, i) => {
                const tr = document.createElement("tr");
                const label = document.createElement("th");
                label.textContent = `P${i}`;
                tr.appendChild(label);

                row.forEach(value => {
                    const td = document.createElement("td");
                    td.textContent = String(value);
                    tr.appendChild(td);
                });
                body.appendChild(tr);
            });

            results.hidden = false;
            status.textContent =
                `Analysis saved to Oracle as run ${result.run_id}.`;
        } catch (error) {
            status.textContent = `Deadlock check failed: ${error.message}`;
        } finally {
            button.disabled = false;
        }
    });
})();

/* Contiguous file allocation dashboard */
(() => {
    const form = document.getElementById("fileAllocationForm");
    if (!form) return;

    const status = document.getElementById("fileAllocationStatus");
    const button = document.getElementById("allocateFilesButton");
    const results = document.getElementById("fileAllocationResults");

    form.addEventListener("submit", async event => {
        event.preventDefault();
        button.disabled = true;
        status.textContent = "Running allocation and saving to Oracle...";

        try {
            const totalBlocks = Number(
                document.getElementById("diskBlocks").value
            );
            const files = document.getElementById("fileRequests").value.trim();

            if (!Number.isInteger(totalBlocks) ||
                totalBlocks < 1 || totalBlocks > 10000) {
                throw new Error("Disk size must be between 1 and 10000 blocks.");
            }
            if (!files) throw new Error("Enter at least one file request.");

            const data = new URLSearchParams();
            data.set("total_blocks", String(totalBlocks));
            data.set("files", files);

            const response = await fetch(
                "http://127.0.0.1:8080/api/file-allocation/run",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/x-www-form-urlencoded"
                    },
                    body: data.toString()
                }
            );

            const result = await response.json();
            if (!response.ok) {
                throw new Error(result.error || `Request failed (${response.status})`);
            }

            document.getElementById("faTotalBlocks").textContent =
                String(result.total_blocks);
            document.getElementById("faFreeBlocks").textContent =
                String(result.free_blocks);
            document.getElementById("faAllocatedFiles").textContent =
                `${result.files_allocated}/${result.files_requested}`;
            document.getElementById("faRunId").textContent =
                String(result.run_id);

            const disk = document.getElementById("fileAllocationDisk");
            disk.replaceChildren();

            const owners = Array(result.total_blocks).fill(null);
            result.results.forEach(file => {
                if (file.success) {
                    for (let i = file.start_block; i <= file.end_block; i++) {
                        owners[i] = file.file_name;
                    }
                }
            });

            owners.forEach((owner, index) => {
                const block = document.createElement("div");
                block.className = "file-block" + (owner ? " occupied" : " free");
                block.textContent = String(index);
                block.title = owner ? `${owner} (block ${index})` : `Free block ${index}`;
                disk.appendChild(block);
            });

            const body = document.getElementById("fileAllocationResultsBody");
            body.replaceChildren();

            result.results.forEach(file => {
                const row = document.createElement("tr");
                const values = [
                    file.file_name,
                    String(file.requested_blocks),
                    file.success ? "Success" : "Failed",
                    file.success
                        ? `${file.start_block}–${file.end_block}`
                        : "—",
                    file.message
                ];

                values.forEach((value, index) => {
                    const cell = document.createElement("td");
                    cell.textContent = value;
                    if (index === 2) {
                        cell.className = file.success ? "allocation-success" : "allocation-failure";
                    }
                    row.appendChild(cell);
                });
                body.appendChild(row);
            });

            results.hidden = false;
            status.textContent = `Run ${result.run_id} saved to Oracle successfully.`;
        } catch (error) {
            status.textContent = `Allocation failed: ${error.message}`;
        } finally {
            button.disabled = false;
        }
    });
})();


/* =========================================
   PAGING SIMULATOR INTEGRATION
   ========================================= */

(() => {
    const form = document.getElementById("pagingForm");
    if (!form || form.dataset.initialized === "true") return;

    form.dataset.initialized = "true";

    const algorithmInput = document.getElementById("pagingAlgorithm");
    const frameInput = document.getElementById("pagingFrameCount");
    const referenceInput = document.getElementById("pagingReferenceString");

    const runButton = document.getElementById("runPagingButton");
    const compareButton = document.getElementById("comparePagingButton");
    const status = document.getElementById("pagingStatus");

    const results = document.getElementById("pagingResults");
    const comparison = document.getElementById("pagingComparison");

    const API = "http://127.0.0.1:8080/api/paging/run";

    function getInput() {
        const frameCount = Number(frameInput.value);

        if (!Number.isInteger(frameCount) || frameCount < 1 || frameCount > 100) {
            throw new Error("Frame count must be between 1 and 100.");
        }

        const raw = referenceInput.value.trim();

        if (!raw) {
            throw new Error("Enter a page reference string.");
        }

        const parts = raw.split(",").map(value => value.trim());

        if (parts.length > 200 || parts.some(value => !/^\d+$/.test(value))) {
            throw new Error("Enter 1–200 non-negative page numbers separated by commas.");
        }

        return {
            frameCount,
            references: parts.map(Number)
        };
    }

    async function executeAlgorithm(algorithm, input) {
        const data = new URLSearchParams();

        data.set("algorithm", algorithm);
        data.set("frame_count", String(input.frameCount));
        data.set("reference_string", input.references.join(","));

        const response = await fetch(API, {
            method: "POST",
            headers: {
                "Content-Type": "application/x-www-form-urlencoded"
            },
            body: data.toString()
        });

        const result = await response.json();

        if (!response.ok) {
            throw new Error(result.error || `Request failed (${response.status})`);
        }

        if (!Array.isArray(result.steps) ||
            result.steps.length !== input.references.length) {
            throw new Error("The API returned an incomplete paging result.");
        }

        return result;
    }

    function createCell(value, className = "") {
        const cell = document.createElement("td");
        cell.textContent = String(value);
        if (className) cell.className = className;
        return cell;
    }

    function renderResult(result) {
        document.getElementById("pagingHits").textContent =
            String(result.page_hits);

        document.getElementById("pagingFaults").textContent =
            String(result.page_faults);

        document.getElementById("pagingFaultRatio").textContent =
            `${(Number(result.page_fault_ratio) * 100).toFixed(2)}%`;

        document.getElementById("pagingRunId").textContent =
            String(result.run_id);

        document.getElementById("pagingResultTitle").textContent =
            `${result.algorithm} Simulation`;

        document.getElementById("pagingReferenceCount").textContent =
            `${result.total_references} page references · ${result.frame_count} frames`;

        // Display final frame contents.
        const finalFrames = result.steps[result.steps.length - 1].frames;
        const frameContainer =
            document.getElementById("pagingFrameVisualization");

        frameContainer.replaceChildren();

        finalFrames.forEach((page, index) => {
            const card = document.createElement("div");
            card.className = "paging-frame" + (page === -1 ? " empty" : "");

            const label = document.createElement("span");
            label.textContent = `Frame ${index + 1}`;

            const value = document.createElement("strong");
            value.textContent = page === -1 ? "Empty" : String(page);

            card.append(label, value);
            frameContainer.appendChild(card);
        });

        // Display every execution step.
        const body = document.getElementById("pagingStepsBody");
        body.replaceChildren();

        result.steps.forEach(step => {
            const row = document.createElement("tr");

            row.appendChild(createCell(step.step_number));
            row.appendChild(createCell(step.page_number));

            const framesCell = document.createElement("td");
            const frames = document.createElement("div");
            frames.className = "paging-frames";

            step.frames.forEach(page => {
                const frame = document.createElement("span");
                frame.className = "paging-mini-frame";
                frame.textContent = page === -1 ? "—" : String(page);
                frames.appendChild(frame);
            });

            framesCell.appendChild(frames);
            row.appendChild(framesCell);

            row.appendChild(createCell(
                step.page_fault ? "PAGE FAULT" : "PAGE HIT",
                step.page_fault ? "paging-fault" : "paging-hit"
            ));

            row.appendChild(createCell(
                step.replaced_page === null ||
                step.replaced_page === undefined
                    ? "—"
                    : step.replaced_page
            ));

            body.appendChild(row);
        });

        results.hidden = false;
    }

    function renderComparison(reports) {
        const body = document.getElementById("pagingComparisonBody");
        const chart = document.getElementById("pagingComparisonChart");

        body.replaceChildren();
        chart.replaceChildren();

        const maximumFaults = Math.max(
            1,
            ...reports.map(report => Number(report.page_faults))
        );

        reports.forEach(report => {
            const row = document.createElement("tr");

            row.appendChild(createCell(report.algorithm));
            row.appendChild(createCell(report.page_hits));
            row.appendChild(createCell(report.page_faults));
            row.appendChild(createCell(
                `${(Number(report.page_fault_ratio) * 100).toFixed(2)}%`
            ));
            row.appendChild(createCell(report.run_id));

            body.appendChild(row);

            const barRow = document.createElement("div");
            barRow.className = "paging-bar-row";

            const label = document.createElement("strong");
            label.textContent = report.algorithm;

            const track = document.createElement("div");
            track.className = "paging-bar-track";

            const bar = document.createElement("div");
            bar.className = "paging-bar";
            bar.style.width =
                `${Number(report.page_faults) / maximumFaults * 100}%`;

            track.appendChild(bar);

            const value = document.createElement("strong");
            value.textContent = String(report.page_faults);

            barRow.append(label, track, value);
            chart.appendChild(barRow);
        });

        comparison.hidden = false;
    }

    function setBusy(busy) {
        runButton.disabled = busy;
        compareButton.disabled = busy;
        algorithmInput.disabled = busy;
        frameInput.disabled = busy;
        referenceInput.disabled = busy;
    }

    form.addEventListener("submit", async event => {
        event.preventDefault();

        setBusy(true);
        comparison.hidden = true;
        status.textContent = "Running simulation and saving results to Oracle...";

        try {
            const input = getInput();
            const algorithm = algorithmInput.value;

            const result = await executeAlgorithm(algorithm, input);

            renderResult(result);

            status.textContent =
                `${algorithm} completed successfully. Oracle run ID: ${result.run_id}.`;
        } catch (error) {
            status.textContent = `Paging simulation failed: ${error.message}`;
        } finally {
            setBusy(false);
        }
    });

    compareButton.addEventListener("click", async () => {
        setBusy(true);
        comparison.hidden = true;
        status.textContent = "Running FIFO, LRU and Optimal simulations...";

        try {
            const input = getInput();
            const algorithms = ["FIFO", "LRU", "OPTIMAL"];

            const reports = [];

            for (const algorithm of algorithms) {
                status.textContent =
                    `Running ${algorithm} and saving to Oracle...`;

                const result = await executeAlgorithm(algorithm, input);
                reports.push(result);
            }

            renderComparison(reports);

            // Show Optimal's detailed steps after comparison.
            const optimal = reports.find(report => report.algorithm === "OPTIMAL");
            if (optimal) {
                renderResult(optimal);
            }

            const best = reports.reduce((a, b) =>
                Number(a.page_faults) <= Number(b.page_faults) ? a : b
            );

            status.textContent =
                `Comparison completed. Best result: ${best.algorithm} with ${best.page_faults} page faults. All three runs saved to Oracle.`;

        } catch (error) {
            status.textContent = `Comparison failed: ${error.message}`;
        } finally {
            setBusy(false);
        }
    });
})();

