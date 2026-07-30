# Testing the Homebrew Formula

This guide provides instructions for testing the BeamCommander Homebrew formula before releasing it to users.

## Prerequisites

- macOS 10.15 or later
- Homebrew installed
- Xcode Command Line Tools installed

## Testing Checklist

Use this checklist when testing a new formula or formula updates:

### 1. Syntax Check

```bash
# Check Ruby syntax
ruby -c Formula/beamcommander.rb

# Should output: Syntax OK
```

### 2. Formula Audit

```bash
# Run Homebrew's audit tool
brew audit --strict Formula/beamcommander.rb

# Fix any style issues
brew style --fix Formula/beamcommander.rb
```

### 3. Test Local Installation

```bash
# Install from the local formula file
brew install --build-from-source ./Formula/beamcommander.rb

# Verify installation
which beamcommander
# Should output: /usr/local/bin/beamcommander or /opt/homebrew/bin/beamcommander

# Check the binary is executable
file $(which beamcommander)
# Should show: ASCII text executable
```

### 4. Test Running the Application

```bash
# Run the application
beamcommander

# Check if it starts without errors
# Note: You may need to close it with Ctrl+C since it's a GUI application
```

### 5. Verify Installed Files

```bash
# Check installed files
brew list beamcommander

# Verify key files exist:
# - Binary: /usr/local/bin/beamcommander (or /opt/homebrew/bin/beamcommander)
# - Libraries: /usr/local/lib/beamcommander/ (or /opt/homebrew/lib/beamcommander/)
# - Config: /usr/local/share/beamcommander/ (or /opt/homebrew/share/beamcommander/)
# - Docs: /usr/local/share/doc/beamcommander/ (or /opt/homebrew/share/doc/beamcommander/)
```

### 6. Test Formula Tests

```bash
# Run the formula's test block
brew test beamcommander

# Should pass without errors
```

### 7. Check Info

```bash
# Display formula information
brew info beamcommander

# Should show:
# - Description
# - Homepage
# - Dependencies
# - Installation status
```

### 8. Test Uninstallation

```bash
# Uninstall
brew uninstall beamcommander

# Verify removal
which beamcommander
# Should output: (nothing) or "not found"
```

### 9. Test Installation from Tap

```bash
# Add the tap
brew tap oliverbyte/beamcommander https://github.com/oliverbyte/beamcommander

# Install from tap
brew install beamcommander

# Verify it works
beamcommander

# Clean up
brew uninstall beamcommander
brew untap oliverbyte/beamcommander
```

## Testing on Different macOS Versions

Test the formula on various macOS versions if possible:

- macOS 10.15 (Catalina) - minimum supported version
- macOS 11 (Big Sur)
- macOS 12 (Monterey)
- macOS 13 (Ventura)
- macOS 14 (Sonoma)
- macOS 15 (Sequoia)

## Testing on Different Architectures

Test on both Intel and Apple Silicon:

- Intel (x86_64): Test on Intel-based Mac
- Apple Silicon (ARM64): Test on M1/M2/M3 Mac

## Common Issues and Solutions

### Build Fails

**Problem**: `make` fails during build

**Solutions**:
- Verify Xcode Command Line Tools: `xcode-select --install`
- Check OpenFrameworks dependencies
- Review build logs for specific errors

### Library Loading Errors

**Problem**: Binary can't find libfmod.dylib

**Solutions**:
- Check wrapper script is correctly setting DYLD_LIBRARY_PATH
- Verify libraries are installed to the correct location
- Use `otool -L` to inspect library dependencies

### Formula Audit Failures

**Problem**: `brew audit` reports issues

**Solutions**:
- Fix style issues: `brew style --fix Formula/beamcommander.rb`
- Review Homebrew style guide
- Check for common formula issues

## Creating a Test Release

To test the formula with a real release:

1. **Create a test tag**:
   ```bash
   git tag -a v1.0.0-test -m "Test release"
   git push origin v1.0.0-test
   ```

2. **Calculate SHA256**:
   ```bash
   curl -L https://github.com/oliverbyte/beamcommander/archive/refs/tags/v1.0.0-test.tar.gz | shasum -a 256
   ```

3. **Update formula**:
   ```ruby
   url "https://github.com/oliverbyte/beamcommander/archive/refs/tags/v1.0.0-test.tar.gz"
   sha256 "ACTUAL_SHA256_HERE"
   ```

4. **Test installation**:
   ```bash
   brew install --build-from-source ./Formula/beamcommander.rb
   ```

5. **Clean up test tag** (if needed):
   ```bash
   git tag -d v1.0.0-test
   git push origin :refs/tags/v1.0.0-test
   ```

## Automated Testing

Consider adding automated testing in CI:

```yaml
# Example GitHub Actions workflow
- name: Test Homebrew Formula
  run: |
    brew install --build-from-source ./Formula/beamcommander.rb
    brew test beamcommander
    brew audit --strict ./Formula/beamcommander.rb
```

## Documentation

After successful testing, update:

- [ ] CHANGELOG.md with release notes
- [ ] Formula version and SHA256
- [ ] README.md if installation instructions changed
- [ ] INSTALL.md if setup procedures changed

## Sign-off Checklist

Before considering the formula ready for release:

- [ ] Ruby syntax is valid
- [ ] Formula passes `brew audit --strict`
- [ ] Installs successfully from local file
- [ ] Application runs without errors
- [ ] All files are installed correctly
- [ ] Formula tests pass
- [ ] Works on both Intel and Apple Silicon (if possible)
- [ ] Works on multiple macOS versions (if possible)
- [ ] Uninstallation is clean
- [ ] Installation from tap works
- [ ] Documentation is updated

## Getting Help

If you encounter issues during testing:

1. Check the [Homebrew Formula Cookbook](https://docs.brew.sh/Formula-Cookbook)
2. Review [Homebrew Troubleshooting](https://docs.brew.sh/Troubleshooting)
3. Search [Homebrew issues](https://github.com/Homebrew/brew/issues)
4. Ask in [Homebrew discussions](https://github.com/Homebrew/discussions/discussions)

## Resources

- [Homebrew Formula Cookbook](https://docs.brew.sh/Formula-Cookbook)
- [Homebrew Formula Style Guide](https://docs.brew.sh/Formula-Cookbook#style-guide)
- [Homebrew Testing Guide](https://docs.brew.sh/Formula-Cookbook#testing)
- [Formula Audit Documentation](https://docs.brew.sh/Formula-Cookbook#audit)
