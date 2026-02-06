# Windows Build Setup Guide

This guide explains how to set up Windows builds for BeamCommander in the GitHub Actions workflow.

## Current Status ✅

**Windows build is now configured!** Visual Studio project files have been added to the repository and the GitHub Actions workflow is ready to build Windows releases.

## What's Included

The repository now contains:
- `BeamCommander.sln` - Visual Studio 2019 solution file
- `BeamCommander.vcxproj` - Visual Studio project file with all addons configured
- `BeamCommander.vcxproj.filters` - Project filters
- `icon.rc` - Icon resource file
- `icon.ico` and `icon_debug.ico` - Application icons

All required addons are configured:
- ofxLaser
- ofxMidi
- ofxOsc
- ofxOpenCv
- ofxNetwork
- ofxPoco

## How to Trigger a Windows Build

To create a Windows release, simply create and push a tag with the pattern `v*-windows`:

```bash
# Create a test tag
git tag v1.0-test-windows

# Push the tag to GitHub
git push origin v1.0-test-windows
```

The GitHub Actions workflow will:
1. Download the required openFrameworks libraries for Windows
2. Build BeamCommander using MSBuild
3. Package the executable with documentation
4. Create a GitHub release with the Windows build artifact

## Alternative Build Options

### Option 1: Visual Studio Project Files (✅ Already Implemented)

This is the **recommended** and **currently implemented** approach. The Visual Studio project files have been added to the repository.

The workflow will:
1. Download openFrameworks libraries using `download_libs.ps1`
2. Build from source using MSBuild
3. Package the executable

**No additional setup required** - just push a tag to trigger the build!

### Option 2: Use Pre-built Windows Executable (Alternative)

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
