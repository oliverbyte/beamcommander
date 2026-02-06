# Windows Test Release - Implementation Complete

## Overview

This PR adds complete Windows build support for BeamCommander, enabling automated Windows releases through GitHub Actions.

## What Was Added

### 1. Visual Studio Project Files
Created complete Visual Studio 2019 project configuration:
- `BeamCommander.sln` - Solution file
- `BeamCommander.vcxproj` - Project file with all addons configured
- `BeamCommander.vcxproj.filters` - Project filters
- `icon.rc` - Icon resource file
- `icon.ico` and `icon_debug.ico` - Application icons

### 2. Addon Configuration
All required addons are properly configured in the project:
- ofxLaser (laser control framework)
- ofxMidi (MIDI controller support)
- ofxOsc (OSC message handling)
- ofxOpenCv (computer vision)
- ofxNetwork (network communication)
- ofxPoco (Poco library integration)

### 3. GitHub Actions Workflow Update
Enhanced `.github/workflows/build-and-release.yml`:
- Added Windows library download step using `download_libs.ps1`
- Configured MSBuild with proper flags for Release builds
- Set up packaging to create `BeamCommander-windows.zip`

### 4. Documentation
Created and updated documentation:
- `WINDOWS_TEST_BUILD.md` - Quick start guide for triggering Windows builds
- Updated `.github/WINDOWS_BUILD.md` - Comprehensive Windows build guide

## How to Test

### Step 1: Merge this PR
Merge this branch into your main branch (or the branch you want to release from).

### Step 2: Create a Test Tag
After merging, create and push a tag to trigger the Windows build:

```bash
# From the main branch (or your release branch)
git tag v1.0-test-windows
git push origin v1.0-test-windows
```

### Step 3: Monitor the Build
1. Go to the "Actions" tab in the GitHub repository
2. Find the workflow run for your tag `v1.0-test-windows`
3. Monitor the build progress:
   - Checkout repository
   - Download Windows libraries (~5-10 minutes)
   - Build with MSBuild (~5-10 minutes)
   - Package release
   - Create GitHub release

### Step 4: Download and Test
Once the workflow completes:
1. Go to "Releases" in your repository
2. Find "BeamCommander v1.0-test-windows"
3. Download `BeamCommander-windows.zip`
4. Extract and test on a Windows machine

## Tag Patterns

The workflow supports different tag patterns:
- `v*-windows` - Build for Windows only
- `v*-macos` - Build for macOS only
- `v*-all` - Build for both platforms

Examples:
```bash
git tag v1.0-windows          # Windows only
git tag v1.0.1-all           # Both platforms
git tag v2025.02.06-windows  # Date-based version
```

## Build Artifacts

The Windows release includes:
- `BeamCommander.exe` - Windows executable
- Required DLL files
- `README.md` - User documentation
- `LICENSE.md` - License information
- `DEVELOPER.md` - Developer documentation
- `doc/` - Documentation and demo files

## Technical Details

### Project Configuration
- **Target Platform:** Windows x64
- **Platform Toolset:** v142 (Visual Studio 2019)
- **C++ Standard:** C++17
- **Configuration:** Release (optimized)
- **Subsystem:** Console
- **Character Set:** Unicode

### Build Process
1. GitHub Actions runner: `windows-latest`
2. MSBuild setup via `microsoft/setup-msbuild@v2`
3. Download openFrameworks libraries: `download_libs.ps1`
4. Build with MSBuild: `/p:Configuration=Release /p:Platform=x64 /m`
5. Package with PowerShell scripts

## Troubleshooting

If the build fails, check:
1. **Actions logs** - Look for error messages in the GitHub Actions output
2. **Library download** - Ensure the download_libs.ps1 step completed
3. **MSBuild errors** - Check for compilation errors in the build step
4. **Missing dependencies** - Verify all addons are available

Common issues:
- **Library download timeout** - Re-run the workflow
- **Compilation errors** - Check the Actions logs for specific C++ errors
- **Missing addons** - Verify addon .props files exist in the addons directories

## Next Steps

After this PR is merged:
1. Test the Windows build by creating a tag
2. Verify the build artifact works on Windows
3. Create a proper release tag (e.g., `v1.0.0-all`) for production release
4. Update README.md to mention Windows support in releases

## References

- `.github/RELEASE.md` - Complete release guide
- `.github/WINDOWS_BUILD.md` - Windows build setup guide
- `WINDOWS_TEST_BUILD.md` - Quick start for Windows builds
- GitHub Actions Workflow: `.github/workflows/build-and-release.yml`

## Summary

This PR completes the Windows build infrastructure for BeamCommander. Simply create and push a tag with the pattern `v*-windows` to trigger an automated Windows release build.
