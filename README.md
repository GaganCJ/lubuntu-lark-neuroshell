# Lubuntu Lark: Cloud-Hybrid Dual-Provider AI Applet

Lubuntu Lark is a highly optimized, cloud-hybrid AI taskbar widget built on top of the **LXQt (Qt6) Desktop Environment**. 

This applet replaces heavy local LLM inference setups with a low-footprint, embedded web viewport streaming Google Gemini and Microsoft Copilot. It utilizes native Qt6 components and Chromium optimizations to minimize memory and CPU usage on resource-constrained hardware.

---

## 1. System Features & Optimizations

*   **Low-Footprint Chromium Engine:** Pre-configures strict rendering and thread optimizations via `qputenv` command-line flags before initialization:
    *   `--single-process` (Clamps execution to a single background process thread)
    *   `--disable-gpu` (Disables heavy 3D acceleration and composition overhead)
    *   `--mute-audio` (Shuts down hardware audio routing)
    *   `--disable-extensions` & `--disable-notifications` (Purges background check loops)
*   **Dual Persistent Sessions:** Redirects cookie, local storage, and cache storage profiles to a custom path: `~/.config/lark/ai_profile`. This keeps both Google and Microsoft user sessions active simultaneously.
*   **Instant Provider Switching:** Hosts separate `QWebEngineView` instances inside a `QStackedWidget` for Gemini and Copilot. Users can toggle between providers instantly without reloading the page or losing current chat context.
*   **Aesthetic Integration:** Uses a custom JavaScript injection hook on page loads to inject CSS stylesheets hiding standard web banners and navigation headers, offering a clean, native desktop application appearance matching the dark VS Code GitHub Copilot theme (`#0d1117` background, subtle `#30363d` borders).

---

## 2. Compilation & Development

### 2.1 Prerequisites

Ensure your target system has the following build tools and libraries installed:
```bash
sudo apt update
sudo apt install build-essential cmake git qt6-base-dev qt6-webengine-dev liblxqt-dev lxqt-panel-dev
```

### 2.2 Compilation Steps

To compile and install the plugin module directly on your system:
```bash
# 1. Create a build directory
mkdir build && cd build

# 2. Configure using CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# 3. Compile the shared library
make

# 4. Install the library module
sudo make install
```

---

## 3. Debian Packaging (`.deb` Generation)

To generate a deployable Debian archive (`.deb`) for distribution:

```bash
# Make the script executable
chmod +x build_package.sh

# Run the build and packaging process
./build_package.sh
```

This automates compiling and packaging, outputting a package file like `lubuntu-lark-neuroshell_1.0.0-git_amd64.deb` in the project root.

### Installation

Deploy the package on your target Lubuntu system:

```bash
sudo apt install ./lubuntu-lark-neuroshell_*.deb
```

Once installed, restart the LXQt panel to register the new widget:
```bash
lxqt-panel --restart
```
Right-click your panel, choose **Configure Panel** / **Add Panel Widgets**, and add the AI Applet.