# Windows Build Test Release Instructions

This document explains how to trigger a test Windows build using the GitHub Actions workflow.

## Prerequisites

The Visual Studio project files have been added to this repository:
- `BeamCommander.sln` - Visual Studio solution file
- `BeamCommander.vcxproj` - Visual Studio project file
- `BeamCommander.vcxproj.filters` - Project filters
- `icon.rc` - Icon resource file
- `icon.ico` and `icon_debug.ico` - Application icons

## How to Trigger a Windows Test Build

To create a test release for Windows, create and push a tag with the pattern `v*-windows`:

```bash
# Create a test tag
git tag v1.0-test-windows

# Push the tag to GitHub
git push origin v1.0-test-windows
```

This will trigger the GitHub Actions workflow which will:
1. Download the required openFrameworks libraries for Windows
2. Build BeamCommander using MSBuild with the Visual Studio project files
3. Package the executable with documentation
4. Create a GitHub release with the Windows build artifact

## Alternative Tag Patterns

You can also use:
- `v*-windows` - Build for Windows only
- `v*-macos` - Build for macOS only  
- `v*-all` - Build for both platforms

## Monitoring the Build

1. Go to the "Actions" tab in the GitHub repository
2. Find the workflow run for your tag
3. Monitor the build progress
4. Once complete, the release will appear in the "Releases" section

## Downloading the Build

After the workflow completes successfully:
1. Go to the "Releases" section of the repository
2. Find your release (e.g., "BeamCommander v1.0-test-windows")
3. Download the `BeamCommander-windows.zip` file
4. Extract and test the Windows executable

## Build Artifacts

The Windows build will include:
- `BeamCommander.exe` - Windows executable
- Required DLL files
- README.md
- LICENSE.md
- DEVELOPER.md
- doc/ folder with documentation and demos

## Troubleshooting

If the build fails:
1. Check the Actions logs for error messages
2. Verify the Visual Studio project files are correct
3. Ensure all required addons are properly configured
4. Check the openFrameworks library download step

For more information, see:
- `.github/RELEASE.md` - Complete release guide
- `.github/WINDOWS_BUILD.md` - Windows build setup guide
