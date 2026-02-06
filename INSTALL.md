# BeamCommander Installation Guide

## Installation Methods

### Method 1: Homebrew (Recommended for macOS users)

Homebrew provides the easiest way to install and manage BeamCommander on macOS.

#### Installing via Homebrew

1. **Install Homebrew** (if not already installed):
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

2. **Add the BeamCommander tap**:
   ```bash
   brew tap oliverbyte/beamcommander https://github.com/oliverbyte/beamcommander
   ```

3. **Install BeamCommander**:
   ```bash
   brew install beamcommander
   ```

4. **Run BeamCommander**:
   ```bash
   beamcommander
   ```

#### Updating BeamCommander

To update to the latest version:
```bash
brew update
brew upgrade beamcommander
```

#### Uninstalling

To remove BeamCommander:
```bash
brew uninstall beamcommander
brew untap oliverbyte/beamcommander
```

#### Configuration Files

After installation via Homebrew, configuration files are located at:
- **Open Stage Control config**: `$(brew --prefix)/share/beamcommander/open-stage-control-server.config`
- **Open Stage Control session**: `$(brew --prefix)/share/beamcommander/open-stage-control-session.json`
- **Documentation**: `$(brew --prefix)/share/doc/beamcommander/`

You can find the exact paths by running:
```bash
brew info beamcommander
```

### Method 2: Pre-built Binary Release

1. **Download the Release**:
   - Visit the [Releases](https://github.com/oliverbyte/beamcommander/releases) page
   - Download the latest `.tar.gz` or `.zip` file for macOS
   - Extract the archive

2. **Run the Application**:
   - Double-click `BeamCommander.app` 
   - Or run from terminal:
     ```bash
     open BeamCommander.app
     ```
   - Or run the binary directly:
     ```bash
     ./BeamCommander.app/Contents/MacOS/BeamCommander
     ```

### Method 3: Building from Source

For developers or advanced users who want to build from source:

#### Prerequisites
- macOS 10.15 (Catalina) or later
- Xcode Command Line Tools
- Git

#### Build Steps

1. **Clone the repository**:
   ```bash
   git clone https://github.com/oliverbyte/beamcommander.git
   cd beamcommander
   ```

2. **Install Xcode Command Line Tools** (if not already installed):
   ```bash
   xcode-select --install
   ```

3. **Build the application**:
   ```bash
   ./build.sh
   ```

4. **Run the application**:
   ```bash
   ./start_server.sh
   ```

For detailed developer documentation, see [DEVELOPER.md](DEVELOPER.md).

## Post-Installation Setup

### 1. Initial Laser Configuration

When you first run BeamCommander, you'll need to configure your laser hardware:

1. **Add Laser Hardware**: Click "Add Laser" to detect your DAC device
2. **Zone Mapping**: Create and configure at least one output zone
3. **Test Output**: Verify laser output is working
4. **Save Configuration**: Settings are automatically saved for future sessions

### 2. Optional: Install Open Stage Control

For web-based touch control interface:

1. **Download Open Stage Control**: [https://openstagecontrol.ammd.net/](https://openstagecontrol.ammd.net/)
2. **Install the application**
3. **Load the BeamCommander configuration**:
   - Use the configuration files installed with BeamCommander
   - For Homebrew installs, find them with: `brew info beamcommander`

### 3. Optional: Connect MIDI Controller

If using an Akai APC40 MIDI controller:

1. Connect the controller via USB
2. BeamCommander will automatically detect and map the controls
3. Refer to the [MIDI Controller Reference](README.md#midi-controller-reference-akai-apc40) in the README

## Troubleshooting

### Homebrew Installation Issues

**Formula not found**:
```bash
# Re-add the tap
brew untap oliverbyte/beamcommander
brew tap oliverbyte/beamcommander https://github.com/oliverbyte/beamcommander
```

**Build failures**:
```bash
# Check Xcode Command Line Tools
xcode-select --install

# Check Homebrew
brew doctor
```

### Runtime Issues

**"BeamCommander" cannot be opened because the developer cannot be verified**:

On macOS, you may need to allow the application to run:
```bash
# For Homebrew installation
xattr -cr $(brew --prefix)/bin/beamcommander

# For .app bundle
xattr -cr /path/to/BeamCommander.app
```

Then try running again.

**Application crashes on startup**:
- Check that you have macOS 10.15 or later
- Ensure Xcode Command Line Tools are installed
- Check Console.app for error messages

**No laser output**:
- Verify laser DAC hardware is connected and powered
- Check network configuration for Ethernet DACs (e.g., EtherDream at 10.0.1.188)
- Review laser configuration in the application UI

## System Requirements

### Minimum Requirements
- **Operating System**: macOS 10.15 (Catalina)
- **Architecture**: Intel x86_64 or Apple Silicon (ARM64)
- **RAM**: 4 GB
- **Disk Space**: 500 MB

### Recommended Requirements
- **Operating System**: macOS 15.6.1 (Sequoia) or later
- **RAM**: 8 GB or more
- **Network**: Ethernet connection for laser DACs
- **USB**: Ports for MIDI controllers and USB laser hardware

## Additional Resources

- **Main Documentation**: [README.md](README.md)
- **Developer Guide**: [DEVELOPER.md](DEVELOPER.md)
- **OSC API Reference**: [README.md#osc-api-reference](README.md#osc-api-reference)
- **MIDI Controller Mapping**: [README.md#midi-controller-reference-akai-apc40](README.md#midi-controller-reference-akai-apc40)
- **Website**: [https://oliverbyte.github.io/beamcommander/](https://oliverbyte.github.io/beamcommander/)
- **Community Discussions**: [GitHub Discussions](https://github.com/oliverbyte/BeamCommander/discussions)

## Getting Help

If you encounter issues:

1. Check the [Troubleshooting](#troubleshooting) section above
2. Review [existing issues](https://github.com/oliverbyte/beamcommander/issues) on GitHub
3. Join the [community discussions](https://github.com/oliverbyte/BeamCommander/discussions)
4. Create a [new issue](https://github.com/oliverbyte/beamcommander/issues/new) with:
   - Installation method used
   - macOS version
   - Complete error messages
   - Steps to reproduce

## License

BeamCommander is open source software. See [LICENSE.md](LICENSE.md) for details.
