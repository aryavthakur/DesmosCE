# Compatibility and evidence boundary

| Component | Evidence | What remains unverified |
|---|---|---|
| TI-84 Plus CE Python native ABI | Actual v15.0 eZ80 cross-build | Launch, input latency and sustained operation on the device |
| Linux host math/editor/app | C tests with ASan/UBSan; independent references | Native library rounding differences and hardware-only faults |
| GraphX/KeypadC/FileIOC | Compiled against v15.0 headers/libraries | Real LCD clipping, keys, OS transitions and archive operations |
| arTIfiCE v3 / OS 5.8.5 | Official compatibility/instructions checked 2026-10-10 | DesmosCE-specific launch and save tests |
| arTIfiCE v2.1 / earlier supported CE OS | Official compatibility documentation | DesmosCE-specific launch and save tests |
| TI-84 Plus CE-T / TI-83 Premium CE variants | Related upstream CE toolchain/launcher targets | Not a validated DesmosCE support claim |
| Non-CE TI-84 / C Silver Edition / TI-Nspire | Different target | Unsupported |
| Cemu | No user ROM supplied; not run | Every ROM-backed emulator acceptance test |

Sources: [CE hardware and types](https://ce-programming.github.io/toolchain/static/hardware.html), [toolchain v15.0](https://github.com/CE-Programming/toolchain/releases/tag/v15.0), [arTIfiCE](https://yvantt.github.io/arTIfiCE/).

The app deliberately uses native C rather than the Python coprocessor. CE `int`/pointers are 24-bit and CE `double` is not the host's usual binary64. `Real` therefore maps to 64-bit `long double` on CE and 64-bit `double` on the test host. Storage asserts `sizeof(Real)==8` and writes a padding-independent binary64 format.

An upstream compatibility statement is not a test result for DesmosCE. Neither host previews nor successful linking close the physical-device verification gap.
