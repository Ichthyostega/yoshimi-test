# Yoshimi-Testsuite


[Yoshimi](http://yoshimi.sourceforge.net/) is a software sound synthesizer, derived from ZynAddSubFx
and developed by an OpenSource community. This Repository **Yoshimi-test** is used by the Yoshimi developers
to maintain a suite of automated acceptance tests for the Yoshimi application. Running these tests regularly
and for each release helps to detect regressions in functionality or performance caused by recent coding work.
By virtue of a special invocation and rigging, the tests can reproduce sound calculations exactly and compare
the created output to a stored baseline

- a *TestInvoker* built into the Yoshimi CLI interface allows to setup parts and voices and then to calculate
  and capture isolated test notes, together with timing measurements
- *(planned)* by loading Yoshimi as a LV2 plug-in, the test runner is able to perform complex integration tests,
  without having to rely on any actual MIDI or sound I/O backend

Similar to Yoshimi itself, this Testsuite is GPL licensed free software.


## Building

The '`yoshimi-testrunner`' subdirectory holds a C++ / CMake project to build a stand-alone testrunner application.
In addition you need a Yoshimi executable, to be invoked by this testrunner as a test subject. There are various
ways to handle a CMake project, e.g. with `cmake-gui` or `ccmake` or with your IDE of choice. However, in its
simplest form, the build can be started with...

    cmake -S yoshimi-testrunner -B yoshimi-testrunner/build
    cmake --build yoshimi-testrunner/build


### Dependencies

- a C++17 compliant compiler
- CMake 3.12 or better
- libSndfile (e.g. `libsndfile1-dev` on Debian/Ubuntu)

Note: we use libSndfile only for reading/writing WAV files. If further dependencies are problematic,
you might `configure --disable-alsa --disable-external-lib` to omit ALSA, FLAC and Vorbis.


## Launching the tests

The Yoshimi-test tree is self-contained and does not need to be installed; rather, after building, you'd invoke
the executable and point it to the 'testsuite' directory. In the simplest form

    ./run-tests testsuite

This will use the Yoshimi installed into the sytem as /usr/bin/yoshimi and perform all tests defined within
the 'testsuite' subdirectory. Alternatively you might want to use another Yoshimi executable:


    ./run-tests --subject=/path/to/yoshimi  testsuite

The basic configuration for the tests can be found in the file '`testsuite/defaults.ini`'. Typically, you do not
want to edit this directly, since it is checked into Git -- rather, for adapting the testsuite to your local
situation, create a file 'setup.ini' in the current working directory; these local settings will take precedence
over the defaults.


## The Testsuite

Both *System Testing* and *Acceptance Testing* rely on a black box approach to verify the application as a whole,
without looking into details of the inner workings. Since ZynAddSubFX / Yoshimi development was never based on a
formal specification, the goal is to ensure the known behaviour of the sound generation is not altered inadvertently,
thus allowing musicians to learn and rely on the fine points and specific traits of existing presets and instruments.
However, it is hard, if not impossible to describe the musically relevant traits of an instrument in a formalised
way that can be verified objectively. Rather, by approximation, we attempt to produce isolated test notes under well
defined starting conditions and compare the calculated sound against a **baseline**, stored as reference wave files.
If any differences are detected, they will be stored as **residual** wave files for further (manual) investigation.
The baseline wave files are checked into this Git repository for ease of distribution and to document changes.
A special *baseline capturing mode* of the testrunner (`--baseline`) allows to create new baseline wave files or
overwrite existing baselines in case the currently produced sound differs.

As the tests cover various aspects of the SynthEngine, they are organised into several sections

- **`features`** covers the expected behaviour of some isolated building blocks, e.g. basic waveform, LFO, filters
- **`patches`** verifies interesting combinations of the base features and some complete voices, thereby
  also stressing known corner cases
- **`scenes`** *(planned)* renders exemplary MIDI sequences by invoking Yoshimi as LV2 plugin, allowing
  to cover some aspects of MIDI handling and performance characteristics of the application.


### Test Case specification

A **Test Case** covers one specific aspect or topic, it is performed with one launch of the subject under test,
which can *succeed* or *fail* (either because the expectations aren't met, or due to execution failure).
All test cases are defined through files '`*.test`', placed into a suitable subdirectory of the Testsuite.
The relative filesystem path from the Testsuite root to the actual test case is used as *"Topic-Path"* to
structure and organise the testsuite and the result report.

The syntax of Test Case specifications is based on the syntax for configuration ("INI file syntax"), with some
extensions

- there must be a *section* `[Test]` — all setting keys following that section start are implicitly prefixed
  by the section name, i.e. "`Test.<theKey>`"
- within the Test section, optionally a `Test.type` can be defined...
  + default is `Test.type=CLI` and causes Yoshimi to be launched as a subprocess, feeding the test through CLI
  + *(planned)* alternatively `Test.type=LV2` will load Yoshimi as a LV2 plugin, allowing for tests with MIDI files
- by default, Yoshimi is launched with the commandline options `--null --no-gui` (as defined in 'setup.ini').
  This argument line can be replaced completely by the setting `arguments`; you may also add further arguments
  at the end of the existing commandline with `addArguments`. The string given here will be split into words;
  use qouting (with `"` or `'`) to retain whitespace within a specific argument.
- for CLI-tests, a *CLI script* should be defined, which is sent through the Yoshimi CLI in order to configure
  and launch the test
  + this script is defined *inline* within the test specification
  + it starts with a marker line comprised solely of the word `Script` and it ends with a similar marker line `End-Script`
  + the text between those markers is *trimmed* and then sent line by line to the Yoshimi subprocess
  + thus, at some point this script must descend into the `test` context of the CLI (e.g. by the command `set test`)
  + and finally, this script shall issue the command `exec` within the test context.
  + this `exec`-command will shut down regular Yoshimi operation, then launch the test as configured, and then terminate Yoshimi.
- optionally further key=value definitions can be given in the `[Test]`-section to override the built-in defaults
  - `verifySound = On|Off` toggles verification of the generated sound data against a baseline wave file
  - `verifyTimes = On|Off` allows to compare the timing measurements against expected values (**TODO** define details)
  - `cliTimeout = <integer number>` overrides the built-in timeout when waiting for response on CLI actions.
    Typically the testrunner waits for some specific token or prefix to appear in the output stream from the Yoshimi subprocess;
    if the timeout threshold is exceeded, the subprocess will be killed and the test case counted as failure.
—
