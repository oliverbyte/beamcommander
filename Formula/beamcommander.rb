class Beamcommander < Formula
  desc "Free & open-source laser control system for live performances"
  homepage "https://oliverbyte.github.io/beamcommander/"
  url "https://github.com/oliverbyte/beamcommander/archive/refs/tags/v1.0.0.tar.gz"
  # TODO: Update SHA256 after creating the release tag
  # Calculate with: curl -L <URL> | shasum -a 256
  sha256 "" # PLACEHOLDER - Must be updated before use
  license "MIT"
  head "https://github.com/oliverbyte/beamcommander.git", branch: "main"

  depends_on "make" => :build
  depends_on xcode: :build
  depends_on :macos

  def install
    # Navigate to BeamCommander directory
    cd "openframeworks-src-master/apps/myApps/BeamCommander" do
      # Build the application
      system "make", "-j#{ENV.make_jobs}"
      
      # Install frameworks/libraries first
      if File.directory?("bin/BeamCommander.app/Contents/Frameworks")
        lib_dir = lib/"beamcommander"
        lib_dir.mkpath
        # Install all dylib files from Frameworks directory
        Dir["bin/BeamCommander.app/Contents/Frameworks/*.dylib"].each do |framework|
          lib_dir.install framework
        end
        
        # Update library references in the binary
        binary_path = "bin/BeamCommander.app/Contents/MacOS/BeamCommander"
        Dir["bin/BeamCommander.app/Contents/Frameworks/*.dylib"].each do |dylib|
          dylib_name = File.basename(dylib)
          system "install_name_tool", "-change",
                 "@executable_path/../Frameworks/#{dylib_name}",
                 "#{lib_dir}/#{dylib_name}",
                 binary_path
        end
      end
      
      # Create wrapper script
      (bin/"beamcommander").write <<~EOS
        #!/bin/bash
        # BeamCommander wrapper script
        export DYLD_LIBRARY_PATH="#{lib}/beamcommander:$DYLD_LIBRARY_PATH"
        exec "#{lib}/beamcommander/BeamCommander" "$@"
      EOS
      
      # Install the actual binary to lib directory
      lib_dir = lib/"beamcommander"
      lib_dir.mkpath
      lib_dir.install "bin/BeamCommander.app/Contents/MacOS/BeamCommander"
      
      # Install the data directory if it exists
      if File.directory?("bin/data")
        pkgshare.install "bin/data"
      end
      
      # Install configuration files
      pkgshare.install "open-stage-control-server.config"
      pkgshare.install "open-stage-control-session.json"
    end
    
    # Install documentation from root
    doc.install "README.md"
    doc.install "DEVELOPER.md"
    doc.install "LICENSE.md"
    doc.install "INSTALL.md"
    doc.install Dir["doc/*"] if File.directory?("doc")
  end

  def caveats
    <<~EOS
      BeamCommander has been installed!
      
      To run BeamCommander:
        beamcommander
      
      Configuration files are available at:
        #{pkgshare}/open-stage-control-server.config
        #{pkgshare}/open-stage-control-session.json
      
      Documentation is available at:
        #{doc}
      
      For detailed installation and setup instructions:
        #{doc}/INSTALL.md
      
      Note: BeamCommander requires:
      - Compatible laser DAC hardware (EtherDream, Helios, LaserDock, etc.)
      - Optional: Akai APC40 MIDI controller
      - Optional: Open Stage Control for web interface
      
      The application listens for OSC commands on UDP port 9000.
    EOS
  end

  test do
    # Test that the binary exists and is executable
    assert_predicate bin/"beamcommander", :exist?
    assert_predicate bin/"beamcommander", :executable?
  end
end
