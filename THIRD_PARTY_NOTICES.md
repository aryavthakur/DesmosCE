# Third-party notices and rebuild information

The original DesmosCE application is MIT-licensed. This does not replace licenses of dependencies or grant rights to TI/Desmos trademarks or firmware.

## CE C/C++ Toolchain v15.0

The native binary uses CE Toolchain runtime/startup/math facilities and the separately installed GraphX, FileIOC, KeypadC and LibLoad libraries. Upstream's root license is GNU LGPL version 3; individual source files retain their own notices, including permissive notices where applicable. Consult those files rather than assuming a single license for every component.

- Source and individual notices: https://github.com/CE-Programming/toolchain/tree/v15.0
- Root license: https://github.com/CE-Programming/toolchain/blob/v15.0/license
- Compiler distribution: https://github.com/CE-Programming/toolchain/releases/tag/v15.0
- Runtime distribution: https://github.com/CE-Programming/libraries/releases/tag/v15.0

The release artifact accompanies the binary with the complete DesmosCE source, makefiles and install information, an upstream v15.0 toolchain source archive, and verbatim GPL-3.0/LGPL-3.0 license documents. No upstream runtime source has been modified for this application. Runtime `.8xv` libraries are obtained separately; interface-compatible replacements can be installed. Rebuild the supplied application source with a modified compatible CE runtime/toolchain using `make clean && make`. The pinned compiler download and its checksum are recorded in `.github/workflows/build.yml`. Compiler tools themselves need not be installed on the calculator.

Modification of library portions and reverse engineering to debug such modifications are not restricted by this project. Do not redistribute the binary alone while omitting its accompanying source/notices and applicable dependency obligations.

## Other development tools

NumPy and SciPy are host-only numerical references and are not linked into the calculator binary. AddressSanitizer/UndefinedBehaviorSanitizer are host-only tests. The optional host SVG preview relies on the viewer's own fonts; no font files are distributed by DesmosCE. LLVM/clang-format formats the source but is not part of the calculator application.

## External software and branding

arTIfiCE, AsmHook2, TI Connect CE and TI firmware are external installation tools, not bundled application components. Follow their own licenses and official instructions. No ROM or firmware is distributed. The app uses original layout code and simple geometric icons; it does not contain Desmos JavaScript, logos or copied UI assets.
