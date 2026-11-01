// SysCallLab SPA Controller
const App = {
  currentSection: 'overview',
  activeExecution: null,
  platform: 'unknown',

  init() {
    this.bindNavigation();
    this.checkHealth();
    this.loadSection('overview');

    // Polling server health every 10 seconds
    setInterval(() => this.checkHealth(), 10000);
  },

  bindNavigation() {
    document.querySelectorAll('.nav-link').forEach(link => {
      link.addEventListener('click', (e) => {
        e.preventDefault();
        const section = link.getAttribute('data-section');
        if (section) {
          this.loadSection(section);
        }
      });
    });
  },

  async checkHealth() {
    const dot = document.getElementById('serverStatusDot');
    const label = document.getElementById('serverStatusText');
    try {
      const res = await API.getHealth();
      if (res.status) {
        this.platform = res.platform || 'unknown';
        dot.className = 'pulse-dot';
        label.textContent = `Online (${res.platform})`;
      } else {
        throw new Error();
      }
    } catch (err) {
      dot.className = 'pulse-dot error';
      label.textContent = 'Server Disconnected';
    }
  },

  loadSection(section) {
    this.currentSection = section;

    document.querySelectorAll('.nav-link').forEach(link => {
      if (link.getAttribute('data-section') === section) {
        link.classList.add('active');
      } else {
        link.classList.remove('active');
      }
    });

    const titleEl = document.getElementById('pageTitle');
    const subtitleEl = document.getElementById('pageSubtitle');
    const container = document.getElementById('contentContainer');

    switch (section) {
      case 'overview':
        titleEl.textContent = 'System Call Operations Overview';
        subtitleEl.textContent = 'Authentic execution telemetry and operating-system summary';
        this.renderOverview(container);
        break;
      case 'file-ops':
        titleEl.textContent = 'File System Calls';
        subtitleEl.textContent = 'Execute native file APIs and inspect kernel return codes';
        this.renderFileOps(container);
        break;
      case 'process-mgmt':
        titleEl.textContent = 'Process Management';
        subtitleEl.textContent = 'Create and supervise native child processes with pipe capture';
        this.renderProcessMgmt(container);
        break;
      case 'error-analyzer':
        titleEl.textContent = 'Kernel Error Analyzer';
        subtitleEl.textContent = 'Inspect native error codes and root cause classification';
        this.renderErrorAnalyzer(container);
        break;
      case 'recovery':
        titleEl.textContent = 'Error Recovery Center';
        subtitleEl.textContent = 'Explicit user-confirmed safe remediation workflows';
        this.renderRecoveryCenter(container);
        break;
      case 'test-runner':
        titleEl.textContent = 'System Call Test Runner';
        subtitleEl.textContent = 'Run automated OS assertions and expected failure suites';
        this.renderTestRunner(container);
        break;
      case 'history':
        titleEl.textContent = 'Execution History';
        subtitleEl.textContent = 'Persistent SQLite database log of all OS executions';
        this.renderHistory(container);
        break;
      case 'audit-logs':
        titleEl.textContent = 'Audit Logging';
        subtitleEl.textContent = 'Verifiable system event trail with CSV export';
        this.renderAuditLogs(container);
        break;
      case 'system-info':
        titleEl.textContent = 'System & Environment Information';
        subtitleEl.textContent = 'Hardware architecture, OS kernel, and compiler specs';
        this.renderSystemInfo(container);
        break;
      case 'settings':
        titleEl.textContent = 'Application Settings';
        subtitleEl.textContent = 'Local HTTP server and backend storage configuration';
        this.renderSettings(container);
        break;
    }
  },

  // 1. OVERVIEW
  async renderOverview(container) {
    container.innerHTML = `
      <div class="system-banner" id="sysBanner">
        <div class="system-meta-item"><span class="label">Operating System</span><span class="value" id="banOS">Loading...</span></div>
        <div class="system-meta-item"><span class="label">Architecture</span><span class="value" id="banArch">Loading...</span></div>
        <div class="system-meta-item"><span class="label">CPU Cores</span><span class="value" id="banCpu">Loading...</span></div>
        <div class="system-meta-item"><span class="label">Host Process ID</span><span class="value" id="banPid">Loading...</span></div>
      </div>

      <div class="stats-grid">
        <div class="stat-card">
          <div class="stat-header"><span class="stat-title">Total Executions</span></div>
          <div class="stat-value" id="statTotal">-</div>
          <div class="stat-sub">Persisted in database</div>
        </div>
        <div class="stat-card">
          <div class="stat-header"><span class="stat-title">Successful Executions</span></div>
          <div class="stat-value" style="color:var(--accent-green)" id="statSuccess">-</div>
          <div class="stat-sub">Returned clean status</div>
        </div>
        <div class="stat-card">
          <div class="stat-header"><span class="stat-title">Failed Executions</span></div>
          <div class="stat-value" style="color:var(--accent-red)" id="statFailed">-</div>
          <div class="stat-sub">Native OS errors captured</div>
        </div>
        <div class="stat-card">
          <div class="stat-header"><span class="stat-title">Recovered Operations</span></div>
          <div class="stat-value" style="color:var(--accent-cyan)" id="statRecovered">-</div>
          <div class="stat-sub">Verified through recovery</div>
        </div>
      </div>

      <div id="recentActivityBox">
        <h3 style="margin-bottom:14px;font-size:1.05rem;">Recent Native System Call Executions</h3>
        <div id="recentActivityTable">Loading...</div>
      </div>
    `;

    try {
      const [sys, stats, hist] = await Promise.all([
        API.getSystemInfo(),
        API.getStatistics(),
        API.getHistory()
      ]);

      document.getElementById('banOS').textContent = sys.os_name + ' (' + sys.os_version + ')';
      document.getElementById('banArch').textContent = sys.architecture;
      document.getElementById('banCpu').textContent = sys.cpu_cores + ' Cores';
      document.getElementById('banPid').textContent = sys.process_id;

      document.getElementById('statTotal').textContent = stats.total_executions;
      document.getElementById('statSuccess').textContent = stats.successful_executions;
      document.getElementById('statFailed').textContent = stats.failed_executions;
      document.getElementById('statRecovered').textContent = stats.recovered_operations;

      const actEl = document.getElementById('recentActivityTable');
      if (!hist.executions || hist.executions.length === 0) {
        actEl.innerHTML = `<div class="empty-state"><p>No execution data available. Select an operation to begin.</p></div>`;
      } else {
        let rows = hist.executions.slice(0, 5).map(e => `
          <tr>
            <td class="code-cell">${e.execution_id}</td>
            <td>${e.timestamp}</td>
            <td><strong>${e.operation}</strong></td>
            <td class="code-cell">${e.native_api}</td>
            <td>${e.success ? '<span class="badge badge-success">SUCCESS</span>' : '<span class="badge badge-danger">FAILED (' + e.native_error_code + ')</span>'}</td>
            <td>${e.recovery_status === 'RECOVERED' ? '<span class="badge badge-success">RECOVERED</span>' : e.recovery_status}</td>
          </tr>
        `).join('');

        actEl.innerHTML = `
          <div class="data-table-container">
            <table class="data-table">
              <thead>
                <tr>
                  <th>Execution ID</th>
                  <th>Timestamp</th>
                  <th>Operation</th>
                  <th>Native API</th>
                  <th>Result</th>
                  <th>Recovery</th>
                </tr>
              </thead>
              <tbody>${rows}</tbody>
            </table>
          </div>
        `;
      }
    } catch (e) {
      console.error(e);
    }
  },

  // 2. FILE OPERATIONS
  renderFileOps(container) {
    container.innerHTML = `
      <div style="display:grid; grid-template-columns: 1fr 1fr; gap: 24px;">
        <div class="result-card" style="margin-top:0;">
          <h3 style="margin-bottom:16px;">Configure File System Call</h3>
          <form id="fileOpForm">
            <div class="form-group">
              <label class="form-label">Operation</label>
              <select class="form-control" id="fileOpSelect">
                <option value="open">Open Existing File (CreateFile / open)</option>
                <option value="read">Read File Bytes (ReadFile / read)</option>
                <option value="write">Write File Data (WriteFile / write)</option>
                <option value="missing_test">Missing File Test (Controlled Failure)</option>
                <option value="invalid_descriptor">Invalid Descriptor Test (Fake Handle)</option>
                <option value="readonly_test">Read-Only Handle Write Test (Access Denied)</option>
              </select>
            </div>
            <div class="form-group" id="filePathGroup">
              <label class="form-label">Target File Path</label>
              <input type="text" class="form-control code" id="filePathInput" placeholder="data/sample_test.txt" value="data/sample_test.txt">
            </div>
            <div class="form-group" id="fileModeGroup">
              <label class="form-label">Open Mode</label>
              <select class="form-control" id="fileModeSelect">
                <option value="r">Read (r / OPEN_EXISTING)</option>
                <option value="w">Write (w / CREATE_ALWAYS)</option>
                <option value="rw">Read-Write (rw)</option>
              </select>
            </div>
            <div class="form-group" id="fileContentGroup" style="display:none;">
              <label class="form-label">Write Payload Content</label>
              <textarea class="form-control code" id="fileContentInput" rows="3" placeholder="Enter content to write...">SysCallLab Live Buffer Data</textarea>
            </div>
            <div class="form-group" id="fileReadSizeGroup" style="display:none;">
              <label class="form-label">Read Size (Bytes)</label>
              <input type="number" class="form-control" id="fileReadSizeInput" value="512" min="1" max="4096">
            </div>
            <button type="submit" class="btn btn-primary" id="btnExecuteFile">
              <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor"><polygon points="5 3 19 12 5 21 5 3"></polygon></svg>
              Execute System Call
            </button>
          </form>
        </div>

        <div>
          <div id="fileResultContainer">
            <div class="result-card" style="margin-top:0;">
              <div class="empty-state">
                <p>No execution data available. Select an operation to begin.</p>
              </div>
            </div>
          </div>
        </div>
      </div>
    `;

    const opSelect = document.getElementById('fileOpSelect');
    const pathGroup = document.getElementById('filePathGroup');
    const modeGroup = document.getElementById('fileModeGroup');
    const contentGroup = document.getElementById('fileContentGroup');
    const readSizeGroup = document.getElementById('fileReadSizeGroup');

    opSelect.addEventListener('change', () => {
      const op = opSelect.value;
      contentGroup.style.display = (op === 'write') ? 'block' : 'none';
      readSizeGroup.style.display = (op === 'read') ? 'block' : 'none';
      modeGroup.style.display = (op === 'open') ? 'block' : 'none';
      pathGroup.style.display = (op === 'invalid_descriptor') ? 'none' : 'block';
    });

    document.getElementById('fileOpForm').addEventListener('submit', async (e) => {
      e.preventDefault();
      const btn = document.getElementById('btnExecuteFile');
      const operation = opSelect.value;
      const mode = document.getElementById('fileModeSelect').value;
      const modifiesFile = operation === 'write' || (operation === 'open' && mode !== 'r');
      if (modifiesFile && !window.confirm('This operation can modify the selected file. Continue?')) {
        return;
      }
      btn.disabled = true;
      btn.innerHTML = `<div class="spinner"></div> Executing kernel API...`;

      try {
        const payload = {
          operation: opSelect.value,
          path: document.getElementById('filePathInput').value,
          mode: mode,
          content: document.getElementById('fileContentInput').value,
          read_size: parseInt(document.getElementById('fileReadSizeInput').value, 10),
          user_confirmed: modifiesFile
        };
        const res = await API.executeFileOp(payload);
        this.activeExecution = res;
        this.displayExecutionResult('fileResultContainer', res);
      } catch (err) {
        document.getElementById('fileResultContainer').innerHTML = `
          <div class="result-card" style="border-color:var(--accent-red)">
            <h4 style="color:var(--accent-red)">Execution Error</h4>
            <p style="margin-top:8px;">${err.message}</p>
          </div>
        `;
      } finally {
        btn.disabled = false;
        btn.innerHTML = `Execute System Call`;
      }
    });
  },

  // 3. PROCESS MANAGEMENT
  renderProcessMgmt(container) {
    container.innerHTML = `
      <div style="display:grid; grid-template-columns: 1fr 1fr; gap: 24px;">
        <div class="result-card" style="margin-top:0;">
          <h3 style="margin-bottom:16px;">Configure Process Execution</h3>
          <form id="procForm">
            <div class="form-group">
              <label class="form-label">Executable Selection</label>
              <select class="form-control" id="procPresetSelect">
                <option value="helper">Helper Child Process (build/bin/test_child.exe)</option>
                <option value="target">Target Clean Executable (build/bin/test_executable.exe)</option>
                <option value="missing">Missing Executable Test (Controlled Failure)</option>
                <option value="custom">Custom Binary Path...</option>
              </select>
            </div>
            <div class="form-group" id="procPathGroup" style="display:none;">
              <label class="form-label">Custom Executable Path</label>
              <input type="text" class="form-control code" id="procPathInput" placeholder="C:\\Windows\\System32\\cmd.exe">
            </div>
            <div class="form-group">
              <label class="form-label">Arguments (Safe Vector)</label>
              <input type="text" class="form-control code" id="procArgsInput" value="0 10" placeholder="0 10 (ExitCode SleepMs)">
            </div>
            <div class="form-group">
              <label class="form-label">Kernel Timeout (Milliseconds)</label>
              <input type="number" class="form-control" id="procTimeoutInput" value="4000" min="500" max="30000">
            </div>
            <button type="submit" class="btn btn-primary" id="btnExecuteProc">Execute Child Process</button>
          </form>
        </div>

        <div>
          <div id="procResultContainer">
            <div class="result-card" style="margin-top:0;">
              <div class="empty-state">
                <p>No execution data available. Select an operation to begin.</p>
              </div>
            </div>
          </div>
        </div>
      </div>
    `;

    const preset = document.getElementById('procPresetSelect');
    const pathGroup = document.getElementById('procPathGroup');
    const pathInput = document.getElementById('procPathInput');
    const argsInput = document.getElementById('procArgsInput');

    preset.addEventListener('change', () => {
      if (preset.value === 'custom') {
        pathGroup.style.display = 'block';
      } else {
        pathGroup.style.display = 'none';
        if (preset.value === 'helper') argsInput.value = '0 10';
        if (preset.value === 'target') argsInput.value = '';
        if (preset.value === 'missing') argsInput.value = '';
      }
    });

    document.getElementById('procForm').addEventListener('submit', async (e) => {
      e.preventDefault();
      const btn = document.getElementById('btnExecuteProc');
      btn.disabled = true;
      btn.innerHTML = `<div class="spinner"></div> Spawning child process...`;

      let targetBinary = '';
      const executableSuffix = this.platform === 'windows' ? '.exe' : '';
      if (preset.value === 'helper') targetBinary = `build/bin/test_child${executableSuffix}`;
      else if (preset.value === 'target') targetBinary = `build/bin/test_executable${executableSuffix}`;
      else if (preset.value === 'missing') targetBinary = 'missing_test';
      else targetBinary = pathInput.value;

      try {
        const payload = {
          executable_path: targetBinary,
          arguments: argsInput.value,
          timeout_ms: parseInt(document.getElementById('procTimeoutInput').value, 10)
        };
        const res = await API.executeProcessOp(payload);
        this.activeExecution = res;
        this.displayExecutionResult('procResultContainer', res);
      } catch (err) {
        document.getElementById('procResultContainer').innerHTML = `
          <div class="result-card" style="border-color:var(--accent-red)">
            <h4 style="color:var(--accent-red)">Process Error</h4>
            <p style="margin-top:8px;">${err.message}</p>
          </div>
        `;
      } finally {
        btn.disabled = false;
        btn.innerHTML = `Execute Child Process`;
      }
    });
  },

  // 4. ERROR ANALYZER
  async renderErrorAnalyzer(container) {
    container.innerHTML = `
      <div class="result-card" style="margin-top:0; margin-bottom: 24px;">
        <h3 style="margin-bottom:12px;">Inspect Stored System Call Error</h3>
        <p style="font-size:0.85rem; color:var(--text-muted); margin-bottom:16px;">
          Select any recent execution from the persistent database to perform in-depth operating-system error analysis.
        </p>
        <div style="display:flex; gap:12px;">
          <input type="text" class="form-control code" id="analyzeIdInput" placeholder="Enter execution ID (e.g. f-open-...)">
          <button class="btn btn-primary" id="btnAnalyze">Analyze Error</button>
        </div>
      </div>
      <div id="analyzeResultContainer">
        <div class="result-card">
          <div class="empty-state">
            <p>No execution data available. Select an operation to begin.</p>
          </div>
        </div>
      </div>
    `;

    document.getElementById('btnAnalyze').addEventListener('click', async () => {
      const id = document.getElementById('analyzeIdInput').value.trim();
      if (!id) return;
      const resContainer = document.getElementById('analyzeResultContainer');
      resContainer.innerHTML = `<div class="result-card"><div class="spinner"></div> Loading analysis from database...</div>`;

      try {
        const data = await API.analyzeError(id);
        this.activeExecution = data;
        this.displayExecutionResult('analyzeResultContainer', data);
      } catch (err) {
        resContainer.innerHTML = `
          <div class="result-card" style="border-color:var(--accent-red)">
            <h4 style="color:var(--accent-red)">Analysis Error</h4>
            <p style="margin-top:8px;">${err.message}</p>
          </div>
        `;
      }
    });

    if (this.activeExecution) {
      document.getElementById('analyzeIdInput').value = this.activeExecution.execution_id;
      this.displayExecutionResult('analyzeResultContainer', this.activeExecution);
    }
  },

  // 5. RECOVERY CENTER
  renderRecoveryCenter(container) {
    container.innerHTML = `
      <div class="result-card" style="margin-top:0; margin-bottom: 24px;">
        <h3 style="margin-bottom:12px;">User-Confirmed Error Recovery</h3>
        <p style="font-size:0.85rem; color:var(--text-muted); margin-bottom:16px;">
          System recovery is never automated. Review the diagnosis and explicitly authorize the repair strategy.
        </p>
        <div style="display:flex; flex-direction:column; gap:14px; max-width: 600px;">
          <div class="form-group">
            <label class="form-label">Failed Execution ID</label>
            <input type="text" class="form-control code" id="recExecIdInput" placeholder="Execution ID needing recovery...">
          </div>
          <div class="form-group">
            <label class="form-label">Confirm Remediation Action</label>
            <div style="display:flex; align-items:center; gap:10px; margin-top:6px;">
              <input type="checkbox" id="recConfirmCheck" style="width:18px; height:18px; cursor:pointer;">
              <label for="recConfirmCheck" style="font-size:0.85rem; cursor:pointer;">
                I explicitly authorize SysCallLab to execute the safe recovery procedure on this resource.
              </label>
            </div>
          </div>
          <button class="btn btn-success" id="btnExecuteRecovery" style="width:fit-content;">
            Execute Recovery Strategy
          </button>
        </div>
      </div>
      <div id="recoveryResultContainer">
        <div class="result-card">
          <div class="empty-state">
            <p>No execution data available. Select an operation to begin.</p>
          </div>
        </div>
      </div>
    `;

    if (this.activeExecution && !this.activeExecution.success) {
      document.getElementById('recExecIdInput').value = this.activeExecution.execution_id;
    }

    document.getElementById('btnExecuteRecovery').addEventListener('click', async () => {
      const id = document.getElementById('recExecIdInput').value.trim();
      const confirmed = document.getElementById('recConfirmCheck').checked;
      const resContainer = document.getElementById('recoveryResultContainer');

      if (!id) {
        alert('Please provide an execution ID.');
        return;
      }

      resContainer.innerHTML = `<div class="result-card"><div class="spinner"></div> Executing kernel recovery verification...</div>`;

      try {
        const res = await API.executeRecovery(id, 'auto_heal', confirmed);
        resContainer.innerHTML = `
          <div class="result-card" style="border-color:${res.success ? 'var(--accent-green)' : 'var(--accent-red)'}">
            <div class="result-card-header">
              <h3>Recovery Result: ${res.recovery_status}</h3>
              <span class="badge ${res.success ? 'badge-success' : 'badge-danger'}">${res.recovery_status}</span>
            </div>
            <div class="result-grid">
              <div class="result-item"><span class="k">Execution Target</span><span class="v">${res.execution_id}</span></div>
              <div class="result-item"><span class="k">Strategy</span><span class="v">${res.strategy}</span></div>
              <div class="result-item"><span class="k">User Confirmed</span><span class="v">${res.user_confirmed ? 'YES' : 'NO'}</span></div>
            </div>
            <div style="margin-top:16px;">
              <span class="result-item"><span class="k">Remediation Verification Details</span></span>
              <p style="margin-top:6px; font-size:0.9rem; line-height:1.5;">${res.details}</p>
            </div>
          </div>
        `;
      } catch (err) {
        resContainer.innerHTML = `
          <div class="result-card" style="border-color:var(--accent-red)">
            <h4 style="color:var(--accent-red)">Recovery Failed</h4>
            <p style="margin-top:8px;">${err.message}</p>
          </div>
        `;
      }
    });
  },

  // 6. TEST RUNNER
  renderTestRunner(container) {
    container.innerHTML = `
      <div class="result-card" style="margin-top:0; margin-bottom: 24px;">
        <h3 style="margin-bottom:12px;">Controlled System Call Test Suite</h3>
        <p style="font-size:0.85rem; color:var(--text-muted); margin-bottom:16px;">
          Tests evaluate whether actual system calls behave correctly. Controlled failures (like missing files) PASS when the expected native error is accurately returned.
        </p>
        <div style="display:flex; gap:12px;">
          <select class="form-control" style="width:200px;" id="testCategorySelect">
            <option value="all">Complete Test Suite</option>
            <option value="file">File Operations Only</option>
            <option value="process">Process Operations Only</option>
            <option value="error">Error Capture Only</option>
          </select>
          <button class="btn btn-primary" id="btnRunTests">Run Selected Tests</button>
        </div>
      </div>
      <div id="testResultsContainer">
        <div class="result-card">
          <div class="empty-state">
            <p>No tests have been executed. Click 'Run Selected Tests' to begin.</p>
          </div>
        </div>
      </div>
    `;

    document.getElementById('btnRunTests').addEventListener('click', async () => {
      const cat = document.getElementById('testCategorySelect').value;
      const resContainer = document.getElementById('testResultsContainer');
      resContainer.innerHTML = `<div class="result-card"><div class="spinner"></div> Executing kernel test suite...</div>`;

      try {
        const data = await API.runTests(cat);
        let rows = data.tests.map(t => `
          <tr>
            <td class="code-cell">${t.test_id}</td>
            <td><strong>${t.name}</strong><br><span style="font-size:0.75rem; color:var(--text-faint)">${t.operation}</span></td>
            <td style="font-size:0.8rem">${t.expected_outcome}</td>
            <td style="font-size:0.8rem">${t.actual_outcome}</td>
            <td class="code-cell">${t.actual_error_code}</td>
            <td>${t.passed ? '<span class="badge badge-success">PASS</span>' : '<span class="badge badge-danger">FAIL</span>'}</td>
          </tr>
        `).join('');

        resContainer.innerHTML = `
          <div class="result-card">
            <div class="result-card-header">
              <h3>Suite Summary (${data.passed_tests} / ${data.total_tests} Passed)</h3>
              <span class="badge ${data.failed_tests === 0 ? 'badge-success' : 'badge-danger'}">
                ${data.failed_tests === 0 ? 'ALL PASSED' : data.failed_tests + ' FAILED'}
              </span>
            </div>
            <div class="data-table-container">
              <table class="data-table">
                <thead>
                  <tr>
                    <th>Test ID</th>
                    <th>Name / Operation</th>
                    <th>Expected Outcome</th>
                    <th>Actual Outcome</th>
                    <th>OS Code</th>
                    <th>Status</th>
                  </tr>
                </thead>
                <tbody>${rows}</tbody>
              </table>
            </div>
          </div>
        `;
      } catch (err) {
        resContainer.innerHTML = `
          <div class="result-card" style="border-color:var(--accent-red)">
            <h4 style="color:var(--accent-red)">Test Suite Error</h4>
            <p style="margin-top:8px;">${err.message}</p>
          </div>
        `;
      }
    });
  },

  // 7. EXECUTION HISTORY
  async renderHistory(container) {
    container.innerHTML = `
      <div class="result-card" style="margin-top:0; margin-bottom: 20px;">
        <div style="display:flex; justify-content:space-between; align-items:center;">
          <div>
            <h3>Persistent Execution History</h3>
            <p style="font-size:0.85rem; color:var(--text-muted); margin-top:4px;">
              All records retrieved live from backend SQLite database.
            </p>
          </div>
          <button class="btn btn-secondary" id="btnRefreshHistory">Refresh Records</button>
        </div>
      </div>
      <div id="historyTableContainer">Loading...</div>
    `;

    const loadRecords = async () => {
      const el = document.getElementById('historyTableContainer');
      try {
        const hist = await API.getHistory();
        if (!hist.executions || hist.executions.length === 0) {
          el.innerHTML = `<div class="result-card"><div class="empty-state"><p>No execution data available. Select an operation to begin.</p></div></div>`;
          return;
        }

        let rows = hist.executions.map(e => `
          <tr>
            <td class="code-cell">${e.execution_id}</td>
            <td>${e.timestamp}</td>
            <td><strong>${e.operation}</strong></td>
            <td class="code-cell">${e.native_api}</td>
            <td class="code-cell">${e.native_error_code}</td>
            <td>${e.error_category}</td>
            <td>${e.success ? '<span class="badge badge-success">SUCCESS</span>' : '<span class="badge badge-danger">FAILED</span>'}</td>
            <td>${e.recovery_status === 'RECOVERED' ? '<span class="badge badge-success">RECOVERED</span>' : e.recovery_status}</td>
            <td>
              <button class="btn btn-secondary" style="padding:4px 8px; font-size:0.75rem;" onclick="App.inspectById('${e.execution_id}')">
                Inspect
              </button>
            </td>
          </tr>
        `).join('');

        el.innerHTML = `
          <div class="data-table-container">
            <table class="data-table">
              <thead>
                <tr>
                  <th>Execution ID</th>
                  <th>Timestamp</th>
                  <th>Operation</th>
                  <th>Native API</th>
                  <th>Error</th>
                  <th>Category</th>
                  <th>Status</th>
                  <th>Recovery</th>
                  <th>Action</th>
                </tr>
              </thead>
              <tbody>${rows}</tbody>
            </table>
          </div>
        `;
      } catch (err) {
        el.innerHTML = `<div class="result-card" style="border-color:var(--accent-red)"><p>${err.message}</p></div>`;
      }
    };

    document.getElementById('btnRefreshHistory').addEventListener('click', loadRecords);
    loadRecords();
  },

  // 8. AUDIT LOGS
  async renderAuditLogs(container) {
    container.innerHTML = `
      <div class="result-card" style="margin-top:0; margin-bottom: 20px;">
        <div style="display:flex; justify-content:space-between; align-items:center;">
          <div>
            <h3>Audit Log Trail</h3>
            <p style="font-size:0.85rem; color:var(--text-muted); margin-top:4px;">
              System event audit trail for forensic compliance.
            </p>
          </div>
          <a href="${API.getExportCsvUrl()}" download class="btn btn-primary" id="btnExportCsv">
            <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path><polyline points="7 10 12 15 17 10"></polyline><line x1="12" y1="15" x2="12" y2="3"></line></svg>
            Export Audit CSV
          </a>
        </div>
      </div>
      <div id="auditLogContainer">Loading...</div>
    `;

    try {
      const hist = await API.getLogs();
      const el = document.getElementById('auditLogContainer');
      if (!hist.executions || hist.executions.length === 0) {
        el.innerHTML = `<div class="result-card"><div class="empty-state"><p>No audit records available in database.</p></div></div>`;
        return;
      }

      let rows = hist.executions.map(e => `
        <tr>
          <td class="code-cell">${e.execution_id}</td>
          <td>${e.timestamp}</td>
          <td>${e.operation}</td>
          <td class="code-cell">${e.native_api}</td>
          <td>${e.description}</td>
          <td>${e.elapsed_us} µs</td>
        </tr>
      `).join('');

      el.innerHTML = `
        <div class="data-table-container">
          <table class="data-table">
            <thead>
              <tr>
                <th>Execution ID</th>
                <th>Timestamp</th>
                <th>Operation</th>
                <th>API</th>
                <th>Description</th>
                <th>Latency</th>
              </tr>
            </thead>
            <tbody>${rows}</tbody>
          </table>
        </div>
      `;
    } catch (err) {
      console.error(err);
    }
  },

  // 9. SYSTEM INFORMATION
  async renderSystemInfo(container) {
    container.innerHTML = `<div class="result-card"><div class="spinner"></div> Reading native OS environment...</div>`;
    try {
      const [info, comp] = await Promise.all([
        API.getSystemInfo(),
        API.getCompatibility()
      ]);

      container.innerHTML = `
        <div style="display:grid; grid-template-columns: 1fr 1fr; gap: 24px;">
          <div class="result-card" style="margin-top:0;">
            <h3 style="margin-bottom:16px;">Host Operating System</h3>
            <div class="result-grid" style="grid-template-columns: 1fr;">
              <div class="result-item"><span class="k">Operating System</span><span class="v">${info.os_name}</span></div>
              <div class="result-item"><span class="k">Kernel / Build</span><span class="v">${info.os_version}</span></div>
              <div class="result-item"><span class="k">Architecture</span><span class="v">${info.architecture}</span></div>
              <div class="result-item"><span class="k">Hostname</span><span class="v">${info.hostname}</span></div>
              <div class="result-item"><span class="k">Process ID / Thread</span><span class="v">PID: ${info.process_id} | TID: ${info.thread_id}</span></div>
            </div>
          </div>

          <div class="result-card" style="margin-top:0;">
            <h3 style="margin-bottom:16px;">Hardware & Compiler Environment</h3>
            <div class="result-grid" style="grid-template-columns: 1fr;">
              <div class="result-item"><span class="k">C Compiler</span><span class="v">${info.compiler_info}</span></div>
              <div class="result-item"><span class="k">Physical CPU Cores</span><span class="v">${info.cpu_cores}</span></div>
              <div class="result-item"><span class="k">Total System Memory</span><span class="v">${info.total_ram_mb} MB</span></div>
              <div class="result-item"><span class="k">Available Memory</span><span class="v">${info.free_ram_mb} MB</span></div>
              <div class="result-item"><span class="k">Cross-Platform Status</span><span class="v"><span class="badge badge-success">ACTIVE (${comp.current_platform})</span></span></div>
            </div>
          </div>
        </div>
      `;
    } catch (err) {
      container.innerHTML = `<div class="result-card" style="border-color:var(--accent-red)"><p>${err.message}</p></div>`;
    }
  },

  // 10. SETTINGS
  async renderSettings(container) {
    container.innerHTML = `<div class="result-card"><div class="spinner"></div> Loading backend settings...</div>`;
    try {
      const s = await API.getSettings();
      container.innerHTML = `
        <div class="result-card" style="margin-top:0; max-width:600px;">
          <h3 style="margin-bottom:16px;">Backend Server Configuration</h3>
          <div class="result-grid" style="grid-template-columns: 1fr;">
            <div class="result-item"><span class="k">Binding Host</span><span class="v">${s.host} (Localhost Only)</span></div>
            <div class="result-item"><span class="k">HTTP Port</span><span class="v">${s.port}</span></div>
            <div class="result-item"><span class="k">SQLite Database File</span><span class="v">${s.db_path}</span></div>
            <div class="result-item"><span class="k">Safe Sandboxed Temp Dir</span><span class="v">${s.temp_dir}</span></div>
            <div class="result-item"><span class="k">Verbose Audit Logging</span><span class="v">${s.verbose_logging ? 'ENABLED' : 'DISABLED'}</span></div>
          </div>
        </div>
      `;
    } catch (err) {
      container.innerHTML = `<div class="result-card" style="border-color:var(--accent-red)"><p>${err.message}</p></div>`;
    }
  },

  // Helpers
  displayExecutionResult(containerId, res) {
    const el = document.getElementById(containerId);
    if (!el) return;

    el.innerHTML = `
      <div class="result-card" style="margin-top:0; border-color:${res.success ? 'var(--border-color)' : 'rgba(239, 68, 68, 0.4)'}">
        <div class="result-card-header">
          <div>
            <h3>${res.operation}</h3>
            <span style="font-size:0.75rem; color:var(--text-faint)">Execution ID: ${res.execution_id}</span>
          </div>
          <span class="badge ${res.success ? 'badge-success' : 'badge-danger'}">
            ${res.success ? 'SUCCESS' : 'FAILED (ERR: ' + res.native_error_code + ')'}
          </span>
        </div>

        <div class="result-grid">
          <div class="result-item"><span class="k">Native API</span><span class="v">${res.native_api}</span></div>
          <div class="result-item"><span class="k">Return Value</span><span class="v">${res.return_value}</span></div>
          <div class="result-item"><span class="k">Native Error Code</span><span class="v">${res.native_error_code}</span></div>
          <div class="result-item"><span class="k">Error Identifier</span><span class="v">${res.error_identifier || 'NONE'}</span></div>
          <div class="result-item"><span class="k">Error Category</span><span class="v">${res.error_category}</span></div>
          <div class="result-item"><span class="k">Execution Time</span><span class="v">${res.elapsed_us} µs</span></div>
        </div>

        <div style="margin-top:16px;">
          <span class="result-item"><span class="k">Operating System Message</span></span>
          <p style="margin-top:4px; font-size:0.9rem; font-family:var(--font-mono); color:var(--accent-cyan);">${res.description}</p>
        </div>

        ${!res.success ? `
          <div style="margin-top:16px; padding:12px; background:rgba(239, 68, 68, 0.08); border-radius:6px; border:1px solid rgba(239, 68, 68, 0.2)">
            <span class="result-item"><span class="k" style="color:var(--accent-red)">Diagnosed Root Cause</span></span>
            <p style="margin-top:4px; font-size:0.85rem;">${res.possible_cause}</p>

            <span class="result-item" style="margin-top:10px;"><span class="k" style="color:var(--accent-amber)">Recommended Recovery Strategy</span></span>
            <p style="margin-top:4px; font-size:0.85rem;">${res.suggested_recovery}</p>

            <div style="margin-top:12px; display:flex; gap:10px;">
              <button class="btn btn-secondary" style="font-size:0.8rem;" onclick="App.goToRecovery('${res.execution_id}')">
                Open Recovery Center
              </button>
            </div>
          </div>
        ` : ''}
      </div>
    `;
  },

  goToRecovery(id) {
    this.loadSection('recovery');
    const input = document.getElementById('recExecIdInput');
    if (input) input.value = id;
  },

  inspectById(id) {
    this.loadSection('error-analyzer');
    const input = document.getElementById('analyzeIdInput');
    if (input) {
      input.value = id;
      document.getElementById('btnAnalyze').click();
    }
  }
};

document.addEventListener('DOMContentLoaded', () => App.init());
