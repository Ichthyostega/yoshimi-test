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

- a C++17 compliant compiler (*)
- CMake 3.12 or better
- libSndfile (e.g. `libsndfile1-dev` on Debian/Ubuntu) (**)

Note:
- (*) we also need the C++17 `<filesystem>` — GCC-9 or above should be fine.
- (**): we use libSndfile only for reading/writing WAV files. If further dependencies are problematic,
you might build libSndfile with `configure --disable-alsa --disable-external-libs` to omit ALSA, FLAC and Vorbis.


## Launching the tests

The Yoshimi-test tree is self-contained and does not need to be installed; rather, after building, you'd invoke
the executable and point it to the 'testsuite' directory. In the simplest form

    ./run-tests testsuite

This will use the Yoshimi installed into the system as /usr/bin/yoshimi and perform all tests defined within
the 'testsuite' subdirectory. Alternatively you might want to use another Yoshimi executable:

    ./run-tests --subject=/path/to/yoshimi  testsuite

For the verification of run times, it is necessary to `--calibrate` your setup at least once initially.
Recommendation is to repeat that *platform calibration* after performing the Testsuite about 10 times.
Please read the chapter [Speed and Timing measurements](#speed-and-timing-measurements) to understand
what this entails.

It is possible to *filter the test cases,* and thus to perform only some tests: just add the filter expression(s)
as additional (positional) arguments; if the filter matches somewhere in a test case name or relative path, the
case is included in the current run (regular expression search).


### Configuration

The basic configuration for the tests can be found in the file '`testsuite/defaults.ini`'. Typically, you do not
want to edit this directly, since it is checked into Git — rather, for adapting the testsuite to your local
situation, create a file '`setup.ini`' in the current working directory; these local settings will take precedence
over the defaults. And command-line options with the same name will take precedence over any configuration.

### Controlling Yoshimi state

Yoshimi maintains a lot of persistent setup and wiring, typically stored into files in '`$HOME/.config/yoshimi`'.
However, a complete "Session state" snapshot can be stored and reloaded as `*.state` file. For reproducible results,
the Testsuite must prevent user's configuration from "leaking" into the test execution — a well defined `initial.state`
file is thus loaded for every launched test. By default, this file is placed into the Testsuite root, while each
subdirectory within the Suite may define its own copy, which will take precedence over the root default.


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
- by default, Yoshimi is launched with the commandline options `--null --no-gui` (as defined in 'defaults.ini').
  This argument line can be replaced completely by the setting `arguments`; you may also add further arguments
  at the end of the existing commandline with `addArguments`. The string given here will be split into words;
  use quoting (with `"` or `'`) to retain whitespace within a specific argument.
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
  - `verifyTimes = On|Off` allows to compare the timing measurements against expected values (see below)
  - `cliTimeout = <integer number>` overrides the built-in timeout when waiting for response on CLI actions.
    Typically the testrunner waits for some specific token or prefix to appear in the output stream from the Yoshimi subprocess;
    if the timeout threshold is exceeded, the subprocess will be killed and the test case counted as failure. Increasing this
    timeout can e.g. be relevant when using a debugger to watch the computations within Yoshimi in detail.
  - `warnLevel = <float>` allows to set an individual trigger level for this test case: *any sound difference below*
    this decibel number is classified as "rounding error" and not counted as actual difference any more. The number is
    in decibel peak RMS(30ms) compared to the overall RMS of the baseline WAV file (default warn level: -120dB RMS).


### Detecting sound differences

If a test case is enabled for `verifySound`, the computed sound samples are checked against a known *baseline WAV*.
This reference waveform is given as a file `<testName>-baseline.wav` in the same directory as the test case; typically
such reference files are created with a recent release build of Yoshimi and checked into the Test-Suite Git repository.
The fundamental assumption is that sound computations are deterministic and can thus be reproduced to the last bit,
assuming a proper test setup (especially (re)seeding the pseudo random number generator and only generating a single
note). Thus developers should be able to detect inadvertent changes and fix the code, to keep Yoshimi's behaviour
long-term stable. In practice, this goal can sometimes be hard to keep up, for the following reasons
- the typical release build of Yoshimi uses `--fast-math` combined with `-O3` and SSE extensions.
- under these conditions, the optimiser is allowed to reorder computations for sake of speed, thereby accepting
  small numeric inaccuracies. Moreover, the optimisers on different compilers and platforms are free to choose
  the appropriate means to reach this goal...
- Yoshimi performs most sound computations using `float` numbers (not `double`). The float mantissa has 23 bits,
  so a single least bit flip corresponds to -138dB(FS) and is thus far from any audible effect. However, errors can accumulate,
  they might form patterns, and especially when a numeric error changes changes some frequency or filter coefficient, the
  consequences *can* be audible at times.
- unfortunately, sometimes the developers are *forced to change* minor details, often due to the evolution of platforms
  and libraries, or similar reasons of maintainability. In such a case, baselines must be recomputed and updated in Git.

The Testrunner computes a sample-for-sample difference of the generated sound against the given *baseline.* If there
is *any* difference, a notice will be printed after the individual test case (visible only with `--verbose`). However,
the actual assessment is based on RMS values, comparing the RMS of the difference (computed over a window of 30ms) against
the overall RMS of the test sound. Differences below a warn level (default -120dB RMS) are ignored, to deal with "number dust".
You may launch the Testrunner with the commandline argument `--strict` to force much smaller differences to be reported.

Whenever a relevant change was detected (i.e. above warn level), then also the difference signal is stored into a WAV
file with the name pattern `<testname>-residual.wav`. Since these WAV files use `float` samples, you often won't
hear anything on playback; in these cases, please use a WAV editor and *normalise* the residual to -0dB to make the
actual difference audible. The less noisy and the more sound-like a residual is, the more it might be a real concern.

In case a difference is spotted (or when the baseline file is missing), you may store a new baseline WAV file by
launching the Testsuite with the argument `--baseline` — but beware: this will recapture baseline WAV and timing
expense factors for all deviant test cases. Tip: use the filter feature to only run some dedicated part of the testsuite
if you only want *some* baselines to be changed; moreover, use Git to manage the actual changes. You can `--force`
a re-computation of the baseline in cases where the actual deviation is below the threshold for small changes,
and thus normally no baseline would be captured.


### Speed and Timing measurements

Besides detecting changes in sound generation, the Testsuite also helps to spot changes related to computation speed
and throughput. Unfortunately, the actual performance depends on the execution environment, and any timings tend to
exhibit random fluctuations. This problem can be mitigated by capturing a *time series* and applying *statistics*
on the data collected. Thus, each test case with enabled running time verification (Parameter `Test.verifyTimes = On`)
maintains a data collection, where each further execution of the Testsuite will add another data point. This data is
stored in CSV files within the Testsuite tree, separate for each test case. The actual time measurement happens directly
within Yoshimi itself, in the "TestInvoker" built into the CLI — capturing the pure Synth computation time and data handling
cost between »NoteOn« and »NoteOff«, yet disregarding any communication latency related to MIDI. (Remark 10/2021 we also
intend to add a second test type eventually, based on loading as LV2 plugin, which would allow to observe the MIDI protocol
overhead under controlled conditions). Based on this raw timing data, a *moving average* of the run time can be computed,
short term and long term *trends* can be observed, and a typical *fluctuation bandwidth* can be established for each
test case, allowing to distinguish between ephemeral and relevant timing changes.

#### Configuration related to timings

- `timingsKeep` (default: 500) number of past timing data points to retain for each testcase
- `baselineKeep` (default: 20) history trail of baseline and calibration values to retain
- `baselineAvg` (default: 10) integration interval (i.e. number of points to average) for "the"
   *current value*, for *baseline checks*, the *platform calibration* and *short-term trends*.
- `longtermAvg` (default: 100) integration interval for detecting *long-term trends*.


#### Timing model and Platform Calibration

In regard to the environment-dependent and fluctuating nature of timing data, \
a *simplified Model* for the timings is postulated, to structure and segregate the observed timing into several components

- a common and generic *speed* factor, shared by all computations,
  and based solely on the number of samples to compute; this is a *linear model*, \
  i.e. time = baseOverhead + speed · sampleCnt
- on top of this generic **Platform Model** for the whole Testsuite,
  each individual case is associated with an **Expense Factor**,
  assumed to be static, portable and independent from the environment
- and the actual observed time is assumed to fluctuate randomly and uncorrelated \
  (normal distribution)

> observedTiming ≕ (baseOverhead + speed·sampleCnt) · expenseFactor + ε

Obviously, this is an oversimplification of the real situation, yet seemingly enough to *calibrate* for a given environment,
allowing to *store the Expense Factors* as "Baseline" to *verify observed timings.* Whenever the actual observed timing
is outside the random fluctuation band (ε) around the expected value as defined by the above equation, an *alarm is raised*.
Thus...

- the *Platform Model* is taken to represent the local environment.\
  It is created or updated by running the Testsuite with the `--calibrate` flag,
  and it is only ever stored locally, not checked into Git (there is a `.gitignore` entry to that effect).
- whereas the *Expense Factors* are taken to be *portable*.\
  They are understood as *characteristic property* of the individual test case and can be captured as
  *Baseline* by running the testsuite with the `--baseline` flag. This will change CSV files checked
  into Git and thus shared with the other users of the Testsuite.

#### Usage in practice

Typically you'll start by checking out the Testsuite from the Git repository — and this implies some existing test cases
defined by other people are given, together with established timing baselines for those cases. However, when running
the testsuite for the first time, your actual timing measurements can not be assessed, since the local *Platform Model*
is yet unknown. Thus, we need to run the testsuite at least once with the `--calibrate` flag. After performing all
tests and collecting the run times, a simple linear regression is computed and stored as your *Platform Model* in
the file '`testsuite/Suite-platform.csv`' From this point on, timings can be verified. If nothing else changed,
all timing related tests should be "GREEN" now. (Please report when this isn't the case, since we'd have to investigate
the reason an possibly need to rework the statistics for computing of the tolerance band)

However, *actual coding changes* might have *altered the runtime behaviour*, and you might get an alarm on some test cases
when running the Testsuite. In such a case, either the code needs to be fixed, or otherwise the developers must reach
the conclusion that the changed timings are inevitable or acceptable. In the latter case, run the Testsuite with the
`--baseline` flag and check-in the resulting expectation changes into Git. Likewise, when *adding new test cases*,
a new baseline should be captured, preferably after having performed that test case at least 5-10 times.

There is a certain amount of leeway built into this error detection logic, yet the Testsuite also watches for coherent
ongoing trends just below the trigger level. And sometimes any computation might behave erratic, so be prepared for
getting a random false alarm sometimes. If that happens, and you can not explain the alarm, it might help to look
at the actual timing data captured for each test case; you may need to delete single outlier measurements in case
you're sure no systematic change happened. It might also happen that you'll get repeated sporadic alarms on a single
test case, due to the fact that your local timing measurements are placed consistently close to the border of the
established tolerance band. Such a case needs to be discussed with the other developers, and might indicate that
the stored baseline value is in itself "lopsided" — the actual average expense for this case might be different
than what has been measured and stored initially as a baseline, and the reasons for that should be investigated.


#### Data files

All timing data is stored in CSV files, actually delimited by '`,`' (comma) and with double quoted `"strings"`.
The data in those files is *tabular*, holding a data record in each line, starting with the most recent data record
at top and descending backwards in time. The first line defines expected columns with a column header string.
These strings need to match literally the expectation within the code, and also the number of columns must match.
When observing these constraints, it is possible to load (and even manipulate) this data with a spreadsheet application.

- `<TestID>-runtime.csv`: Time series with the actual run time measurements.
  Each run of the Testsuite will add yet another row at the top of this table and discard the
  oldest measurements after `timingsKeep` rows. (&rarr; TimingObservation.cpp)
  * "Timestamp": the Testsuite run when this data record was captured
  * "Runtime ms": **actual timing measurement** for this test case (milliseconds)
  * "MA Time": moving average over the last `baselineAvg` data points to level out fluctuations
  * "Samples": overall number of samples computed for this test case
  * "Notes": overall number of test notes (NoteOn &harr; NoteOff) in this test case
  * "Platform ms": run time *predicted by current platform model*, given (samples,notes)
  * "Expense": this factor is the *established baseline* for this test case (expectation)
  * "Expense(curr)": effective real expense factor for this actual runtime, computed as `runtime/plattform`
  * "Delta ms": absolute **difference** of current runtime **against expectation**
  * "Tolerance": error bandwidth based on the actual fluctuations observed over the last `baselineAvg`
    data points. Calculated as 3·σ around the moving average of the *preceding* data point; thus we can
    expect the Δ to fluctuate randomly within ± this band. Additionally, we have to take the *fitting error*
    of the Platform Model into account. If the Δ goes beyond those tolerance limits, an alarm is triggered.


- `<TestID>-expense.csv`: Baseline definition with the Expense Factor for this test case.
  * "Timestamp": Testsuite run when this Baseline was established
  * "Averaged points": number of past measurements averaged
  * "Runtime(avg) ms": averaged runtime used to define the Baseline
  * "Samples count": (samples,notes) as currently used by this test case
  * "Notes count"
  * "Platform ms": runtime prediction by the current local platform model, \
    using the given (samples,notes) as input
  * "Expense Factor": resulting **expense baseline**, calculated as
    > expense ≔ averagedRuntime / platformPrediction


- `testsuite/Suite-platform.csv`: Local Platform Model calibration. (&rarr; Timings.cpp)
  * "Timestamp": Testsuite run when this calibration was performed
  * "Data points": number of timing tests fitted for this calibration
  * "Socket ms": **constant offset** of the **regression line**
  * "Speed ns/smp": **gradient** of the **regression line**
  * "Correlation": »Pearson's R« for the fitted data points, -1≤R≤+1,
    should be close to 1, indicating that the linear model describes the real data points well.
  * "Delta (max)": maximum offset of any data point to the regression line
  * "Delta (sdev)": estimation of √Σσ² for this platform fit; this *fitting error*
    indicates the spread of the real averaged measurement points around the regression line


- `testsuite/Suite-platform.csv`: Snapshot of the data used for calculating the
  latest Platform Model (regression line). This data is dumped out to allow manual inspection
  and graphing of the model fit, e.g. by loading it into a spreadsheet application.
  Each line represents a single (averaged) measurement point.
  * "Samples count": predictor variable for the linear regression (X-Value)
  * "Runtime ms": actual (averaged) runtime in milliseconds
  * "Runtime(norm)": after normalising away the (known, established) Expense Factor,
    this is the Y-Value actually used for the regression fit. Thus anything non-linear is
    "stashed" into the Expense Factor and assumed to be a *property of this specific test*
  * "Runtime(model)": Y-value of the regression line for this samples count
  * "Expense Factor": Expense Factor, also used as weight factor on this data point
  * "Delta": difference between Runtime(norm) and model, used to calculate Delta(sdev)
  * "Test-ID": name of the test definition


- `testsuite/Suite-statistic.csv`: ongoing global statistics for the Testsuite,
  computed after performing all timing measurements
  * "Timestamp": the Testsuite run when this global statistic record was computed
  * "Data points": number of timing measurement tests in the suite
  * "Socket ms": current local Platform Model parameter (constant offset)
  * "Speed ns/smp": current local Platform Model parameter (gradient)
  * "Delta (avg)": **averaged Δ** over all test cases; should ideally be zero.
  * "Delta (max)": maximum Δ encountered in any test case
  * "Delta (sdev)": standard deviation of the individual Δ around avgΔ
  * "Tolerance": error tolerance band, based on fluctuation of avgΔ over time



## Hints and Tricks

**Yoshimi settings**: by default, the Testrunner launches Yoshimi with `--null --no-gui`.
Unfortunately Yoshimi remembers the last settings used, which means, after running the
Testsuite, your regular Yoshimi may start without GUI and without audio / MIDI next time.
As of 11/2021 the Yoshimi developers are aware of that problem, but haven't decided yet
if, and how this should be addressed.

**Experimentation**: you may setup your own test suite, dedicated to a specific purpose.
This is the reason why the testsuite directory needs to be given for every invocation
(unless you'll place a default into your private `setup.ini`). Such a special purpose
test suite will have its own `defaults.ini`, its own `initial.state` and just the
test definition(s) you'll need.

Special case **Statistics**: by default, the Testrunner employs some heuristics as to when *not*
to compute general statistics. Especially when there are less than 5 tests with timing measurement,
danger is to create lopsided statistics. Global statistic calculation is skipped in those cases,
and thus it is also impossible to `--calibrate` the platform model. However, for "special investigations"
you might setup a dedicated separate test suite, and you might deliberately want to calculate statistics,
calibrate or rewrite baselines even when below error trigger limit. For these cases, you can overrule
those heuristics with the `--force` flag, and you can also tighten the warning bounds with `--strict`.

Pinpoint **Regressions** with `git-bisect`: To find the precise point in the Git history where a
performance regression was introduced, a dedicated Testsuite can be rigged. Add *two test cases*
to cover precisely the relevant feature with two quite distinct sample counts. Run this at least
10 times, then `--force --calibrate` a platform model. Since we're using exactly two points,
this model is a line through there two measurement points and thus the predicted runtime will
be exactly the average of the last test runs. You may then check this testsuite into a special
Git branch, and maybe even *manipulate* the tolerance value in the `*runtime.csv` files to suite
the special investigation. Running this testsuite can then be automated with a shell script
to be used as "oracle" for `git-bisect`.

**Debugging**: while it is straight forward to debug the test runner, watching Yoshimi's operations
can be tricky, since by default we launch into a forked process, and then use a background thread
to wait for feedback from the test subject. However, instead of Yoshimi itself, it is possible to
launch `gdbserver`, to be remote controlled via some port. You may thus connect `gdb` (or your IDE)
against this port and then single-step through the test execution within Yoshimi. Obviously, it
is recommended just to run a single test case in the suite, and also to increase the `cliTimeout`
(setting it e.g. to 3600, which gives you one hour for debugging).

Example configuration (`setup.ini` or `defaults.ini` in a separate testsuite directory)

    subject= /usr/bin/gdbserver
    arguments=  hostname:2345 /path/to/build/yoshimi.special --null --no-gui --cmdline

You'd then point `gdb` to the same executable (or another copy with debug informations, the actual
running executable does not need to have debug infos in this case). After GDB started, connect
to the server with

    target remote hostname:2345

Or use your favourite IDE (actually recommended for debugging) and create a suitable *launch configuration*
