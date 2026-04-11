● Building Veena Plugin on macOS

  Prerequisites                                                                                                                                                                                                                              
  1. Xcode (and Command Line Tools)                                                                                                                                                                                                          
  Xcode is Apple's IDE and provides the C++ compiler (Clang), linker, and macOS/iOS SDKs.

  # Install Xcode from the Mac App Store, then:
  xcode-select --install

  Verify:
  clang++ --version

  2. CMake (3.22 or later)

  Install via Homebrew (recommended):
  brew install cmake

  Or download the macOS installer from https://cmake.org/download/

  Verify:
  cmake --version

  3. Git

  Usually comes with Xcode Command Line Tools. Verify:
  git --version

  If missing: brew install git

  4. JUCE Framework

  Clone JUCE next to the project directory:
  cd /path/to/your/dev/folder
  git clone https://github.com/juce-framework/JUCE.git

  The project expects JUCE to be at ../JUCE relative to the project root. So the folder structure should be:
  /your/dev/folder/
    ├── JUCE/          ← cloned here
    └── Shabda/        ← the project

  Build Steps

  1. Clone the project
  cd /path/to/your/dev/folder
  git clone https://github.com/kirantoday/Shabda.git
  cd Shabda

  2. Verify JUCE is in the right location
  ls ../JUCE/CMakeLists.txt
  # Should print the path. If JUCE is elsewhere, pass -DJUCE_PATH=/your/juce/path in step 3.

  3. Configure with CMake
  cmake -B build -G "Xcode"

  Or to use Unix Makefiles (faster for command-line builds, no Xcode project):
  cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release

  If JUCE is not at ../JUCE:
  cmake -B build -G "Xcode" -DJUCE_PATH=/absolute/path/to/JUCE

  The first configure takes several minutes (building juceaide). Subsequent configures are fast.

  4. Build

  With Xcode generator:
  cmake --build build --config Release --target VeenaPlugin_Standalone
  cmake --build build --config Release --target VeenaPlugin_VST3

  With Unix Makefiles:
  cmake --build build --target VeenaPlugin_Standalone
  cmake --build build --target VeenaPlugin_VST3

  5. Run

  Standalone app:
  open build/VeenaPlugin_artefacts/Release/Standalone/Veena.app

  VST3 plugin (copy to system plugin folder for DAW scanning):
  cp -r build/VeenaPlugin_artefacts/Release/VST3/Veena.vst3 ~/Library/Audio/Plug-Ins/VST3/

  Troubleshooting

  ┌─────────────────────────────┬────────────────────────────────────────────────────────────────────────┐
  │            Issue            │                                  Fix                                   │
  ├─────────────────────────────┼────────────────────────────────────────────────────────────────────────┤
  │ JUCE not found              │ Pass -DJUCE_PATH=/path/to/JUCE to cmake                                │
  ├─────────────────────────────┼────────────────────────────────────────────────────────────────────────┤
  │ No Xcode installation found │ Run xcode-select --install or install Xcode from App Store             │
  ├─────────────────────────────┼────────────────────────────────────────────────────────────────────────┤
  │ juceaide build fails        │ Ensure Xcode license is accepted: sudo xcodebuild -license accept      │
  ├─────────────────────────────┼────────────────────────────────────────────────────────────────────────┤
  │ cmake not found             │ brew install cmake or add CMake.app's bin to PATH                      │
  ├─────────────────────────────┼────────────────────────────────────────────────────────────────────────┤
  │ codesigning errors          │ Add -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO to cmake configure │
  └─────────────────────────────┴────────────────────────────────────────────────────────────────────────┘

  Quick Copy-Paste (full sequence)

  # Install prerequisites
  xcode-select --install
  brew install cmake

  # Clone JUCE and Shabda side by side
  mkdir -p ~/Dev && cd ~/Dev
  git clone https://github.com/juce-framework/JUCE.git
  git clone https://github.com/kirantoday/Shabda.git

  # Build
  cd Shabda
  cmake -B build -G "Xcode"
  cmake --build build --config Release --target VeenaPlugin_Standalone
  cmake --build build --config Release --target VeenaPlugin_VST3

  # Run standalone
  open build/VeenaPlugin_artefacts/Release/Standalone/Veena.app

───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── plucked-string-waveguide ──> 
─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
  ⏵⏵ bypass permissions on (shift+tab to cycle)



  --------------------------------------------------------------------------------------

   Building Veena Plugin on Windows (Fresh Machine)

  Prerequisites                                                                                                                                                                                 
  1. Visual Studio 2022 or 2026 (Community Edition is free)                                                                                                                                     
  Download from: https://visualstudio.microsoft.com/downloads/

  During installation, check the "Desktop development with C++" workload. This includes:
  - MSVC compiler
  - Windows SDK
  - CMake integration (optional, you can use standalone CMake instead)

  2. CMake (3.22 or later)

  Download the Windows x64 Installer (.msi) from: https://cmake.org/download/

  During install, check "Add CMake to the system PATH for all users" (or current user).

  Verify in a new terminal:
  cmake --version

  3. Git for Windows

  Download from: https://git-scm.com/download/win

  Verify:
  git --version

  ---
  Setup Steps

  1. Choose a folder for your dev work (e.g., C:\DEV)

  mkdir C:\DEV
  cd C:\DEV

  2. Clone JUCE (the project expects it at ..\JUCE relative to Shabda)

  git clone https://github.com/juce-framework/JUCE.git

  This will create C:\DEV\JUCE.

  3. Clone Shabda

  git clone https://github.com/kirantoday/Shabda.git

  Final folder structure:
  C:\DEV\
    ├── JUCE\        ← cloned
    └── Shabda\      ← cloned

  ---
  Build Steps

  1. Open a terminal in C:\DEV\Shabda

  Use Command Prompt, PowerShell, or Git Bash — all work.

  2. Configure CMake

  For Visual Studio 2022:
  cmake -B build -G "Visual Studio 17 2022" -A x64

  For Visual Studio 2026:
  cmake -B build -G "Visual Studio 18 2026" -A x64

  If JUCE is at a different location:
  cmake -B build -G "Visual Studio 17 2022" -A x64 -DJUCE_PATH=C:/path/to/JUCE

  First-time configure takes 3-5 minutes because CMake builds JUCE's juceaide helper tool. Subsequent configures are ~10 seconds.

  3. Build

  Standalone (recommended for testing):
  cmake --build build --config Debug --target VeenaPlugin_Standalone

  VST3 plugin:
  cmake --build build --config Debug --target VeenaPlugin_VST3

  Or build Release (faster runtime, larger build time):
  cmake --build build --config Release --target VeenaPlugin_Standalone

  ---
  Run

  Standalone app:
  build\VeenaPlugin_artefacts\Debug\Standalone\Veena.exe

  (Replace Debug with Release if you built Release.)

  VST3 plugin — copy to your DAW's plugin folder:
  xcopy /E /I /Y build\VeenaPlugin_artefacts\Debug\VST3\Veena.vst3 "%CommonProgramFiles%\VST3\Veena.vst3"

  Then rescan plugins in your DAW.

  ---
  Quick Copy-Paste Sequence

  mkdir C:\DEV
  cd C:\DEV
  git clone https://github.com/juce-framework/JUCE.git
  git clone https://github.com/kirantoday/Shabda.git
  cd Shabda
  cmake -B build -G "Visual Studio 17 2022" -A x64
  cmake --build build --config Debug --target VeenaPlugin_Standalone
  build\VeenaPlugin_artefacts\Debug\Standalone\Veena.exe

  ---
  Troubleshooting

  ┌──────────────────────────────────────────────┬───────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
  │                   Problem                    │                                                        Fix                                                        │
  ├──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
  │ cmake: command not found                     │ CMake not on PATH. Reinstall with "Add to PATH" checked, or use full path: "C:\Program Files\CMake\bin\cmake.exe" │
  ├──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
  │ could not find any instance of Visual Studio │ Wrong generator. List available: cmake --help. Match the version you installed (17 2022 vs 18 2026)               │
  ├──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
  │ JUCE not found                               │ JUCE isn't at ..\JUCE. Pass -DJUCE_PATH=C:/full/path/to/JUCE to cmake                                             │
  ├──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
  │ First build is very slow                     │ Normal — juceaide compilation takes ~5 min on first configure. Only happens once                                  │
  ├──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
  │ LNK1168: cannot open Veena.exe for writing   │ The standalone app is still running. Close it first                                                               │
  ├──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
  │ No audio device shows up                     │ Click the gear/settings icon in the standalone app to select your audio output                                    │
  └──────────────────────────────────────────────┴───────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘

  ---
  Playing the Plugin

  Once Veena.exe is running:
  - Click the on-screen piano keyboard with your mouse, OR
  - Use A-S-D-F-G-H-J-K-L for white keys (starts at C3)
  - W-E-T-Y-U for black keys
  - Adjust the sliders for tone, expression, and resonance
  - Try Legato + glide curves for meend phrases

──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── plucked-string-waveguide ──
> 
────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
  ⏵⏵ bypass permissions on (shift+tab to cycle)

                                                                                                                                                                                                                                                                                                
