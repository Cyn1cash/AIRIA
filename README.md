# AIRIA

## Development Setup

### 1. Download Prerequisites

#### Windows

On Windows, use **[Scoop](https://scoop.sh/)** to download Git.

```pwsh
# Install Scoop
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression
# Install Git
scoop install git
```

#### MacOS

Git ships with MacOS by default, so no installation is required. However, a **[Homebrew](https://brew.sh/)** install would make your life a lot easier in the following steps.

```sh
# Install Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

#### Linux

Installation instructions differ per distro (but Git ships with most Linux distros). Also, if you use Linux, you probably already know how to do this.

### 2. Download Visual Studio Code

Download **[Visual Studio Code](https://code.visualstudio.com)** and run the installer.

```sh
# Windows
winget install -e --id Microsoft.VisualStudioCode
# MacOS
brew install --cask visual-studio-code
```

### 3. Download PlatformIO

Download **[PlatformIO IDE](https://platformio.org/platformio-ide)** from the Extensions tab in Visual Studio Code.

```sh
# Command Palette (Ctrl + Shift + P, then backspace)
ext install platformio.platformio-ide
```

### 4. Clone this repository and open it locally

You can close Visual Studio Code after your extensions finish installing. These commands open Visual Studio Code for you in the right directory.

```sh
git clone https://github.com/thaddeuskkr/AIRIA.git
cd AIRIA
code .
```
