# Reproducing the development artifact

On Linux, install the pinned CE Toolchain v15.0 and add `CEdev/bin` to PATH. A normal `make` emits the native binary. For the same stable transfer-file checksum used in CI:

```sh
make clean
make -j2
python3 tools/normalize_binary.py
python3 tools/check_size.py
python3 tools/check_matrix.py
```

The CE toolchain normally puts a wall-clock timestamp in the 42-byte TI transfer-file comment. Normalization replaces only that comment with a fixed project/version label. The script verifies signature, length and checksum first and leaves the compressed native program and its checksum unchanged. The locally compiled and initial CI programs had identical payloads before normalization; only two timestamp digits differed.

Host checks are independent of CE headers and use `Makefile.host`. CI installs NumPy 2.2.6 and SciPy 1.15.3, runs ASan/UBSan tests, reference comparisons and the real-renderer preview generator, then compiles and enforces the native static memory gate.

The successful Actions artifact contains the exact Git source snapshot including dotfiles/workflow, source commit, native `.8xp`, linker map, logs, reference results, memory report, previews, notices and a SHA256SUMS manifest. Upstream dependency source is included with example/test font files omitted; application-relevant runtime source and its notices are retained. Runtime AppVars are not bundled and must be downloaded from the official v15 release.

The workflow uses read-only repository permissions. It does not merge branches, publish a stable release, install calculator firmware, or run on a user's physical device. The source snapshot and the recorded commit, not an unrelated download, identify the code behind each build.
