# Swaralaya Media Player

Swaralaya Media Player is a lightweight, high-performance desktop video player specifically built for a pristine karaoke practice experience. Powered by **Qt 6** and **libmpv**, this player guarantees instant startup times, drag-and-drop playback, and real-time, hardware-accelerated audio effects.

## Core Features

- **Instant Playback**: Forget scanning media libraries. Simply drag and drop your karaoke file or set the player as the default application for instant, single-file playback.
- **Real-Time Loudness Normalization**: Eliminates the hassle of manually adjusting volume for inconsistent karaoke tracks. Uses FFmpeg's `dynaudnorm` filter to automatically normalize all videos to your desired loudness target.
- **Pitch and Tempo Shifting**: Easily change the pitch (key) or tempo (speed) of your backing tracks in real-time.
- **Minimalist Dark UI**: Focused entirely on the practice session. The bottom control bar automatically hides when in fullscreen mode for an immersive experience.
- **Persistent Settings**: Your audio adjustments (Pitch, Tempo, Loudness Target) are automatically saved to `settings.json` and restored on your next session.

## Keyboard Shortcuts

The player is optimized for quick adjustments while practicing:
- <kbd>Spacebar</kbd>: Play / Pause
- <kbd>F</kbd>: Toggle Fullscreen
- <kbd>O</kbd> / <kbd>P</kbd>: Decrease / Increase Pitch
- <kbd>R</kbd> / <kbd>T</kbd>: Decrease / Increase Tempo
- <kbd>-</kbd> / <kbd>+</kbd>: Decrease / Increase Volume
- <kbd>B</kbd> / <kbd>N</kbd>: Decrease / Increase Loudness Normalization Target Level
- <kbd>D</kbd>: Toggle Loudness Normalization ON/OFF
- <kbd>M</kbd>: Mute
- <kbd>Left</kbd> / <kbd>Right</kbd>: Seek backwards/forwards by 5 seconds
- <kbd>Shift</kbd> + <kbd>Left</kbd> / <kbd>Right</kbd>: Seek backwards/forwards by 30 seconds

## License
This software is strictly for **Free, Personal, and Non-Commercial Use**. Please review the `License.txt` file for complete details.

---

## Compiling from Source

Follow these instructions to compile the Swaralaya Media Player yourself using CMake and Qt 6.

### Prerequisites

1. **C++ Compiler**: MinGW-w64 (preferred and tested) or MSVC 2019/2022.
2. **CMake**: Version 3.16 or higher.
3. **Qt 6**: Install via the Qt Maintenance Tool. Ensure you install the Desktop component (e.g., MinGW 64-bit or MSVC 64-bit).
4. **libmpv**: You need the `libmpv` developer files (the DLL, the headers `mpv/client.h`, and the `.a` or `.lib` file).

### Step 1: Setup libmpv
1. Download a pre-compiled Windows build of `libmpv` that includes developer headers.
2. Inside the repository root, ensure the following structure exists:
   ```text
   third_party/
     mpv/
       mpv/
         client.h (and other mpv headers)
       mpv.lib (or libmpv.dll.a for MinGW)
   ```
3. Ensure the actual `libmpv-2.dll` is available (you will need it to run the executable).

### Step 2: Configure with CMake
Open a terminal (e.g., PowerShell or Command Prompt) and set up your Qt and CMake paths.
```powershell
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"
```
*(Note: Replace `6.x.x/mingw_64` with your exact Qt version and compiler toolchain path).*

### Step 3: Build the Project
Run the build command:
```powershell
cmake --build . --config Release
```

### Step 4: Deploy Qt Dependencies
Because Qt relies on many dynamically loaded plugins and DLLs, you must use the `windeployqt` tool to copy them into your build folder alongside the executable.
```powershell
C:\Qt\6.x.x\mingw_64\bin\windeployqt.exe --compiler-runtime .\SwaralayaMediaPlayer.exe
```

### Step 5: Run the Application
Finally, copy your `libmpv-2.dll` into the `build` directory next to `SwaralayaMediaPlayer.exe`. 
You can now run the application!

---
*(Optional) If you have Inno Setup 6 installed, you can compile the `setup.iss` script to generate a distributable Windows Installer.*
