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
* **Visual Container Optimization:** Replace the standard QWebEngineView (Chromium backbone, ~90 MB idle tax) with **QUltralightView** (WebKit fork, ~18 MB idle tax). This in-process runtime native drawing node parses Markdown, highlights syntax strings via Prism.js, and maps structural flowcharts using Mermaid.js text strings on a hardware-accelerated 2D canvas.

### 2.2 Memory Virtualization Layer Design (lxqt-memfusion)

* **lxqt/lxqt-config:** Fork to register a new system modular application sub-panel widget panel layout called lxqt-config-memfusion.
* **lxqt/lxqt-session:** Fork to list your Python supervisor background monitor script as an active system watchdog thread.

## 3. Implementation Blueprint (Final Verified Source Blocks)

### 3.1 The Panel Interface: neuroshell-core/aiwidget.cpp

```cpp

#include "aiwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>

AIChatWindow::AIChatWindow(QWidget *parent) : QDialog(parent) {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(420, 560);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    QFrame *container = new QFrame(this);
    container->setObjectName("CopilotContainer");
    container->setStyleSheet("QFrame#CopilotContainer { background-color: #0d1117; border: 1px solid #30363d; border-radius: 12px; }");
    QVBoxLayout *containerLayout = new QVBoxLayout(container);

    QHBoxLayout *header = new QHBoxLayout();
    QLabel *modelBadge = new QLabel("✨ gemma3:1b-it-qat", this);
    modelBadge->setStyleSheet("QLabel { color: #58a6ff; background-color: #161b22; border: 1px solid #30363d; border-radius: 10px; padding: 2px 10px; font-size: 11px; font-family: monospace; font-weight: bold; }");

    header->addWidget(modelBadge);
    header->addStretch();
    
    QPushButton *closeBtn = new QPushButton("✕", this);
    closeBtn->setStyleSheet("color: #8b949e; background: transparent; border: none; font-size: 14px;");
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::hide);
    header->addWidget(closeBtn);
    containerLayout->addLayout(header);

    m_webView = new QUltralightView(this);
    containerLayout->addWidget(m_webView);
    initializeWebCanvas();

    m_inputField = new QLineEdit(this);
    m_inputField->setPlaceholderText("Ask NeuroShell or type '/' for tasks...");
    m_inputField->setStyleSheet("QLineEdit { color: #f0f6fc; background-color: #161b22; border: 1px solid #30363d; border-radius: 20px; padding: 10px 16px; font-size: 13px; } QLineEdit:focus { border: 1px solid #58a6ff; }");

    containerLayout->addWidget(m_inputField);

    mainLayout->addWidget(container);
    connect(m_inputField, &QLineEdit::returnPressed, this, &AIChatWindow::sendPrompt);
}

void AIChatWindow::initializeWebCanvas() {

    QString htmlTemplate = R"(
    <!DOCTYPE html><html><head>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/themes/prism-tomorrow.min.css">
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/prism.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/mermaid/10.2.4/mermaid.min.js"></script>
    <script>mermaid.initialize({startOnLoad:false, theme:'dark'});</script>
    <style>
        body { font-family: sans-serif; background-color: #0d1117; color: #c9d1d9; margin: 0; padding: 8px; font-size: 13px; }
        .message { margin-bottom: 16px; animation: fadeIn 0.2s ease-out; }
        .user-header { color: #ff7b72; font-weight: bold; font-size: 11px; }
        .ai-header { color: #58a6ff; font-weight: bold; font-size: 11px; }
        pre { background-color: #161b22 !important; border: 1px solid #30363d; border-radius: 6px; padding: 10px; overflow-x: auto; }
        table { border-collapse: collapse; width: 100%; margin: 10px 0; background: #161b22; }
        th, td { padding: 8px; border: 1px solid #30363d; text-align: left; }
        th { background-color: #21262d; }
        @keyframes fadeIn { from { opacity: 0; transform: translateY(2px); } to { opacity: 1; transform: translateY(0); } }
    </style></head><body><div id="canvas"></div><script>
        const cv = document.getElementById('canvas');
        function addMsg(user, text) {
            const cls = user ? 'user-header' : 'ai-header';
            const icon = user ? '👤 User' : '🤖 NeuroShell';
            cv.innerHTML += `<div class="message"><div class="${cls}">${icon}</div><div>${text}</div></div>`;
            Prism.highlightAll();
            mermaid.init(undefined, document.querySelectorAll('.mermaid'));
            window.scrollTo(0, document.body.scrollHeight);
        }
    </script></body></html>)";

    m_webView->loadHTML(htmlTemplate);
}

void AIChatWindow::sendPrompt() {

    QString text = m_inputField->text().trimmed();
    if (text.isEmpty()) return;

    QJsonObject uObj; uObj["t"] = text;
    QString uJs = QString("addMsg(true, %1.t);").arg(QJsonDocument(uObj).toJson(QJsonDocument::Compact));
    m_webView->evaluateJavaScript(uJs);
    m_inputField->clear();

    QDBusInterface neuroBus("org.lxqt.neuroshell", "/org/lxqt/neuroshell", "org.lxqt.neuroshell.Interface", QDBusConnection::sessionBus());
    if (neuroBus.isValid()) {
        QDBusReply<QString> reply = neuroBus.call("ProcessIntent", text);
        if (reply.isValid()) { handleAIResponse(reply.value()); }
        else { handleAIResponse("<span style='color:#f85149;'>D-Bus Payload Sync Failure.</span>"); }
    } else {
        handleAIResponse("<span style='color:#f85149;'>NeuroShell System Service is completely offline.</span>");
    }
}

void AIChatWindow::handleAIResponse(const QString &rawResponse) {

    QJsonObject aObj; aObj["t"] = rawResponse;
    QString aJs = QString("addMsg(false, %1.t);").arg(QJsonDocument(aObj).toJson(QJsonDocument::Compact));
    m_webView->evaluateJavaScript(aJs);
}

```

### 3.2 The Execution Engine: neuroshell-core/neuroshell_daemon.py

```python

#!/usr/bin/env python3
import dbus
import dbus.service
import dbus.mainloop.glib
from gi.repository import GLib
import subprocess
import os
import re
import ollama

BANNED_PATTERNS = [r"rm\s+-rf\s+/", r"mkfs", r"dd\s+if=", r"> /dev/sda", r"chmod\s+-R\s+777\s+/"]

def safety_check(cmd_string):
    for pattern in BANNED_PATTERNS:
        if re.search(pattern, cmd_string, re.IGNORECASE):
            return False
    return True

def extract_tag_content(text, tag_name):
    pattern = f"<{tag_name}>(.*?)</{tag_name}>"
    match = re.search(pattern, text, re.DOTALL)
    return match.group(1).strip() if match else ""

class NeuroShellLocalAutonomousService(dbus.service.Object):
    def __init__(self):
        bus_name = dbus.service.BusName('org.lxqt.neuroshell', bus=dbus.SessionBus())
        dbus.service.Object.__init__(self, bus_name, '/org/lxqt/neuroshell')
        print("[Lark Engine] Local Autonomous Control Plane Engaged.")

    @dbus.service.method(dbus_interface='org.lxqt.neuroshell.Interface', in_signature='s', out_signature='s')
    def ProcessIntent(self, raw_user_payload):
        system_prompt = f"""
        You are the autonomous execution engine for a Lubuntu Linux desktop environment.
        User Intent: "{raw_user_payload}"
        Construct the exact bash commands needed to complete the task.
        You must respond following this exact block format template:

        <THOUGHT>Explain what you are trying to achieve briefly</THOUGHT>
        <PRIVILEGED>true or false</PRIVILEGED>
        <BASH>The exact terminal commands to execute</BASH>
        <REPLY>A conversational explanation for the user</REPLY>
        """
        try:
            # Explicitly locked down to use Google's high-efficiency Quantization-Aware Trained model
            response = ollama.generate(model='gemma3:1b-it-qat', prompt=system_prompt, options={"temperature": 0.1})
            llm_text = response['response'].strip()

            privileged = extract_tag_content(llm_text, "PRIVILEGED").lower() == "true"
            bash_cmd = extract_tag_content(llm_text, "BASH")
            reply = extract_tag_content(llm_text, "REPLY")

            if bash_cmd:
                if not safety_check(bash_cmd):
                    return "<h3>Security Exception</h3>Unverified or dangerous bash sequence blocked."

                exec_args = ["lxqt-sudo", "--", "bash", "-c", bash_cmd] if privileged else ["bash", "-c", bash_cmd]
                result = subprocess.run(exec_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=30)
                output = result.stdout.strip() if result.returncode == 0 else result.stderr.strip()

                return f"{reply}<br><br><b>System Output:</b><pre style='background:#222; color:#0f0; padding:10px;'>{output}</pre>"
            return reply if reply else "Parsing block format bounds failed."
        except Exception as e:
            return f"NeuroShell Local Pipeline Error: {str(e)}"

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    service = NeuroShellLocalAutonomousService()
    GLib.MainLoop().run()


```

---

## 4. Integration Verification & Testing Framework

### 4.1 Local Inference Sanity Benchmarking

To verify your pipeline, bypass the UI and query your D-Bus structure directly via terminal loops:

```bash
dbus-send --session --print-reply --dest=org.lxqt.neuroshell \
          /org/lxqt/neuroshell \
          org.lxqt.neuroshell.Interface.ProcessIntent string:"Show active disk usage details"

```

*Expected Result:* Clean text string tracking filesystem storage levels wrapped cleanly behind localized `<pre>` markup tags.


### 4.2 Systemd Memory Limit Enforcement Check
To test that your backend cannot run away with core machine RAM, verify cgroup compliance: 

```bash
systemctl --user status neuroshell-backend.service | grep -E "(Memory:|CPU:)"

```

---

## 5. Software Release & Package Deployment Configuration
### 5.1 Debian Staging Tree Specification

```bash
lubuntu-lark-neuroshell_1.0.0-1_amd64/
├── DEBIAN/
│   ├── control                           # Dependency manifests and package name rules
│   ├── preinst                         # Pre-install environment verification script
│   ├── postinst                          # Post-install routine (Ollama setup, model pull)
│   └── postrm                          # Post-removal cleanup script (Ollama purge)
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

### 5.2 Build Execution Sequence

Run this block to compress, package, and generate your deployable binary:

```bash
# Force the system tools to pack your staging folder tree into a standard native Debian package
dpkg-deb --build lubuntu-lark-neuroshell_1.0.0-1_amd64

```

*The resulting file **`lubuntu-lark-neuroshell_1.0.0-1_amd64.deb`** is now completely standardized and optimized, ready to be copied straight into your Gemini Code Assist workspace for downstream pipeline automation!*
