const API = {
  baseUrl: '',

  async getHealth() {
    const res = await fetch(`${this.baseUrl}/api/health`);
    if (!res.ok) throw new Error(`Health check failed: ${res.status}`);
    return res.json();
  },

  async getSystemInfo() {
    const res = await fetch(`${this.baseUrl}/api/system/info`);
    if (!res.ok) throw new Error(`System info failed: ${res.status}`);
    return res.json();
  },

  async getStatistics() {
    const res = await fetch(`${this.baseUrl}/api/statistics`);
    if (!res.ok) throw new Error(`Statistics failed: ${res.status}`);
    return res.json();
  },

  async getCompatibility() {
    const res = await fetch(`${this.baseUrl}/api/system/compatibility`);
    if (!res.ok) throw new Error(`Compatibility failed: ${res.status}`);
    return res.json();
  },

  async executeFileOp(payload) {
    const res = await fetch(`${this.baseUrl}/api/file/execute`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });
    if (!res.ok) throw new Error(`File operation failed: ${res.status}`);
    return res.json();
  },

  async executeProcessOp(payload) {
    const res = await fetch(`${this.baseUrl}/api/process/execute`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });
    if (!res.ok) throw new Error(`Process operation failed: ${res.status}`);
    return res.json();
  },

  async analyzeError(executionId) {
    const res = await fetch(`${this.baseUrl}/api/error/analyze`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ execution_id: executionId })
    });
    if (!res.ok) throw new Error(`Analysis failed: ${res.status}`);
    return res.json();
  },

  async executeRecovery(executionId, action, confirmed) {
    const res = await fetch(`${this.baseUrl}/api/recovery/execute`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        execution_id: executionId,
        action: action,
        user_confirmed: confirmed
      })
    });
    if (!res.ok) throw new Error(`Recovery execution failed: ${res.status}`);
    return res.json();
  },

  async runTests(category) {
    const res = await fetch(`${this.baseUrl}/api/tests/run`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ category: category || 'all' })
    });
    if (!res.ok) throw new Error(`Test run failed: ${res.status}`);
    return res.json();
  },

  async getHistory() {
    const res = await fetch(`${this.baseUrl}/api/history`);
    if (!res.ok) throw new Error(`History fetch failed: ${res.status}`);
    return res.json();
  },

  async getLogs() {
    const res = await fetch(`${this.baseUrl}/api/logs`);
    if (!res.ok) throw new Error(`Logs fetch failed: ${res.status}`);
    return res.json();
  },

  getExportCsvUrl() {
    return `${this.baseUrl}/api/logs/export`;
  },

  async getSettings() {
    const res = await fetch(`${this.baseUrl}/api/settings`);
    if (!res.ok) throw new Error(`Settings fetch failed: ${res.status}`);
    return res.json();
  }
};
