# Windows Build Setup Guide

This guide explains how to set up Windows builds for BeamCommander in the GitHub Actions workflow.

## Current Status

The Windows build currently fails because there are no Visual Studio project files (`.sln`/`.vcxproj`) in the repository, and building with `make` on Windows requires complex MinGW/MSYS2 setup that isn't configured.

## Solutions

You have two options to enable Windows builds:

### Option 1: Add Visual Studio Project Files (Recommended)

This allows the GitHub Actions workflow to build from source on Windows.

#### Steps:

1. **On a Windows machine with openFrameworks installed:**
   
   Open openFrameworks projectGenerator:
   ```
   openframeworks-src-master/projectGenerator.exe
   ```

2. **Configure the project:**
   - Import the existing BeamCommander project
   - Set project path to: `apps/myApps/BeamCommander`
   - Add all the addons used in `addons.make`:
     - ofxLaser
     - ofxMidi
     - ofxOsc
   - Click "Generate"

3. **Commit the generated files:**
   ```bash
   cd openframeworks-src-master/apps/myApps/BeamCommander
   git add *.sln *.vcxproj *.vcxproj.filters
   git commit -m "Add Visual Studio project files for Windows build"
   git push
   ```

4. **Test the workflow:**
   ```bash
   git tag v1.1-alpha-windows
   git push origin v1.1-alpha-windows
   ```

### Option 2: Use Pre-built Windows Executable

This allows you to include a pre-built Windows executable in the repository for releases.

#### Steps:

1. **Build BeamCommander on Windows:**
   
   Using Visual Studio or your preferred method, build the BeamCommander executable locally.

2. **Locate the executable:**
   ```
   openframeworks-src-master/apps/myApps/BeamCommander/bin/BeamCommander.exe
   ```

3. **Commit the pre-built executable:**
   ```bash
   cd openframeworks-src-master/apps/myApps/BeamCommander/bin
   git add BeamCommander.exe
   git add *.dll  # Include any required DLL files
   git commit -m "Add pre-built Windows executable"
   git push
   ```

4. **Test the workflow:**
   ```bash
   git tag v1.1-alpha-windows
   git push origin v1.1-alpha-windows
   ```

## How the Workflow Works

The updated workflow now:

1. **Checks for build files** in this order:
   - Visual Studio solution files (`.sln`)
   - Visual Studio project files (`.vcxproj`)
   - Pre-built executable (`bin/BeamCommander.exe`)

2. **Takes appropriate action:**
   - If VS project files exist → Builds from source using MSBuild
   - If pre-built exe exists → Uses the pre-built executable
   - If neither exists → Fails with helpful error message

3. **Packages the result** into `BeamCommander-windows.zip`

## Recommendation

**Option 1 (Visual Studio project files)** is recommended because:
- Builds from source ensure consistency
- Easier to maintain and update
- Smaller repository size (no large .exe files)
- Better for version control

**Option 2 (Pre-built executable)** is simpler but:
- Requires manual rebuilding for each change
- Larger repository size
- Harder to track changes in the binary

## Testing Locally

Before pushing to GitHub, you can test the build locally:

### With Visual Studio:
```powershell
cd openframeworks-src-master/apps/myApps/BeamCommander
msbuild BeamCommander.sln /p:Configuration=Release /p:Platform=x64
```

### Check the output:
```powershell
cd bin
dir BeamCommander.exe
```

## Need Help?

If you encounter issues:
1. Verify openFrameworks is properly installed on Windows
2. Ensure all addons (ofxLaser, ofxMidi, ofxOsc) are in the addons folder
3. Check that the Visual Studio project targets the correct Windows SDK version
4. Make sure all required DLLs are in the bin folder

## Current Error Explained

The error you're seeing:
```
couldn't find cairo zlib openssl freetype2 glew glfw3 glm libcurl liburiparser nlohmann_json openal pugixml rtaudio
```

This occurs because the workflow tried to use `make` (Unix-style build), which requires:
- MinGW/MSYS2 environment
- pkg-config
- All openFrameworks dependencies compiled for MinGW

This is complex to set up in GitHub Actions, so using Visual Studio (Option 1) or pre-built binaries (Option 2) is much simpler.
