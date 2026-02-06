# Homebrew Formula Maintenance Guide

This guide explains how to maintain and publish the BeamCommander Homebrew formula.

## Overview

The BeamCommander Homebrew formula is located at `Formula/beamcommander.rb`. This allows users to install BeamCommander using Homebrew, which is the recommended installation method for macOS users.

## Formula Location

The formula can be used in two ways:

1. **As a Tap** (Recommended): Users can tap this repository directly
2. **In Homebrew Core**: Eventually, this could be submitted to Homebrew's main repository

## Using the Formula as a Tap

### For Users

Users can install BeamCommander by tapping this repository:

```bash
# Add the tap
brew tap oliverbyte/beamcommander https://github.com/oliverbyte/beamcommander

# Install
brew install beamcommander

# Run
beamcommander
```

### For Maintainers

When you update the formula:

1. Make changes to `Formula/beamcommander.rb`
2. Commit and push to the repository
3. Users can update with:
   ```bash
   brew update
   brew upgrade beamcommander
   ```

## Updating the Formula for New Releases

When releasing a new version:

### 1. Create a GitHub Release

```bash
# Tag the release
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin v1.0.0
```

### 2. Update the Formula

Edit `Formula/beamcommander.rb`:

```ruby
url "https://github.com/oliverbyte/beamcommander/archive/refs/tags/v1.0.0.tar.gz"
sha256 "NEW_SHA256_HERE"
```

### 3. Calculate the SHA256

```bash
# Download the tarball
curl -L https://github.com/oliverbyte/beamcommander/archive/refs/tags/v1.0.0.tar.gz -o beamcommander.tar.gz

# Calculate SHA256
shasum -a 256 beamcommander.tar.gz
```

### 4. Test the Formula

```bash
# Install locally
brew install --build-from-source Formula/beamcommander.rb

# Test it
beamcommander --help

# Audit the formula
brew audit --strict Formula/beamcommander.rb

# Uninstall test version
brew uninstall beamcommander
```

### 5. Commit and Push

```bash
git add Formula/beamcommander.rb
git commit -m "Update formula to v1.0.0"
git push
```

## Testing the Formula Locally

Before pushing changes, test the formula:

```bash
# Test installation from local formula
brew install --build-from-source ./Formula/beamcommander.rb

# Run the application
beamcommander

# Check it works
brew test beamcommander

# Audit for issues
brew audit --strict ./Formula/beamcommander.rb

# Uninstall when done testing
brew uninstall beamcommander
```

## Formula Structure

### Key Components

```ruby
class Beamcommander < Formula
  desc "..."           # Short description
  homepage "..."       # Project homepage
  url "..."           # Source tarball URL
  sha256 "..."        # SHA256 of the tarball
  license "MIT"       # License
  head "..."          # Development version

  depends_on ...      # Dependencies

  def install
    # Build and install logic
  end

  def caveats
    # Post-install message
  end

  def test
    # Test logic
  end
end
```

### Dependencies

The formula declares these dependencies:

- `make` (build dependency)
- `xcode` (build dependency, macOS only)
- `:macos` (platform requirement)

### Install Process

The formula:

1. Builds BeamCommander using `make`
2. Installs the binary to the Homebrew bin directory
3. Installs libraries to the Homebrew lib directory
4. Installs data files to the Homebrew share directory
5. Installs documentation to the Homebrew doc directory

## Submitting to Homebrew Core (Future)

To eventually submit BeamCommander to Homebrew's main repository:

### Prerequisites

1. The formula must be in a tap for at least 30 days
2. The software should be notable (stars, users, etc.)
3. The formula must pass all audits
4. Must have a stable release with semantic versioning

### Submission Process

1. **Fork homebrew-core**:
   ```bash
   gh repo fork homebrew/homebrew-core --clone
   ```

2. **Create formula in homebrew-core**:
   ```bash
   cd homebrew-core
   cp /path/to/Formula/beamcommander.rb Formula/beamcommander.rb
   ```

3. **Test thoroughly**:
   ```bash
   brew install --build-from-source ./Formula/beamcommander.rb
   brew test beamcommander
   brew audit --strict --online ./Formula/beamcommander.rb
   ```

4. **Create PR**:
   ```bash
   git checkout -b beamcommander
   git add Formula/beamcommander.rb
   git commit -m "beamcommander 1.0.0 (new formula)"
   gh pr create --title "beamcommander 1.0.0 (new formula)"
   ```

5. **Address review feedback** from Homebrew maintainers

## Common Issues and Solutions

### Build Failures

If the build fails:

1. Check that all dependencies are declared
2. Verify the build commands work manually
3. Check OpenFrameworks compilation requirements
4. Test on different macOS versions

### Library Path Issues

The formula uses a wrapper script to set `DYLD_LIBRARY_PATH` for finding libfmod.dylib. If users report library loading errors:

1. Verify the wrapper script is being created correctly
2. Check that libraries are installed to the correct location
3. Use `otool -L` to inspect library dependencies

### Formula Audit Failures

Common audit issues:

```bash
# Fix style issues
brew style --fix Formula/beamcommander.rb

# Check for common problems
brew audit --strict Formula/beamcommander.rb

# Test on clean system
brew install --build-from-source Formula/beamcommander.rb
```

## Resources

- [Homebrew Formula Cookbook](https://docs.brew.sh/Formula-Cookbook)
- [Homebrew Acceptable Formulae](https://docs.brew.sh/Acceptable-Formulae)
- [How to Create Homebrew Taps](https://docs.brew.sh/How-to-Create-and-Maintain-a-Tap)
- [Homebrew Formula Style Guide](https://docs.brew.sh/Formula-Cookbook#style-guide)

## Maintenance Checklist

- [ ] Test formula on Intel Mac
- [ ] Test formula on Apple Silicon Mac
- [ ] Verify all dependencies are correct
- [ ] Update SHA256 for each release
- [ ] Test clean install
- [ ] Test upgrade from previous version
- [ ] Update documentation
- [ ] Audit formula for style and correctness
- [ ] Create GitHub release with release notes

## Contact

For questions about formula maintenance, contact the repository maintainers or open an issue on GitHub.
