# Building Shabda Veena Plugin on macOS

## Prerequisites

### 1. Xcode

Xcode is Apple's development environment. It provides the C++ compiler (Clang), linker, and macOS SDKs.

**Install from the Mac App Store:**
- Open the App Store on your Mac
- Search for "Xcode"
- Click "Get" / "Install" (it's free, ~12 GB download)
- Wait for installation to complete

**After Xcode is installed, install the Command Line Tools:**

Open Terminal (Applications > Utilities > Terminal) and run:

```bash
xcode-select --install
```

A dialog will pop up — click "Install" and wait for it to finish.

**Accept the Xcode license:**

```bash
sudo xcodebuild -license accept
```

You'll be prompted for your Mac password.

**Verify the compiler is working:**

```bash
clang++ --version
```

You should see output like `Apple clang version 15.x.x` or similar.

---

### 2. Homebrew (macOS Package Manager)

Homebrew makes it easy to install developer tools on macOS.

**Install Homebrew:**

Open Terminal and paste this command:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Follow the on-screen instructions. After installation, you may need to add Homebrew to your PATH. The installer will tell you the exact commands — they look like:

```bash
echo >> ~/.zprofile
echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
eval "$(/opt/homebrew/bin/brew shellenv)"
```

**Verify Homebrew is working:**

```bash
brew --version
```

---

### 3. CMake

CMake is the build system generator used by this project.

**Install via Homebrew:**

```bash
brew install cmake
```

**Verify:**

```bash
cmake --version
```

You should see version 3.22 or later.

---

### 4. Git

Git is usually pre-installed with Xcode Command Line Tools.

**Verify:**

```bash
git --version
```

If it's not installed:

```bash
brew install git
```

---

## Setup

### Step 1: Create a working directory

Open Terminal and run:

```bash
mkdir -p ~/Dev
cd ~/Dev
```

### Step 2: Clone the JUCE framework

JUCE is the audio plugin framework. The project expects it to be in a folder next to the Shabda project.

```bash
git clone https://github.com/juce-framework/JUCE.git
```

This creates `~/Dev/JUCE/`.

### Step 3: Clone the Shabda project

```bash
git clone https://github.com/kirantoday/Shabda.git
```

This creates `~/Dev/Shabda/`.

### Step 4: Verify folder structure

```bash
ls ~/Dev/
```

You should see:

```
JUCE    Shabda
```

Both folders must be side by side. The project's CMakeLists.txt looks for JUCE at `../JUCE` relative to the Shabda folder.

---

## Building

### Step 5: Navigate to the project folder

```bash
cd ~/Dev/Shabda
```

### Step 6: Run CMake to configure the build

**Option A: Using Xcode generator (recommended if you want to open the project in Xcode):**

```bash
cmake -B build -G "Xcode"
```

**Option B: Using Unix Makefiles (faster command-line build, no Xcode project):**

```bash
cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
```

**Note:** The first time you run this, CMake will build JUCE's `juceaide` helper tool. This takes 3-5 minutes. Subsequent runs are fast (~10 seconds).

**If JUCE is not at `~/Dev/JUCE`**, specify the path manually:

```bash
cmake -B build -G "Xcode" -DJUCE_PATH=/your/path/to/JUCE
```

### Step 7: Build the Standalone app

**With Xcode generator:**

```bash
cmake --build build --config Release --target VeenaPlugin_Standalone
```

**With Unix Makefiles:**

```bash
cmake --build build --target VeenaPlugin_Standalone
```

### Step 8: Build the VST3 plugin (optional)

**With Xcode generator:**

```bash
cmake --build build --config Release --target VeenaPlugin_VST3
```

**With Unix Makefiles:**

```bash
cmake --build build --target VeenaPlugin_VST3
```

---

## Running

### Run the Standalone app

```bash
open build/VeenaPlugin_artefacts/Release/Standalone/Veena.app
```

If you built with Unix Makefiles (no config subfolder):

```bash
open build/VeenaPlugin_artefacts/Standalone/Veena.app
```

### Install the VST3 plugin for use in a DAW

Copy the VST3 bundle to your system's plugin folder:

```bash
cp -r build/VeenaPlugin_artefacts/Release/VST3/Veena.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

Then open your DAW (Logic Pro, Ableton, Reaper, etc.) and rescan plugins. "Veena" by "AudioJira" should appear.

---

## Playing the Plugin

Once the Standalone app is running:

- **Select your audio output:** Click the gear/settings icon in the standalone window to choose your speakers or headphones
- **Play with the on-screen piano keyboard:** Click the keys with your mouse
- **Use QWERTY keyboard for notes:**
  - **A S D F G H J K L** = white keys (C D E F G A B C D), starting at C3
  - **W E T Y U** = black keys (C# D# F# G# A#)
- **Thalam (drone) strings:** Press **Z** (Sa), **X** (Pa), **C** (sa) for rhythmic punctuation
- **Adjust controls:** Use the knobs for pluck position, damping, brightness, body mix, sympathetic resonance, and thalam volume
- **Pitch bend:** Drag the vertical strip in the Expression section
- **Vibrato:** Turn the vibrato knob to add kampita (Indian vibrato)
- **Expression:** Drag the vertical strip to control volume
- **Settings:** Use dropdowns for body mode (Modal/Convolution/Hybrid), raga sympathetic tuning, Sa tuning, bend range, and glide curve
- **Legato:** When enabled (default), playing notes quickly glides smoothly between them without replucking

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| `cmake: command not found` | Run `brew install cmake`, then open a new Terminal window |
| `xcode-select: error: command line tools are already installed` | That's fine — the tools are already there |
| `could not find any instance of Visual Studio` | You're using a Windows generator on Mac. Use `-G "Xcode"` or `-G "Unix Makefiles"` |
| `No CMAKE_C_COMPILER could be found` | Run `xcode-select --install` and `sudo xcodebuild -license accept` |
| `JUCE not found` / `add_subdirectory given source not found` | JUCE isn't at `../JUCE`. Pass `-DJUCE_PATH=/full/path/to/JUCE` to cmake |
| First build takes 5+ minutes | Normal — JUCE builds its `juceaide` helper once. Subsequent builds are fast |
| `codesigning errors` | Add `-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO` to the cmake command |
| No audio output | Open the standalone app's audio settings (gear icon) and select your output device |
| Build succeeds but app won't open | Try right-clicking the .app > "Open" to bypass Gatekeeper, or check System Settings > Privacy & Security |
| `Killed: 9` when running | macOS blocked the unsigned app. Go to System Settings > Privacy & Security and click "Allow Anyway" |

---

## Quick Copy-Paste (Full Sequence)

For a brand new Mac with nothing installed:

```bash
# 1. Install Xcode Command Line Tools
xcode-select --install

# 2. Accept Xcode license
sudo xcodebuild -license accept

# 3. Install Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 4. Install CMake
brew install cmake

# 5. Create dev folder
mkdir -p ~/Dev && cd ~/Dev

# 6. Clone JUCE and Shabda side by side
git clone https://github.com/juce-framework/JUCE.git
git clone https://github.com/kirantoday/Shabda.git

# 7. Build
cd Shabda
cmake -B build -G "Xcode"
cmake --build build --config Release --target VeenaPlugin_Standalone

# 8. Run
open build/VeenaPlugin_artefacts/Release/Standalone/Veena.app
```

---

## Building a Debug Version (for development)

If you want to build a debug version for testing:

```bash
cmake --build build --config Debug --target VeenaPlugin_Standalone
open build/VeenaPlugin_artefacts/Debug/Standalone/Veena.app
```

Debug builds are slower but include debug symbols for troubleshooting.

---

## Rebuilding After Code Changes

If you modify source files and want to rebuild:

```bash
cd ~/Dev/Shabda
cmake --build build --config Release --target VeenaPlugin_Standalone
```

CMake only recompiles changed files, so rebuilds are fast.

If you add or remove source files, re-run the configure step first:

```bash
cmake -B build -G "Xcode"
cmake --build build --config Release --target VeenaPlugin_Standalone
```
