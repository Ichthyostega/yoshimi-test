/*
 *  Watcher - to oversee output and termination of a subprocess
 *
 *  Copyright 2021, Hermann Vosseler <Ichthyostega@web.de>
 *
 *  This file is part of the Yoshimi-Testsuite, which is free software:
 *  you can redistribute and/or modify it under the terms of the GNU
 *  General Public License as published by the Free Software Foundation,
 *  either version 3 of the License, or (at your option) any later version.
 *
 *  Yoshimi-Testsuite is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with yoshimi.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************/


/** @file Watcher.hpp
 ** Maintain the input/output pipes to a subprocess, and wait for defined events.
 ** Some of the Yoshimi acceptance test use a setup, where the subject under test is
 ** launched as a subprocess. After successfully starting, the setup- and test scripts
 ** are fed into the STDIN of this process to reach the Yoshimi CLI; at the same time the
 ** STDOUT and STDERR of the test subject must be captured to observe the progress of testing.
 **
 ** # Technology
 **
 ** We use `posix_spawn()` to start a child process; this setup is equivalent to the well known
 ** sequence of `fork()` to create a clone of the current process (the yoshimi-testrunner) as
 ** child process, sharing all open files and pipes with the parent, then followed by a call to
 ** the `exec*()` family of POSIX functions, which replace the address space of the child with
 ** the executable launched thereby. The result is a child process running the desired executable
 ** under a known PID, and a set of _pipes_ connected to the subject's STDIN, STDOUT and STDERR.
 **
 ** The \ref Watcher component encapsulates the setup for supervision; a dedicated thread is
 ** spawned to receive and evaluate the output channels, possibly to kill the subject and
 ** to reap the exit value. The main thread, which performs the test suite, can tap into
 ** this supervision by blocking on Futures, to await expected stages of the test to
 ** be reached with a timeout as safeguard.
 ** 
 ** @todo WIP as of 7/21
 ** @see Schaffolding.hpp
 ** @see suite::step::ExeLauncher usage
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_WATCHER_HPP_
#define TESTRUNNER_SUITE_STEP_WATCHER_HPP_


#include "util/file.hpp"
#include "util/nocopy.hpp"
#include "util/filehandle.hpp"
#include "suite/Progress.hpp"
#include "suite/step/MatchTask.hpp"

#include <future>
#include <thread>
#include <vector>
#include <string>

namespace suite{
namespace step {

using std::move;


struct SubProcHandle
{
    int pid;
    int pipeChildIN;
    int pipeChildOUT;
};

using VectorS = std::vector<std::string>;

/**
 * Launch a subprocess and connect it's input/output pipes.
 * @arg executable a complete path to the executable to launch
 * @arg arguments a vector with the actual arguments to pass;
 *      the 0th argument (=filename) will be injected automatically.
 * @remark the implementation is based on [posix_spawn()]
 *
 * [posix_spawn()]: https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawn.html
 */
SubProcHandle launchSubprocess(fs::path executable, VectorS arguments);



/**
 * Adapter to oversee the input/output streams connected to a
 * subprocess, with the ability to wait for expected events.
 */
class Watcher
    : util::NonCopyable
{
    const SubProcHandle child_;
    std::promise<int> exitus_;
    std::thread listener_;

    util::OStreamFilehandle outputToChild_;

public:
    MatchTask matchTask;

    Watcher(SubProcHandle chld);
   ~Watcher();

    void kill();
    std::future<int> retrieveExitCode();
    void send2child(string);
    void TODO_forceQuit();
private:
    void observeOutput();
    void awaitTermination();
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_WATCHER_HPP_*/
