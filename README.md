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

### How to Compile

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

### How to Clean Up

To completely clean the project, simply delete the `build` folder:

```powershell
# Make sure you are in the project root folder
cd D:\swaralayaplayer

# Delete the build folder and all its contents
Remove-Item -Recurse -Force build
```
