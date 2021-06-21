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

The '`yoshimi-testrunner`' subdirectory holds a C++ / CMake project to build a standalone testrunner application.
In addition you need a Yoshimi executable, to be invoked by this testrunner as a test subject.

### Dependencies

- a C++17 compliant compiler
- CMake 3.12 or better
- libSOX (e.g. `libsox-dev` on Debian/Ubuntu)


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

