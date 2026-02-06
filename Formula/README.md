# BeamCommander Homebrew Formula

This directory contains the Homebrew formula for BeamCommander.

## For Users

### Installing BeamCommander via Homebrew

To install BeamCommander using Homebrew:

```bash
# Add the BeamCommander tap
brew tap oliverbyte/beamcommander https://github.com/oliverbyte/beamcommander

# Install BeamCommander
brew install beamcommander

# Run BeamCommander
beamcommander
```

### Updating

To update to the latest version:

```bash
brew update
brew upgrade beamcommander
```

### Uninstalling

To remove BeamCommander:

```bash
brew uninstall beamcommander
brew untap oliverbyte/beamcommander
```

## For Developers and Maintainers

### Testing the Formula Locally

Before releasing changes to the formula:

```bash
# Test installation from the local formula file
brew install --build-from-source ./Formula/beamcommander.rb

# Run the application to verify it works
beamcommander

# Run formula tests
brew test beamcommander

# Audit for any issues
brew audit --strict ./Formula/beamcommander.rb

# Clean up after testing
brew uninstall beamcommander
```

### Updating the Formula for New Releases

1. **Create and push a new git tag**:
   ```bash
   git tag -a v1.0.0 -m "Release version 1.0.0"
   git push origin v1.0.0
   ```

2. **Calculate the SHA256 of the release tarball**:
   ```bash
   curl -L https://github.com/oliverbyte/beamcommander/archive/refs/tags/v1.0.0.tar.gz | shasum -a 256
   ```

3. **Update the formula** (`Formula/beamcommander.rb`):
   - Update the `url` to point to the new tag
   - Update the `sha256` with the calculated value

4. **Test the updated formula**:
   ```bash
   brew install --build-from-source ./Formula/beamcommander.rb
   brew test beamcommander
   brew audit --strict ./Formula/beamcommander.rb
   ```

5. **Commit and push the changes**:
   ```bash
   git add Formula/beamcommander.rb
   git commit -m "Update Homebrew formula to v1.0.0"
   git push
   ```

### Formula Structure

The formula:
- Builds BeamCommander from source using `make`
- Installs the binary to Homebrew's `bin` directory
- Installs shared libraries to Homebrew's `lib` directory
- Installs data files and configurations to Homebrew's `share` directory
- Installs documentation to Homebrew's `doc` directory
- Creates a wrapper script to handle library paths

### More Information

For detailed information about maintaining the formula, see [HOMEBREW.md](../HOMEBREW.md) in the repository root.

## Documentation

- **Installation Guide**: [INSTALL.md](../INSTALL.md)
- **User Documentation**: [README.md](../README.md)
- **Developer Guide**: [DEVELOPER.md](../DEVELOPER.md)
- **Homebrew Maintenance**: [HOMEBREW.md](../HOMEBREW.md)

## Support

If you encounter issues with the Homebrew installation:

1. Check the [INSTALL.md](../INSTALL.md) troubleshooting section
2. Review [existing issues](https://github.com/oliverbyte/beamcommander/issues)
3. Create a [new issue](https://github.com/oliverbyte/beamcommander/issues/new)

## License

BeamCommander is licensed under the MIT License. See [LICENSE.md](../LICENSE.md) for details.
