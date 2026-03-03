# editor_v3 — Rich-Text Exam Editor with Typing Analytics

A Qt-based rich-text exam editor with integrated real-time keystroke analysis, designed for typing performance evaluation in exam-like scenarios.

Built with **Qt 6 / C++**, using QTextEdit for rich-text editing and QtCharts for data visualization.

---

## Features

### Editor (20 Toolbar Functions)

| Function | Description |
|---|---|
| Undo / Redo | Full undo/redo history |
| Cut / Copy / Paste | Clipboard operations |
| Page Break | Insert page break for print/PDF |
| Highlight Color | Background color picker |
| **Bold** / *Italic* / Underline | Character formatting |
| Superscript | X² style formatting |
| Alignment | Left, Center, Right, Justify |
| Indent | Increase block indent |
| Non-Breaking Space | Insert U+00A0 |
| Special Characters | Dialog with 70 characters (§, €, ©, Ä, Ö, Ü, ß, …) |
| Table | Insert table with row/column dialog |
| PDF Preview | Full document print preview |

### Typing Analytics (Header Menu "Auswertung")

All keystrokes are tracked in real-time while typing. The analysis menu provides:

- **Performance Dashboard** — Three metric cards showing WPM (Words Per Minute), Accuracy, and KSPC (Keystrokes Per Character) with color-coded ratings
- **Detailed Statistics** — Session duration, backspace categorization (motoric / automatization / content), pause analysis, legal character timing (§, ä, ö, ü, ß)
- **Error Pie Chart** — Visual breakdown of error types based on reaction time heuristics
- **Keyboard Heatmap** — QWERTZ layout with color-coded error frequency per key
- **WPM Over Time** — Line chart showing typing speed progression per minute
- **Recommendations** — Automatically generated training tips based on detected weaknesses

---

## Screenshots

*Coming soon*

---

## Prerequisites

| Dependency | Version | Notes |
|---|---|---|
| **Qt** | 6.5+ (tested with 6.10.2) | Modules: Core, Gui, Widgets, PrintSupport, Charts |
| **CMake** | 3.16+ | |
| **C++ Compiler** | C++17 compatible | MinGW, GCC, Clang, or MSVC |

### Platform-Specific Requirements

**Windows:** Qt 6 with MinGW 64-bit or MSVC. Recommended: Install via [Qt Online Installer](https://www.qt.io/download-qt-installer).

**Linux (Ubuntu/Debian):**
```bash
sudo apt install qt6-base-dev qt6-charts-dev libqt6printsupport6 cmake g++
```

**Linux (Fedora):**
```bash
sudo dnf install qt6-qtbase-devel qt6-qtcharts-devel cmake gcc-c++
```

**macOS:**
```bash
brew install qt@6 cmake
```

---

## Build Instructions

### 1. Clone the Repository

```bash
git clone https://github.com/Max-A92/editor-v3.git
cd editor-v3
```

### 2. Build

#### Option A: Qt Creator (All Platforms)

1. Open `editor_v3/CMakeLists.txt` in Qt Creator
2. Select your Qt 6 kit (e.g. *Desktop Qt 6.10.2 MinGW 64-bit*)
3. Click **Build & Run** (Ctrl+R / Cmd+R)

#### Option B: Command Line — Windows (MinGW)

```cmd
cd editor_v3
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

Make sure the Qt and MinGW `bin` directories are in your `PATH`, or run from the **Qt MinGW terminal**.

#### Option C: Command Line — Linux / macOS

```bash
cd editor_v3
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

If Qt is not found automatically, specify the path:

```bash
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64
```

On macOS with Homebrew:

```bash
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)
```

### 3. Run

```bash
# Windows
build\editor_v3.exe

# Linux / macOS
./build/editor_v3
```

---

## Project Structure

```
editor_v3/
├── CMakeLists.txt              # Build configuration
├── main.cpp                    # Entry point, Fusion style, light palette
├── mainwindow.h / .cpp         # Header bar, toolbar, analysis dialogs
├── richtexteditor.h / .cpp     # Paper-style QTextEdit + keystroke tracking
├── ngramanalyzer.h / .cpp      # Analysis engine (WPM, KSPC, backspace heuristics)
├── ngramtablemodel.h / .cpp    # Table model for n-gram statistics
├── specialchardialog.h / .cpp  # Special character picker (70 characters)
├── tabledialog.h / .cpp        # Table insert dialog
└── README.md
```

---

## How It Works

The editor captures every keypress and key release event with millisecond timestamps via `QElapsedTimer`. This data feeds into the `NgramAnalyzer` which computes:

- **Gross/Net WPM** — Raw and error-adjusted typing speed (5 characters = 1 word)
- **Accuracy** — Ratio of correct keystrokes to total keystrokes
- **KSPC** — Total keystrokes divided by final characters (1.0 = perfect, >1.3 = many corrections)
- **Backspace Categorization** — Heuristic classification based on reaction time:
  - < 200ms: motoric error (reflexive mistype)
  - 200–1000ms: automatization error (key search)
  - > 1000ms: content error (spelling uncertainty)
- **Pause Detection** — Identifies inter-keystroke intervals > 500ms with surrounding context
- **N-Gram Analysis** — Timing statistics for character sequences (configurable 2–7 grams)

> **Note:** The backspace categorization is a heuristic estimate (~60–70% accuracy) based on HCI research thresholds (Soukoreff & MacKenzie, 2003). The actual cause of an error can only be determined by the user.

---

## Roadmap

- [ ] SQLite persistence for session history and progress tracking
- [ ] Multi-session trend analysis and comparison
- [ ] Rule-based diagnostic system with personalized training plans
- [ ] ML-based error classification (ONNX integration)
- [ ] Export statistics as PDF/CSV

---

## Technical Notes

- **Qt 6.10 Compatibility:** A char16_t to int cast fix is applied in `specialchardialog.cpp` for `QString::arg()` template resolution
- **Dark Theme Handling:** The app forces a light palette via `QPalette` in `main.cpp` to prevent Windows 11 dark mode inheritance
- **Paper-Style Rendering:** Custom `paintEvent` draws a white page on gray background; `setViewportMargins` + `document()->setTextWidth()` handle centered layout with proper word wrapping

---

## License

*No license specified yet.*

---

## Author

**Max-A92** — [GitHub](https://github.com/Max-A92)
