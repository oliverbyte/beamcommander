# Release Guide for BeamCommander

This document explains how to create releases for BeamCommander using the automated GitHub Actions workflow.

## Tag-Based Releases

The workflow is triggered by pushing specific tags to the repository from **any branch**. It will **NOT** build on regular commits. Tags can be created and pushed from main, development branches, or any other branch.

### Tag Formats

Use the following tag patterns to trigger builds:

#### Build macOS Only
```bash
git tag v1.0.0-macos
git push origin v1.0.0-macos
```

#### Build Windows Only
```bash
git tag v1.0.0-windows
git push origin v1.0.0-windows
```

#### Build Both Platforms
```bash
git tag v1.0.0-all
git push origin v1.0.0-all
```

## Release Process

### 1. Prepare Your Code
- Ensure all changes are committed (on any branch)
- Test the application locally
- Update README.md or CHANGELOG if needed

### 2. Create and Push a Tag

**Note:** You can create tags from any branch (main, develop, feature branches, etc.)

For a macOS-only release:
```bash
git tag v1.0.0-macos
git push origin v1.0.0-macos
```

For a Windows-only release:
```bash
git tag v1.0.0-windows
git push origin v1.0.0-windows
```

For both platforms:
```bash
git tag v1.0.0-all
git push origin v1.0.0-all
```

**Example from a feature branch:**
```bash
# You're on a feature branch
git checkout feature/new-feature
git tag v1.1.0-beta-all
git push origin v1.1.0-beta-all
```

### 3. Monitor the Build
- Go to the "Actions" tab in your GitHub repository
- Watch the build progress for your tag
- The workflow will:
  - Build the application(s) for the specified platform(s)
  - Package the binaries with documentation
  - Create a GitHub release with the tag name
  - Attach the platform binaries to the release

### 4. Verify the Release
- Go to the "Releases" section in your GitHub repository
- Verify the release was created with the correct tag
- Download and test the binaries

## Release Artifacts

### macOS Release (`*-macos` or `*-all`)
- **File**: `BeamCommander-macos.tar.gz`
- **Contains**:
  - `BeamCommander.app` (macOS application bundle)
  - README.md
  - LICENSE.md
  - DEVELOPER.md
  - doc/ (documentation and demo files)

### Windows Release (`*-windows` or `*-all`)
- **File**: `BeamCommander-windows.zip`
- **Contains**:
  - `BeamCommander.exe` (Windows executable)
  - Required DLL files
  - README.md
  - LICENSE.md
  - DEVELOPER.md
  - doc/ (documentation and demo files)

## Version Numbering Recommendations

Use semantic versioning for your tags:
- `v1.0.0-macos` - Major release
- `v1.1.0-windows` - Minor update
- `v1.1.1-all` - Patch/bugfix

Or use date-based versions:
- `v2025.10.15-macos` - macOS release on Oct 15, 2025
- `v2025.10.15-all` - Both platforms on Oct 15, 2025

## Troubleshooting

### Windows Build Requirements

For Windows builds to work, you need **one of the following**:

1. **Visual Studio Project Files** (recommended for building from source):
   - Add `.sln` (solution) and `.vcxproj` (project) files to the repository
   - Generate these using openFrameworks' projectGenerator tool
   - Commit them to the repository

2. **Pre-built Executable**:
   - Build `BeamCommander.exe` locally on Windows
   - Place it in `openframeworks-src-master/apps/myApps/BeamCommander/bin/`
   - Commit the pre-built executable to the repository

**Note:** The workflow will automatically detect which method is available and use it.

### Build Fails
- Check the Actions logs for error messages
- Verify that pre-built binaries exist in the repository (for macOS)
- Ensure Visual Studio project files exist (for Windows)

### Release Not Created
- Verify the tag was pushed successfully
- Check that at least one build job succeeded
- Review the Actions logs for the release step

### Wrong Platform Built
- Double-check the tag suffix:
  - `-macos` for macOS only
  - `-windows` for Windows only
  - `-all` for both platforms

## Deleting a Tag (if needed)

If you need to delete a tag and retry:

```bash
# Delete local tag
git tag -d v1.0.0-macos

# Delete remote tag
git push --delete origin v1.0.0-macos
```

Then create and push the tag again.
