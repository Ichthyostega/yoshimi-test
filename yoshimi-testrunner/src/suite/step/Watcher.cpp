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
 ** @todo WIP as of 7/21
 ** @todo the LV2-plugin setup is future work as of 7/2021
 ** @see Scaffolding.hpp
 ** @see TestStep.hpp
 ** 
 */



#include "suite/step/Watcher.hpp"
#include "util/error.hpp"
#include "util/format.hpp"///////TODO
#include "util/filehandle.hpp"

#include <unistd.h>
#include <spawn.h>
#include <cassert>
#include <iostream>
#include <string>

using std::cerr;
using std::endl;
using util::formatVal;//////////TODO


/**
 * A null terminated list holding current process environment in `key=value` format.
 * @remark nonstandard but defined by all known compilers
 */
extern char** environ;



namespace suite{
namespace step {


SubProcHandle launchSubprocess(fs::path executable, ArgVect arguments)
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
    arguments.insert(arguments.begin(), exeName);  // setup 0th argument (the exec name)
    arguments.push_back(nullptr);                  // execve() requires a NULL terminated array
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





Watcher::Watcher(SubProcHandle chld, Progress& log)
    : child_{chld}
    , listener_{[this,&log]() { observeOutput(log); }}
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
void Watcher::observeOutput(Progress& log)
{
    util::IStreamFilehandle outputFromChild(child_.pipeChildOUT);

    log.out("==Listener=Thread==");
    for (string line; std::getline(outputFromChild, line); )
    {
        log.out(line); ///////////TODO logging shall be done by the MatchCond, and only on demand...
        matchTask.evaluate(line);
    }
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



}}//(End)namespace suite::step
