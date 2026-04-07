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
                                                                                                                                                                                                                                                                                                
