# editor_v5 — Rich-Text Exam Editor with Typing Analytics

A Qt-based rich-text exam editor with integrated real-time keystroke analysis, scientific calculator, session persistence, automated diagnostic feedback, and anonymized data export for ML training. Designed as a typing trainer for exam preparation.

Built with **Qt 6 / C++17**, using QTextEdit for rich-text editing, QtCharts for visualization, and SQLite for session storage.

---

## Features

### Editor (20 Toolbar Functions)

| Function | Description |
|---|---|
| Undo / Redo | Full undo/redo history |
| Cut / Copy / Paste | Clipboard operations |
| Page Break | Insert page break for print/PDF |
| Highlight Color | Background color picker |
| Bold / Italic / Underline | Character formatting |
| Superscript | X² style formatting |
| Alignment | Left, Center, Right, Justify |
| Indent | Increase block indent |
| Non-Breaking Space | Insert U+00A0 |
| Special Characters | Dialog with 70 characters |
| Table | Insert table with row/column dialog |
| PDF Preview | Full document print preview with correction margin |

### Correction Margin (Korrekturrand)

Right-side correction margin (1/3 of page width) with red separator line, visible in editor and PDF print. Configurable via `setCorrectionMarginRatio()`.

### Scientific Calculator

Accessible via Header > Werkzeuge > Rechner. Supports basic operations, trigonometry (sin/cos/tan with Deg/Rad), logarithms, roots, powers, memory functions and more.

### Typing Analytics (Header Menu "Auswertung")

All keystrokes are tracked in real-time:

- **Performance Dashboard** — WPM, Accuracy, KSPC with color-coded cards
- **Detailed Statistics** — Backspace categorization, pause analysis, legal character timing
- **Error Pie Chart** — Motoric / automatization / content error breakdown
- **Keyboard Heatmap** — QWERTZ layout with error frequency coloring
- **WPM Over Time** — Typing speed line chart per minute
- **Recommendations** — Auto-generated training tips

### Session Persistence (SQLite)

- **Save sessions** to local SQLite database
- **Session history** table with all metrics
- **Trend analysis** with rule-based diagnostic system detecting improvement, decline, and chronic patterns

### Anonymized Data Export

Export all session metrics as a JSON file for research and ML training. Contains only numerical metrics — no typed text, no personal data. Accessible via Auswertung > Daten exportieren (anonym).

### Bug Reporting

Built-in dialog that opens a pre-filled GitHub Issue in the browser with system info attached.

---

## Prerequisites

| Dependency | Version | Notes |
|---|---|---|
| **Qt** | 6.5+ (tested with 6.10.2) | Modules: Core, Gui, Widgets, PrintSupport, Charts, Sql |
| **CMake** | 3.16+ | |
| **C++ Compiler** | C++17 compatible | MinGW, GCC, Clang, or MSVC |

### Platform-Specific Requirements

**Windows:** Qt 6 with MinGW 64-bit or MSVC. Install via [Qt Online Installer](https://www.qt.io/download-qt-installer).

**Linux (Ubuntu/Debian):**
```bash
sudo apt install qt6-base-dev qt6-charts-dev libqt6sql6-sqlite libqt6printsupport6 cmake g++
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

### 1. Clone

```bash
git clone https://github.com/Max-A92/editor-v3.git
cd editor-v3
```

### 2. Build

#### Qt Creator (All Platforms)

1. Open `editor_v5/CMakeLists.txt`
2. Select Qt 6 kit
3. Build & Run (Ctrl+R / Cmd+R)

#### Windows (MinGW)

```cmd
cd editor_v5
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

#### Linux / macOS

```bash
cd editor_v5
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64
cmake --build . -j$(nproc)
```

macOS with Homebrew:
```bash
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)
```

### 3. Run

```bash
# Windows
build\editor_v5.exe

# Linux / macOS
./build/editor_v5
```

---

## Project Structure

```
editor_v5/
├── CMakeLists.txt              # Build config (Core, Gui, Widgets, PrintSupport, Charts, Sql)
├── main.cpp                    # Entry point, Fusion style, light palette
├── mainwindow.h / .cpp         # Header, toolbar, analysis dialogs, data export
├── richtexteditor.h / .cpp     # Paper-style editor + keystroke tracking + correction margin
├── ngramanalyzer.h / .cpp      # Analysis engine (WPM, KSPC, backspace heuristics, n-grams)
├── ngramtablemodel.h / .cpp    # Table model for n-gram statistics
├── calculatordialog.h / .cpp   # Scientific calculator
├── databasemanager.h / .cpp    # SQLite persistence + trend analysis + diagnostics
├── bugreportdialog.h / .cpp    # GitHub Issues integration
├── specialchardialog.h / .cpp  # Special character picker
└── tabledialog.h / .cpp        # Table insert dialog
```

---

## How It Works

### Keystroke Tracking

Captures every keypress/release with millisecond timestamps via `QElapsedTimer`. Computes:

- **Gross/Net WPM** — Raw and error-adjusted (5 chars = 1 word)
- **Accuracy** — Correct keystrokes / total keystrokes
- **KSPC** — Total keystrokes / final characters (1.0 = perfect)
- **Backspace Categorization** — By reaction time: <200ms motoric, 200-1000ms automatization, >1000ms content
- **Pause Detection** — IKI > 500ms with context
- **N-Gram Analysis** — Timing stats for 2-7 character sequences

### Data Export Format

The JSON export contains only aggregated metrics per session:

```json
{
  "app": "editor_v5",
  "sessions": [
    {
      "date": "2026-03-11T14:00:00",
      "gross_wpm": 45.3,
      "accuracy": 0.912,
      "kspc": 1.15,
      "motoric_rate": 35.0,
      "automatization_rate": 40.0,
      "content_rate": 25.0
    }
  ],
  "summary": { "avg_wpm": 42.8, "wpm_trend": 3.2 }
}
```

No typed text is stored or exported.

---

## Roadmap

- [x] Rich-text editor with 20 toolbar functions
- [x] Real-time keystroke analytics
- [x] Scientific calculator
- [x] Correction margin (editor + PDF)
- [x] SQLite session persistence
- [x] Rule-based diagnostic system with trend analysis
- [x] Bug report via GitHub Issues
- [x] Anonymized data export (JSON)
- [ ] Raw keystroke timing export for ML training
- [ ] ONNX model integration for real-time error classification
- [ ] Export statistics as PDF/CSV
- [ ] Session comparison reports

---

## Technical Notes

- **Qt 6.10 Compatibility:** `char16_t` to `int` cast fix in `specialchardialog.cpp`
- **Dark Theme Handling:** Forces light palette via `QPalette` in `main.cpp`
- **Paper-Style Rendering:** Custom `paintEvent` for white page + correction margin on gray background
- **Recursion Guard:** `m_inLayoutUpdate` flag prevents resize event loops

---

## License

This project is licensed under the **GNU General Public License v3.0** (GPL-3.0).

You are free to use, modify, and distribute this software under the terms of the GPL v3. Any derivative work must also be distributed under the same license.

See [LICENSE](LICENSE) for the full license text.

---

## Notice

```
editor_v5 — Rich-Text Exam Editor with Typing Analytics
Copyright (C) 2025–2026 Max-A92

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
```

### Third-Party Components

This project uses the following open-source libraries:

| Component | License |
|---|---|
| [Qt 6](https://www.qt.io/) | LGPL v3 / GPL v3 / Commercial |
| [SQLite](https://www.sqlite.org/) | Public Domain |

---

## Author

**Max-A92** — [GitHub](https://github.com/Max-A92)
