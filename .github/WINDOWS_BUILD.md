# Windows Build Setup Guide

This guide explains how to set up Windows releases for BeamCommander **without needing Windows** for CI/CD.

## Current Status ✅

**Windows releases are now possible without Windows in CI/CD!** 

The workflow uses a **pre-built Windows executable** approach:
- Build the Windows executable **once** on a Windows machine
- Commit it to the repository  
- GitHub Actions packages it from Linux (ubuntu-latest)
- No Windows runners or Visual Studio needed in CI/CD

## How It Works

### One-Time Windows Build (by someone with Windows access)
1. Build `BeamCommander.exe` on Windows (using Visual Studio or MinGW)
2. Test the executable
3. Commit `BeamCommander.exe` to `openframeworks-src-master/apps/myApps/BeamCommander/bin/`
4. Push to repository

### Release Process (from any machine)
```bash
git tag v1.0-windows
git push origin v1.0-windows
```

GitHub Actions (running on **Linux**) will:
1. Find the pre-built `BeamCommander.exe`
2. Package it with documentation
3. Create release with `BeamCommander-windows.zip`

**No Windows required!**

## How to Build Windows Executable

See **[BUILD_WINDOWS_EXECUTABLE.md](BUILD_WINDOWS_EXECUTABLE.md)** for detailed instructions on:
- Building with Visual Studio
- Building with MSYS2/MinGW
- Testing the executable
- Committing to the repository

## Quick Start

### If You Have Windows Access:
1. Follow [BUILD_WINDOWS_EXECUTABLE.md](BUILD_WINDOWS_EXECUTABLE.md)
2. Build and commit `BeamCommander.exe`
3. Done! Releases can now be created from any machine

### If You Don't Have Windows Access:
Ask someone with Windows to:
1. Build `BeamCommander.exe` following the guide
2. Test it works
3. Commit and push to the repository

Then **you** can create releases from Linux/macOS!

## Creating a Windows Release

Once the Windows executable is in the repository:

```bash
git tag v1.0-windows
git push origin v1.0-windows
```

The GitHub Actions workflow (running on **ubuntu-latest**) will:
1. Check for `BeamCommander.exe` in `bin/` folder
2. Package it with documentation
3. Create a GitHub release with `BeamCommander-windows.zip`

## Benefits of This Approach

✅ **No Windows needed for CI/CD** - Workflow runs on Linux
✅ **Fast releases** - No compilation, just packaging (~30 seconds)
✅ **Pre-tested** - Executable is tested before committing
✅ **Simple** - No complex Windows build setup in GitHub Actions
✅ **Reliable** - Same binary every time

## When to Update the Windows Executable

Rebuild and commit a new `BeamCommander.exe` when:
- Source code changes
- Dependencies are updated
- Bug fixes for Windows
- New features added

## Workflow Details

The GitHub Actions workflow:
- **Runner**: ubuntu-latest (Linux)
- **Checks**: Verifies `BeamCommander.exe` exists
- **Packages**: Creates zip with exe + docs
- **Uploads**: Creates GitHub release

No Windows runners, Visual Studio, or MSBuild needed!
