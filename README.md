# CortexOS v1.0 — Terminal Operating System Simulator

A professional, modular, terminal-based Operating System Simulator written in pure C, designed for Windows with VS Code + MinGW/GCC.

---

## ✅ Features

| Module | Description |
|--------|-------------|
| Boot System | Realistic BIOS + boot animation |
| User Auth | Signup, login, logout, passwd, admin roles |
| Virtual File System | Tree-based VFS: mkdir, touch, ls, cd, pwd, rm, mv, cp, cat, edit, chmod, chown |
| Text Editor | Multi-line, save, append, in-VFS editor |
| Process Manager | run, ps, kill — linked-list processes |
| CPU Scheduler | FCFS, SJF, Priority, Round Robin with Gantt charts |
| Memory Manager | First Fit, Best Fit, Worst Fit with visual memory map |
| Security | SHA-256 hashing, password strength analysis, file checksums |
| Logs | System log with levels: INFO, WARN, ERROR, AUTH, FS, PROC |
| Virtual Browser | cortexmail.os, cortexnews.os, cortexstore.os |
| Email System | User-to-user mail, inbox, send |
| LAN Simulator | connect, chat, disconnect |
| Database Engine | Mini SQL: CREATE TABLE, INSERT, SELECT, DELETE |
| Blockchain | Block mining, chain verification, transactions |
| Backup & Recovery | Snapshot creation, restore, list |
| Package Manager | install, remove, update, list |
| App Store | Built-in apps registry |
| Calculator | Interactive expression evaluator |
| Calendar | Monthly calendar view |
| Notes | Per-user note storage in VFS |
| Todo Manager | Task list with add/done/clear |
| Snake | Turn-based ASCII Snake game |
| Chess | ASCII chess board display |
| Sudoku | Interactive Sudoku puzzle |
| Minesweeper | Full ASCII minesweeper |
| VM Manager | Create/start/stop virtual machines |
| Command History | Stack-based history, `history` command |
| Kernel Info | Display kernel + algorithm status |

---

## 🛠️ Requirements

- **Windows 10/11**
- **MinGW-w64 (GCC)** — [Download here](https://www.mingw-w64.org/) or install via [MSYS2](https://www.msys2.org/)
- **VS Code** with C/C++ extension (Microsoft)

### Verify GCC is installed

Open a terminal (PowerShell / CMD) and run:
```
gcc --version
```
You should see a version number. If not, add MinGW's `bin` folder to your PATH.

---

## 🚀 Quick Start

### Option A — Double-click (easiest)
1. Double-click `build.bat` — compiles everything
2. Double-click `CortexOS.exe` — runs the simulator

### Option B — VS Code
1. Open the `CortexOS` folder in VS Code (`File → Open Folder`)
2. Press **Ctrl+Shift+B** to build (uses `.vscode/tasks.json`)
3. The terminal will show build output; run `CortexOS.exe`

### Option C — Terminal
```bash
cd CortexOS
gcc -Wall -Wextra -std=c99 -O2 -Isrc src/main.c src/shell.c src/users.c src/security.c src/filesystem.c src/process.c src/scheduler.c src/memory.c src/network.c src/database.c src/blockchain.c src/backup.c src/apps.c src/logs.c -o CortexOS.exe -lws2_32
CortexOS.exe
```

---

## 📁 Project Structure

```
CortexOS/
├── src/
│   ├── main.c           Boot, init, main menu
│   ├── common.h         Shared definitions, macros, globals
│   ├── shell.c/h        Command dispatcher, shell loop
│   ├── users.c/h        User authentication & management
│   ├── security.c/h     SHA-256, password analysis
│   ├── filesystem.c/h   Virtual file system (tree)
│   ├── process.c/h      Process management (linked list)
│   ├── scheduler.c/h    CPU scheduling algorithms
│   ├── memory.c/h       Memory management (First/Best/Worst Fit)
│   ├── network.c/h      Browser, email, LAN chat
│   ├── database.c/h     Mini SQL engine
│   ├── blockchain.c/h   Blockchain simulator
│   ├── backup.c/h       Backup & recovery
│   ├── apps.c/h         All built-in apps + package manager
│   └── logs.c/h         System logging
├── data/                Auto-created runtime data folder
│   ├── users.dat
│   ├── system.log
│   ├── blockchain.dat
│   ├── packages.dat
│   ├── mail/
│   ├── backups/
│   └── databases/
├── .vscode/
│   ├── tasks.json       VS Code build tasks
│   ├── launch.json      Debugger config
│   └── c_cpp_properties.json
├── Makefile
├── build.bat            One-click Windows build
├── run.bat              Build + run
└── README.md
```

---

## 💡 First Login

Default admin account created on first run:
- **Username:** `admin`
- **Password:** `Admin@123`

---

## 📖 Command Reference

Type `help` inside the shell for a full command list.

### Quick examples:
```
signup                   # Create new account
login admin              # Login as admin
whoami                   # Show current user info
mkdir /home/admin/docs   # Create directory
touch notes.txt          # Create file
edit notes.txt           # Open text editor
ls -l                    # List with details
cat notes.txt            # Print file contents
tree                     # Directory tree view
ps                       # List processes
schedule rr 3            # Round Robin scheduling (quantum=3)
memory                   # Memory map
alloc 64 myapp           # Allocate 64KB
db                       # Open SQL console
mine "Hello World"       # Mine a blockchain block
blockchain               # View the chain
calc                     # Calculator
snake                    # Snake game
sudoku                   # Sudoku
minesweeper              # Minesweeper
backup                   # Create system snapshot
logs 20                  # Show last 20 log lines
```

---

## ⚙️ Troubleshooting

| Problem | Solution |
|---------|----------|
| `gcc: not found` | Install MinGW-w64 and add `C:\mingw64\bin` to PATH |
| Colors not showing | Use Windows Terminal or VS Code integrated terminal |
| Build error `_CRT_SECURE_NO_WARNINGS` | Already handled in code; use GCC not MSVC |
| `data` folder missing | Run once — it auto-creates, or run `build.bat` |

---

## 📝 License

Educational project. Free to use and modify.
