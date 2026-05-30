# Lubuntu Lark: NeuroShell & MemFusion Ecosystem

Lubuntu Lark is an optimized, privacy-first, autonomous AI operating system flavor built on top of the **LXQt (Qt6) Desktop Environment**. This ecosystem decouples low-level virtual memory allocation from an unconstrained, local AI control plane, allowing resource-constrained hardware to run advanced OS automation completely offline with zero network latency.

---

## 1. System Architecture & Components

The workspace is organized into two core decoupled frameworks interacting via system IPC and local REST loops:

```text
lubuntu-lark-neuroshell/
├── lxqt-memfusion/                  # Hardware Optimization Layer
│   ├── lxqt-config-memfusion/       # C++: MemFusion Settings GUI
│   └── memfusion-watchdog.py        # Python: zRam Watchdog Daemon
└── neuroshell-core/                 # Local Inference Control Layer
    ├── lxqt-plugin-neuroshell/      # C++: NeuroShell Panel Plugin
    ├── lxqt-config-neuroshell/      # C++: NeuroShell Config GUI
    └── neuroshell_daemon.py         # Python: NeuroShell Execution Daemon

```

### 1.1 `lxqt-memfusion` (Hardware Layer)

* **Dynamic zRam Scaling:** Dynamically initializes a compressed swap pool (`/dev/zram0`) utilizing high-density `zstd` compression mapped at maximum kernel priority.
* **Android-Style Writeback:** Links memory blocks to physical storage backing devices (`/sys/block/zram0/backing_dev`) to flush incompressible pages seamlessly.
* **Session Supervision:** The daemon runs as a native `X-LXQt-Module`, allowing `lxqt-session` to automatically monitor and restart the loop during extreme Out-Of-Memory (OOM) spikes.

### 1.2 `neuroshell-core` (AI Inference & Control Layer)

* **Native Qt UI:** Uses standard, lightweight Qt6 widgets (`QTextBrowser`, `QTableWidget`) to render the user interface. This provides maximum performance and the lowest possible memory footprint while ensuring a perfectly native look and feel.
* **Unconstrained ReAct Daemon:** Intercepts system payloads via local **D-Bus IPC** (`org.lxqt.neuroshell`). The backend prompts a local **`tinydolphin`** model to dynamically plan, draft, and execute multi-line Bash sequences securely through an explicit tag-matching runtime engine.
* **Model Management Hub:** A dedicated settings app querying Ollama's local REST API endpoint (`http://localhost:11434`) to provide a graphical interfaces for downloading models, inspecting real-time VRAM allocations, and purging cache arrays.

### 1.3 Configurable AI Backend

The `neuroshell_daemon.py` can be configured to use different AI backends by changing the `AI_BACKEND` variable at the top of the script.

*   **`local` (Default):** Uses a local Ollama model. This is excellent for privacy and offline use but is limited by your local hardware.
*   **`cloud` (OpenAI):** Uses the official OpenAI API. This provides extremely fast responses from powerful models like GPT-4 but requires an internet connection and a paid API key.
*   **`azure` (Azure OpenAI):** Uses Microsoft's Azure OpenAI Service. This offers the power of OpenAI's models within the Microsoft cloud ecosystem, providing another high-performance, cloud-based option.

The specific model used by each backend can also be configured via environment variables. This allows you to easily switch models without editing the code.

*   **`AI_BACKEND`**: Sets the backend to use (`local`, `cloud`, or `azure`).
*   **`OLLAMA_MODEL`**: Sets the model for the `local` backend (Default: `tinydolphin`).
*   **`OPENAI_MODEL`**: Sets the model for the `cloud` backend (Default: `gpt-4-turbo-preview`).
*   **`AZURE_OPENAI_DEPLOYMENT`**: Sets the model (deployment name) for the `azure` backend.

**Example:**
```bash
export OLLAMA_MODEL="phi-2"
```

---

## 2. Communication Matrix

| Interface Track | Protocol | Source Endpoint | Destination Endpoint | Purpose |
| --- | --- | --- | --- | --- |
| **Automation Wire** | **D-Bus IPC** | `lxqt-plugin-neuroshell` (C++) | `neuroshell_daemon.py` (Python) | High-speed, local binary prompt and output payload synchronization. |
| **Model Registry** | **REST API** | `lxqt-config-neuroshell` (C++) | Ollama Daemon (`127.0.0.1:11434`) | Managing disk usage, checking VRAM status, and downloading models via HTTP. |

---

## 3. Local Development & Compilation

### 3.1 Prerequisites

Ensure your Lubuntu host environment has the following development libraries installed:
```bash
sudo apt install liblxqt2-dev qt6-base-dev libqt6widgets6 python3-dbus python3-gi python3-ollama python3-jinja2 curl cmake build-essential
```

### 3.2 Building the Graphical Plugins

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install

```

This compiles and installs the shared objects (`.so` modules) directly into the native LXQt panel plugin and application configuration module folders.

---

## 4. Debian Packaging (`.deb` Generation)

To package the entire ecosystem into a single deployable artifact for distribution, simply run the automated build script:

```bash
# Make the script executable
chmod +x build_package.sh

# Run the build and packaging process
./build_package.sh
```

This script handles compilation, staging, permissions, and final package generation, producing a `.deb` file in the project root (e.g., `lubuntu-lark-neuroshell_1.0.0-5-git123abc_amd64.deb`).

### Installation

Deploy the package on any Lubuntu host client machine:

```bash
# The version will vary based on your git history
sudo apt install ./lubuntu-lark-neuroshell_*.deb
```

---

## 5. System Safety & Privilege Escalation Guardrails

1. **Regex Content Scanning:** The Python daemon passes all code blocks generated by the local LLM through an internal validation pass. Commands containing hazardous pattern expressions (e.g., `rm -rf /` or device sector block erasures) fail instantly with an explicit security exception.
2. **Privilege Isolation:** Minor environmental variables are checked using low-privilege standard shells. Modifying system packages (`apt`), managing system-wide configuration files, or tweaking operational states causes the model to set `<PRIVILEGED>true</PRIVILEGED>`. This wraps execution blocks through **`lxqt-sudo`**, safely validating structural permissions through standard Linux session caches.

 