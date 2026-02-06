# Building Windows Executable for BeamCommander

## Overview

This guide explains how to build a Windows executable for BeamCommander **once** on a Windows machine, which can then be committed to the repository for creating releases **without needing Windows** in CI/CD.

## Why This Approach?

- **No Windows required for releases**: Once the executable is built and committed, GitHub Actions can package it from Linux
- **Simple CI/CD**: No complex Windows build setup in CI/CD pipelines  
- **Pre-tested binary**: The executable can be tested before committing
- **Faster releases**: No compilation time in CI/CD

## Prerequisites (One-Time Setup on Windows)

1. **Windows 10/11** (x64)
2. **Visual Studio 2019 or later** with C++ desktop development workload
3. **OpenFrameworks** for Windows (download from https://openframeworks.cc/)
4. **Git** for Windows

## Option 1: Build with Visual Studio (Recommended)

### Step 1: Generate Visual Studio Project Files

1. Download and extract openFrameworks for Windows
2. Run the **projectGenerator** tool:
   ```
   openframeworks/projectGenerator/projectGenerator.exe
   ```

3. In projectGenerator:
   - Click "Import" and select the BeamCommander project folder
   - Or create a new project named "BeamCommander"
   - Add these addons:
     - ofxLaser
     - ofxMidi
     - ofxOsc
     - ofxOpenCv
     - ofxNetwork
     - ofxPoco
   - Click "Generate"

### Step 2: Build the Project

1. Open the generated `.sln` file in Visual Studio
2. Select **Release** configuration and **x64** platform
3. Build > Build Solution (or press F7)
4. The executable will be in: `bin/BeamCommander.exe`

### Step 3: Collect Required Files

The build output should include:
- `BeamCommander.exe` - Main executable
- `*.dll` files - Required libraries (if any in the bin folder)

## Option 2: Build with MSYS2/MinGW (Alternative)

### Step 1: Install MSYS2

1. Download and install MSYS2 from https://www.msys2.org/
2. Open MSYS2 MinGW 64-bit terminal
3. Install dependencies:
   ```bash
   pacman -Syu
   pacman -S make mingw-w64-x86_64-gcc mingw-w64-x86_64-tools
   ```

### Step 2: Install OpenFrameworks Dependencies

```bash
cd /path/to/openframeworks
./scripts/msys2/install_dependencies.sh --noconfirm
./scripts/msys2/download_libs.sh
```

### Step 3: Build BeamCommander

```bash
cd apps/myApps/BeamCommander
make Release -j4
```

The executable will be in: `bin/BeamCommander.exe`

## Committing the Windows Executable

### Step 1: Copy to Repository

1. Clone the BeamCommander repository (if not already done)
2. Copy the built executable to:
   ```
   openframeworks-src-master/apps/myApps/BeamCommander/bin/BeamCommander.exe
   ```
3. Copy any required DLL files to the same `bin/` folder

### Step 2: Test the Executable

Before committing, test the executable:
- Double-click `BeamCommander.exe` to run it
- Verify it starts correctly
- Test basic functionality
- Check for any missing DLL errors

### Step 3: Commit to Repository

```bash
git add openframeworks-src-master/apps/myApps/BeamCommander/bin/BeamCommander.exe
git add openframeworks-src-master/apps/myApps/BeamCommander/bin/*.dll  # if any
git commit -m "Add pre-built Windows executable"
git push
```

**Important**: The executable should be committed with Git LFS if it's large (>50MB), or ensure your repository allows binaries.

## Creating a Windows Release

Once the executable is committed, creating a Windows release is simple:

```bash
# From any machine (no Windows needed!)
git tag v1.0.0-windows
git push origin v1.0.0-windows
```

The GitHub Actions workflow will:
1. Check out the repository
2. Find the pre-built `BeamCommander.exe`
3. Package it with documentation
4. Create a release with `BeamCommander-windows.zip`

## Updating the Windows Executable

When code changes require a new Windows build:

1. Pull the latest code on a Windows machine
2. Rebuild using Visual Studio or MSYS2 (steps above)
3. Replace the `BeamCommander.exe` in the repository
4. Commit and push
5. Create a new release tag

## Troubleshooting

### Missing DLLs

If the executable fails to run due to missing DLLs:
- Use **Dependency Walker** (depends.exe) to identify missing DLLs
- Copy required DLLs to the `bin/` folder
- Commit them along with the executable

Common DLLs that may be needed:
- `libgcc_s_seh-1.dll`
- `libstdc++-6.dll`
- `libwinpthread-1.dll`
- OpenGL-related DLLs

### Executable Too Large for Git

If the executable is very large (>100MB):
1. Consider using Git LFS (Large File Storage)
2. Or use Release assets instead of committing to repo

### Build Errors

**Visual Studio errors:**
- Ensure all addons are properly configured
- Check that OpenFrameworks libraries are downloaded
- Verify Windows SDK is installed

**MSYS2 errors:**
- Run `pacman -Syu` to update packages
- Ensure MinGW toolchain is installed
- Check that OpenFrameworks dependencies are installed

## Alternative: Docker Cross-Compilation (Advanced)

For advanced users who want to avoid Windows entirely, cross-compilation with Docker is possible but complex. This requires:
- Setting up MinGW cross-compiler in Docker
- Cross-compiling all OpenFrameworks dependencies
- Managing Windows-specific libraries

This approach is not covered here due to its complexity.

## Summary

**Simple workflow:**
1. Build on Windows once → 
2. Commit `BeamCommander.exe` → 
3. Create releases from any machine (Linux/macOS/Windows)

**Benefits:**
- ✅ No Windows needed for CI/CD
- ✅ Faster release process
- ✅ Pre-tested executables
- ✅ Simple automation

**When to rebuild:**
- Code changes affecting Windows
- Dependency updates
- Bug fixes
- New features
