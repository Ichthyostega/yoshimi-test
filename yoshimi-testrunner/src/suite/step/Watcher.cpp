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


/** @file Scaffolding.cpp
 ** Implementation details of launching a subprocess and controlling input/output.
 ** To launch a child process with connected input/output streams, we first need to create
 ** anonymous pipes; the POSIX `pipe()` system call returns two file handles for each pipe,
 ** a _read end_ and a _write end_. Since fork() duplicates the complete process setup, our
 ** child process will also hold those pipes open; each partner then needs to connect those
 ** ends of the pipes to use and close the other ends. The `posix_spawn()` function allows
 ** to define action commands for these setup steps, which are then performed within the
 ** child process prior to starting the target executable through the `execve()` call.
 ** Besides that, we _could_ (maybe future work?) also modify signal masks here.
 **
 ** @todo do we need to setup signal masking for the Child process?
 ** @see Scaffolding.hpp
 ** @see MatchTask.hpp
 ** @see TestStep.hpp
 **
 */



#include "suite/Result.hpp"
#include "suite/step/Watcher.hpp"
#include "util/filehandle.hpp"
#include "util/format.hpp"
#include "util/error.hpp"

#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <wait.h>
#include <cassert>
#include <iostream>
#include <string>

using std::cerr;
using std::endl;
using std::vector;
using std::future;
using std::promise;
using util::formatVal;


/**
 * A null terminated list holding current process environment in `key=value` format.
 * @remark nonstandard but defined by all known compilers
 */
extern char** environ;



namespace suite{
namespace step {


SubProcHandle launchSubprocess(fs::path executable, VectorS const& argSeq)
{
    enum PipeEnd{ READ=0, WRITE };

    SubProcHandle childHandle;
    int parent2child[2];
    int child2parent[2];

    int res;
#define ___MAYBE_FAIL(_MSG_) \
    if (res < 0) throw error::State("failed to " _MSG_)

    // Create pipes to connect with child process...
    res = pipe(parent2child);
    ___MAYBE_FAIL("create pipe parent->child");
    res = pipe(child2parent);
    ___MAYBE_FAIL("create pipe child->parent");

    // Define I/O connections to be wired within child *after* the fork()
    posix_spawn_file_actions_t actions_after_fork;
    res = posix_spawn_file_actions_init(&actions_after_fork);
    ___MAYBE_FAIL("init POSIX spawn-file-actions");

    res = posix_spawn_file_actions_adddup2 (&actions_after_fork, parent2child[PipeEnd::READ],   STDIN_FILENO);
    ___MAYBE_FAIL("prepare connection of child STDIN");
    res = posix_spawn_file_actions_adddup2 (&actions_after_fork, child2parent[PipeEnd::WRITE], STDOUT_FILENO);
    ___MAYBE_FAIL("prepare connection of child STDOUT");
    res = posix_spawn_file_actions_adddup2 (&actions_after_fork, child2parent[PipeEnd::WRITE], STDERR_FILENO);
    ___MAYBE_FAIL("prepare redirect child STDERR");

    // Child must close those pipe ends to be used by the parent
    res = posix_spawn_file_actions_addclose(&actions_after_fork, parent2child[PipeEnd::WRITE]);
    ___MAYBE_FAIL("prepare to close write end of parent->child");
    res = posix_spawn_file_actions_addclose(&actions_after_fork, child2parent[PipeEnd::READ]);
    ___MAYBE_FAIL("prepare to close read end of child->parent");

    // Setup further child attributes
    posix_spawnattr_t childAttribs;
    res = posix_spawnattr_init(&childAttribs);
    ___MAYBE_FAIL("init POSIX spawn-child-attibutes");
    //////////////TODO we could define some signal masks here

    // prepare program name and argument array...
    const char* const exePath = executable.c_str();
    const char* const exeName = executable.filename().c_str();
    vector<const char*> arguments{exeName}; // setup 0th argument (the exec name)
    for (auto& arg : argSeq)
        arguments.emplace_back(arg.c_str());
    arguments.push_back(nullptr);                 // execve() requires a NULL terminated array
    auto args = const_cast<char* const*>(arguments.data());

    // pass Environment of the test-runner unaltered into child process
    char* const * environment = environ;

    // Spawn the child process...
    res = posix_spawnp (&childHandle.pid, exePath, &actions_after_fork, &childAttribs, args, environment);
    ___MAYBE_FAIL("fork and spawn child process" + string{executable});


    // After child process is launched, parent must close the pipe ends to be used by the child
    res = close(parent2child[PipeEnd::READ]);
    ___MAYBE_FAIL("close read end of STDIN-pipe within parent");
    res = close(child2parent[PipeEnd::WRITE]);
    ___MAYBE_FAIL("close write end of STDOUT-pipe within parent");

    // and these are the pipe ends to retain for communication...
    childHandle.pipeChildIN  = parent2child[PipeEnd::WRITE];
    childHandle.pipeChildOUT = child2parent[PipeEnd::READ];
    assert(childHandle.pid > 0);
    return childHandle;
}





Watcher::Watcher(SubProcHandle chld)
    : child_{chld}
    , listener_{[this]() { observeOutput(); }}
{ }


Watcher::~Watcher() ///< @note blocks until the child output listener thread has terminated
try {
     if (listener_.joinable())
         listener_.join();
}
catch(std::exception const& ex) {
    cerr << "WARNING: failure while disposing Watcher thread: "<<ex.what()<< endl;
} catch(...) {
    cerr << "WARNING: unidentified problems in Watcher destructor."<< endl;
}


/**
 * Output listener thread: receive and watch output from child process
 */
void Watcher::observeOutput()
{
    util::IStreamFilehandle outputFromChild(child_.pipeChildOUT);

    for (string line; std::getline(outputFromChild, line); )
        matchTask.evaluate(line);
    matchTask.deactivate();
    awaitTermination();
}


/**
 * establish a way to retrieve the exit code of the child process.
 * @warning can be invoked only once (and this is what we need)
 */
future<int> Watcher::retrieveExitCode()
{
    return exitus_.get_future();
}


/** @internal use POSIX to wait blocking for the Child's death */
void Watcher::awaitTermination()
{
    int childStatus;
    pid_t pid = waitpid(child_.pid, &childStatus, 0);
    childStatus = WIFEXITED(childStatus)        ? WEXITSTATUS(childStatus)
                : WCOREDUMP(childStatus)        ? YOSHIMI_COREDUMP
                : WIFSIGNALED(childStatus) and
                  SIGSEGV==WTERMSIG(childStatus)? YOSHIMI_SEGFAULT
                :                                 YOSHIMI_CONFUSED;
    exitus_.set_value(childStatus);
    if (pid != child_.pid)
        throw error::LogicBroken("My child wasn't my child!?!");
}
/**
 * @todo placeholder code to terminate Yoshimi instead of launching a test
 */
void Watcher::TODO_forceQuit()
{
    util::OStreamFilehandle intputToChild(child_.pipeChildIN);

    usleep(2*1000*1000);
    intputToChild << "exit force"<<endl;
}


/** forcibly terminate the Yoshimi subprocess */
void Watcher::kill()
{
    int res = ::kill(child_.pid, SIGKILL);
    if (0 != res and ESRCH != res)
        throw error::State("Failed to kill the subject. PID="+formatVal(child_.pid)
                          +" Error-Code="+formatVal(res));
}



}}//(End)namespace suite::step
