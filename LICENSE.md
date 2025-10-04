# BeamCommander - License Information

BeamCommander is a composite project that incorporates multiple open-source components with different licenses. This document provides comprehensive licensing information for all components.

## BeamCommander Application Code

The BeamCommander application code (original code in `openframeworks-src-master/apps/myApps/BeamCommander/src/`) is distributed under the **MIT License**.

### MIT License

Copyright (c) 2024-2025 BeamCommander Project

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

---

## Third-Party Components

### OpenFrameworks v0.12.0 (Modified)

**Location**: `openframeworks-src-master/` (excluding `addons/ofxLaser/`)  
**License**: [MIT License](https://en.wikipedia.org/wiki/MIT_License)  
**Copyright**: (c) 2004 - openFrameworks Community  
**Website**: [openframeworks.cc](https://openframeworks.cc/)

OpenFrameworks is distributed under the MIT License. The full license text can be found at `openframeworks-src-master/LICENSE.md`.

**Modifications Made**: Architecture compatibility fixes, build system optimizations, ImGui integration improvements.

### ofxLaser of_11.0.2 Branch (Modified)

**Location**: `openframeworks-src-master/addons/ofxLaser/`  
**License**: [MIT License](https://opensource.org/licenses/MIT)  
**Author**: Seb Lee-Delisle  
**Website**: [github.com/sebleedelisle/ofxLaser/tree/of_11.0.2](https://github.com/sebleedelisle/ofxLaser/tree/of_11.0.2)

ofxLaser is based on the of_11.0.2 branch (legacy version for OpenFrameworks v0.11.x) and is distributed under the MIT License. The full license text can be found at `openframeworks-src-master/addons/ofxLaser/LICENSE`.

**Modifications Made**: Complete joystick support removal, ImGui stability improvements, enhanced exit safety procedures, build system updates.

### Additional OpenFrameworks Addons

The following standard OpenFrameworks addons are included with their respective licenses:

- **ofxOsc**: MIT License (part of OpenFrameworks core addons)
- **ofxMidi**: MIT License (part of OpenFrameworks core addons)
- **ofxOpenCv**: MIT License (part of OpenFrameworks core addons) 
- **ofxNetwork**: MIT License (part of OpenFrameworks core addons)
- **ofxPoco**: MIT License (part of OpenFrameworks core addons)

### Bundled Libraries

OpenFrameworks ships with various third-party libraries, each with their own licenses:

- **OpenGL**, **GLEW**, **GLUT**, **libtess2**, **cairo** - Graphics libraries
- **rtAudio**, **PortAudio**, **FMOD**, **Kiss FFT** - Audio libraries
- **FreeType** - Font rendering
- **FreeImage** - Image processing
- **Poco** - Utilities
- **ImGui** - Immediate mode GUI (included in ofxLaser)

For detailed license information on these libraries, refer to `openframeworks-src-master/docs/libraries.md`.

---

## License Compatibility Notice

This project combines:
- **MIT Licensed** components (OpenFrameworks, BeamCommander application code, ofxLaser, and standard addons)
- **Various Open Source Licensed** third-party libraries (see bundled libraries section above)

All major components (OpenFrameworks, ofxLaser of_11.0.2 branch, and BeamCommander application code) use the MIT License, providing maximum compatibility and freedom for both commercial and non-commercial use.

## Important Notes

### Laser Safety Disclaimer

**LASERS ARE DANGEROUS - USE THIS SOFTWARE AT YOUR OWN RISK. NEVER LOOK INTO THE BEAM.**

This software is provided for laser control applications. Users must:
- Take proper laser safety training
- Obtain necessary licenses for high-power laser operation in their jurisdiction
- Implement appropriate safety measures including emergency stops
- Never allow unauthorized access to laser exposure areas
- Check for reflective surfaces in the exposure area

The authors and contributors accept no responsibility for injuries, property damage, or legal violations resulting from the use of this software.

### FMOD Licensing Notice

If using FMOD for audio (default in some OpenFrameworks builds), note that FMOD has specific licensing requirements:
- **Non-commercial use**: Free
- **Commercial use**: Requires FMOD license from Firelight Technologies

Refer to [FMOD licensing](https://www.fmod.com/licensing) for details.

---

## Contact

For licensing questions or commercial use inquiries, please create an issue in the project repository.

For OpenFrameworks licensing questions: [openframeworks.cc](https://openframeworks.cc/)  
For ofxLaser licensing questions: [Seb Lee-Delisle](https://github.com/sebleedelisle/ofxLaser)