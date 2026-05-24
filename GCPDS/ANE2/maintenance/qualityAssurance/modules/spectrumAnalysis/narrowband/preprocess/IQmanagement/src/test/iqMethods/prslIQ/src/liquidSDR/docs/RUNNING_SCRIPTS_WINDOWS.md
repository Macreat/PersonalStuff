# How to Run Scripts on Windows

## 1. Running .sh Files (Bash)
On Windows, `.sh` files are not natively executable by the Command Prompt or PowerShell. You should use one of the following:

- **Git Bash**: Included with [Git for Windows](https://git-scm.com/download/win). This is the recommended way as it provides a Linux-like terminal.
  - *Usage*: Right-click in the folder -> `Git Bash Here` -> `./scripts/build_and_run.sh`
- **WSL (Windows Subsystem for Linux)**: If you have Ubuntu or another distro installed on Windows.
- **Cygwin / MSYS2**: Professional Unix-like environments for Windows.

---

## 2. Running .ps1 Files (PowerShell)
I have provided equivalent `.ps1` scripts for native Windows execution.

### Execution Policy
If PowerShell blocks the script, you may need to run this once as Administrator:
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Usage
```powershell
./scripts/build_and_run.ps1
./scripts/deployRaspi.ps1
```
