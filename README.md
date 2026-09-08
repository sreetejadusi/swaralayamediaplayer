# Swaralaya Media Player

Swaralaya Media Player is a lightweight, high-performance desktop video player specifically built for a pristine karaoke practice experience. Powered by **Qt 6** and **libmpv**, this player guarantees instant startup times, drag-and-drop playback, and real-time, hardware-accelerated audio effects.

## Core Features

- **Instant Playback**: Forget scanning media libraries. Simply drag and drop your karaoke file or set the player as the default application for instant, single-file playback.
- **Real-Time Loudness Normalization**: Eliminates the hassle of manually adjusting volume for inconsistent karaoke tracks. Uses FFmpeg's `dynaudnorm` filter to automatically normalize all videos to your desired loudness target.
- **Pitch and Tempo Shifting**: Easily change the pitch (key) or tempo (speed) of your backing tracks in real-time.
- **Minimalist Dark UI**: Focused entirely on the practice session. The bottom control bar automatically hides when in fullscreen mode for an immersive experience.
- **Persistent Settings**: Your audio adjustments (Pitch, Tempo, Loudness Target) are automatically saved to `settings.json` and restored on your next session.
- **Cross-Platform Native**: Fully hardware-accelerated on both Windows (Direct3D 11) and Linux (Native Wayland/X11 via OpenGL).

## Keyboard Shortcuts

The player is optimized for quick adjustments while practicing:
- <kbd>Spacebar</kbd>: Play / Pause
- <kbd>F</kbd>: Toggle Fullscreen
- <kbd>O</kbd> / <kbd>P</kbd>: Decrease / Increase Pitch
- <kbd>R</kbd> / <kbd>T</kbd>: Decrease / Increase Tempo
- <kbd>-</kbd> / <kbd>+</kbd> (or <kbd>=</kbd>): Decrease / Increase Volume
- <kbd>K</kbd> / <kbd>L</kbd>: Decrease / Increase Loudness Normalization Target Level
- <kbd>D</kbd>: Toggle Loudness Normalization ON/OFF
- <kbd>M</kbd>: Mute
- <kbd>Left</kbd> / <kbd>Right</kbd>: Seek backwards/forwards by 10 seconds
- <kbd>Shift</kbd> + <kbd>Left</kbd> / <kbd>Right</kbd>: Seek backwards/forwards by 30 seconds

## License
This software is strictly for **Free, Personal, and Non-Commercial Use**. Please review the `License.txt` file for complete details.

---

## Compiling from Source

Swaralaya Media Player can be built on both Windows and Linux using CMake and Qt 6.

### Windows Prerequisites

1. **C++ Compiler**: MinGW-w64 (preferred and tested) or MSVC 2019/2022.
2. **CMake**: Version 3.16 or higher.
3. **Qt 6**: Install via the Qt Maintenance Tool. Ensure you install the Desktop component (e.g., MinGW 64-bit or MSVC 64-bit).
4. **libmpv**: You need the `libmpv` developer files (the DLL, the headers `mpv/client.h`, and the `.a` or `.lib` file).

### Linux Prerequisites (Debian/Ubuntu)

Ensure you have the build essentials, Qt 6 (including OpenGL and Wayland support), and libmpv development libraries installed:

```bash
sudo apt update
sudo apt install build-essential cmake pkg-config qt6-base-dev qt6-wayland libqt6opengl6-dev libqt6core5compat6-dev libmpv-dev
```

### How to Compile on Windows

Open a PowerShell window in your project root, and run these steps in order:

```powershell
# 1. Create and enter the build directory
mkdir build
cd build

# 2. Add Qt's CMake and Compiler tools to your temporary system PATH
$env:PATH += ";C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin"

# 3. Configure the CMake project
# (It connects the project to your Qt installation)
# NOTE: We include -G "MinGW Makefiles" to ensure CMake uses the proper compiler instead of Visual Studio.
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/mingw_64"

# 4. Compile the source code into an executable
cmake --build . --config Release

# 5. Deploy Qt Dependencies 
# (This copies all required Qt DLLs alongside your .exe so it can run)
C:\Qt\6.11.1\mingw_64\bin\windeployqt.exe --compiler-runtime .\SwaralayaMediaPlayer.exe

# 6. Build the Windows Installer (Optional)
# (This packages everything into SwaralayaMediaPlayer_Setup.exe)
& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" ..\setup.iss
```

### How to Compile on Linux (and create `.deb`)

Open a terminal in your project root and run:

```bash
# 1. Create and enter the build directory
mkdir build
cd build

# 2. Configure the CMake project
cmake ..

# 3. Compile the source code into an executable
make -j$(nproc)

# 4. Generate the Debian package (.deb)
cpack -G DEB
```
When installed, the `.deb` package will automatically configure Swaralaya Media Player as your system's default handler for media files.

### How to Clean Up

To completely clean the project, simply delete the `build` folder:

**Windows:**
```powershell
Remove-Item -Recurse -Force build
```

**Linux:**
```bash
rm -rf build
```
