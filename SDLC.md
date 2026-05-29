# Software Development Life Cycle (SDLC) Specification Document
**Project Umbrella Title:** Lubuntu Lark Core Ecosystem
**Target Platform:** Lubuntu 26.04 LTS (LXQt / Qt6 Architecture)
**Core Modules:** lxqt-memfusion (Hardware Layer) & neuroshell-core (Inference Control Layer)

## 1. Requirements Specification & Architecture Definition
### 1.1 Functional Requirements
* **Asynchronous Prompt Execution:** Users must be able to query a local, unconstrained AI model through a panel-anchored lxqt-panel widget interface.
* **Autonomous Shell Generation (ReAct Paradigm):** The local model must dynamically evaluate machine state, draft its own valid Multi-line Bash scripts, and execute them safely.
* **Unified Inter-Process Communication (IPC):** Graphical frontends and background daemons must communicate with absolute efficiency using the Linux user session **D-Bus platform** to prevent process blocking.
* **Dynamic RAM Expansion Management:** The system must actively manage a compressed zRam loop device partition, backed by physical storage device spillover configurations.
* **Centralized AI Model Provisioning Store:** Users must have a clean, graphical control desk to download, monitor, and purge local weights using **Ollama’s local REST API framework**.

### 1.2 Non-Functional Requirements
* **Strict Resource Constraints:** The entire idle memory footprint of the graphical user interfaces must not exceed **~40 MB RAM**. The background processing tasks must remain constrained to an absolute ceiling of **1.2 GB RAM** during deep reasoning loops.
* **Privacy Guardrails:** Zero network dependencies or outbound data leaks; all user selections, text highlighting, and machine data analysis must remain completely offline.
* **System Integrity & Safety Filters:** Script generation must fail immediately if destructive shell regex combinations (e.g., rm -rf /) are detected.

### 1.3 Communication Protocols Matrix

```text

  +───────────────────────────────────────────────────────────────────────────+
  │                               LUBUNTU LARK                                │
  │                                                                           │
  │  +─────────────────────────+                 +─────────────────────────+  │
  │  │  lxqt-panel UI Widget   │                 │     Local LLM Manager   │  │
  │  +────────────┬────────────+                 +────────────┬────────────+  │
  │               │                                           │               │
  │               │ (Binary Payload Array)                    │ (JSON Over    │
  │               ▼                                           ▼  Localhost)   │
  │    =======================                     =======================    │
  │         D-BUS IPC BUS                             LOCAL REST API          │
  │    =======================                     =======================    │
  │               │                                           │               │
  │               ▼                                           ▼               │
  │  +─────────────────────────+                 +─────────────────────────+  │
  │  │  neuroshell_daemon.py   │                 │   Ollama Background     │  │
  │  +─────────────────────────+                 │   Service Endpoint      │  │
  │                                              +─────────────────────────+  │
  +───────────────────────────────────────────────────────────────────────────+



```

## 2. Component Design & Code Repositories Mapping

To inject these components as native citizens inside your Lubuntu platform fork, you must fork and implement modifications across the following upstream **LXQt Git repositories**:

### 2.1 Interface & Automation Layer Design (neuroshell-core)

* **lxqt/lxqt-panel:** Fork to append a new plugin subdirectory named plugin-neuroshell. This contains the visual chat window workspace layout.
* **lxqt/lxqt-globalkeys:** Fork to register a default global shortcut combination (Meta + A) tied directly to a DBus notification handler that toggles the panel interface view layer.
* **Visual Container Optimization:** Uses `QWebView` (from the Qt WebKit module) to render the UI. This provides a lightweight, low-memory footprint rendering surface suitable for resource-constrained systems. It parses Markdown and highlights syntax in code blocks using **Highlight.js**.

### 2.2 Memory Virtualization Layer Design (lxqt-memfusion)

* **lxqt/lxqt-config:** Fork to register a new system modular application sub-panel widget panel layout called lxqt-config-memfusion.
* **lxqt/lxqt-session:** Fork to list your Python supervisor background monitor script as an active system watchdog thread.

## 3. Integration Verification & Testing Framework

### 3.1 Local Inference Sanity Benchmarking

To verify your pipeline, bypass the UI and query your D-Bus structure directly via terminal loops:

```bash
dbus-send --session --print-reply --dest=org.lxqt.neuroshell \
          /org/lxqt/neuroshell \
          org.lxqt.neuroshell.Interface.ProcessIntent string:"Show active disk usage details"

```

*Expected Result:* Clean text string tracking filesystem storage levels wrapped cleanly behind localized `<pre>` markup tags.


### 3.2 Systemd Memory Limit Enforcement Check
To test that your backend cannot run away with core machine RAM, verify cgroup compliance: 

```bash
systemctl --user status neuroshell-backend.service | grep -E "(Memory:|CPU:)"

```

---

## 4. Software Release & Package Deployment Configuration
### 4.1 Debian Staging Tree Specification

```bash
lubuntu-lark-neuroshell_1.0.0-1_amd64/
├── DEBIAN/
│   ├── control                           # Dependency manifests and package name rules
│   ├── preinst                         # Pre-install environment verification script
├── usr/local/bin/
│   ├── neuroshell_daemon.py              # In-tree copy of executable daemon script
│   └── memfusion-watchdog.py             # In-tree copy of zRam optimizer script
├── etc/xdg/autostart/
│   ├── lxqt-memfusion-daemon.desktop     # Autostarts the MemFusion watchdog via lxqt-session
│   └── lxqt-neuroshell-daemon.desktop    # Autostarts the NeuroShell D-Bus daemon
└── usr/share/applications/
    ├── lxqt-config-memfusion.desktop   # Integrates MemFusion GUI into the Control Center
    └── lxqt-config-neuroshell.desktop    # Integrates NeuroShell GUI into the Control Center

```

### 4.2 Build Execution Sequence

Run this block to compress, package, and generate your deployable binary:

```bash
# Force the system tools to pack your staging folder tree into a standard native Debian package
dpkg-deb --build lubuntu-lark-neuroshell_1.0.0-1_amd64

```

*The resulting file **`lubuntu-lark-neuroshell_1.0.0-1_amd64.deb`** is now completely standardized and optimized, ready to be copied straight into your Gemini Code Assist workspace for downstream pipeline automation!*
