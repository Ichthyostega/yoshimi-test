/*
 *  MatchTask - wait for match in output
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


/** @file MatchTask.hpp
 ** Allow for blocking wait until a condition matches on the subprocess output.
 ** THe subject under test is launched as a subprocess, reading back it's STDOUT from a
 ** separate watcher thread. The main thread, when conducting the test, needs to wait for
 ** some known marker text to appear in the lines of output read from the child process;
 ** a typical example would be the prompt from the Yoshimi CLI. One thread thus needs to
 ** provide match conditions, which are then evaluated on each line by the other thread,
 ** while the initiator goes into blocking wait.
 **
 ** # Wait and match Protocol
 **
 ** A blocking wait is implemented by a promise to future channel. The Watcher thread consumes
 ** lines of output, feeding each line for evaluation to a MatchTask component. Initially, this
 ** MatchTask is "empty", i.e. no check is performed. The main thread can build and enable an
 ** actual condition to match, which yields a future to block on. Safe hand-over of these actual
 ** conditions is coordinated by a atomic flag variable within the MatchTask component.
 ** - from the MatchTask, a builder is established to define the actual conditions
 ** - conditions are given as functor, referring to an output line string, returning a RegExp match.
 ** - a MatchCond state object is heap allocated with the main condition and possibly a precondition.
 ** - then the atomic flag is flipped, activating the match evaluation in the Watcher thread.
 ** - this causes each further line of output to be fed into the MatchCond instance for evaluation...
 ** - if a match is detected, the atomic flag is flipped to deactivate evaluation
 ** - and then the resulting `std::smatch` object is stored into the `promise`
 ** - which in turn will unblock the main thread waiting on the `future` end of the channel.
 ** 
 ** @todo WIP as of 8/21
 ** @see Watcher.hpp
 ** @see suite::step::ExeLauncher usage
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_MATCH_TASK_HPP_
#define TESTRUNNER_SUITE_STEP_MATCH_TASK_HPP_


#include "util/nocopy.hpp"
#include "suite/Progress.hpp"

#include <functional>
#include <optional>
#include <memory>
#include <future>
#include <atomic>
#include <thread>
#include <string>

namespace suite{
namespace step {

using std::string;
using std::move;

using MaybeLogger = std::optional<std::reference_wrapper<Progress>>;


/**
 * Combined conditions to be evaluated line by line on the output of the subprocess.
 */
class MatchCond
    : util::NonCopyable
{
public:
    using Matcher = std::function<bool(string const&)>;

    MatchCond(Matcher targetCond, Matcher precond, MaybeLogger logger)
        : primary_{targetCond}
        , precond_{precond}
        , logger_{logger}
    { }

    /** invoke the matching functor(s). */
    bool doCheck(string const& line);

private:
    Matcher primary_;
    Matcher precond_;
    bool fulfilledPrecond_ = false;
    MaybeLogger logger_;
};

/* ========= predefined matchers ================= */

extern const MatchCond::Matcher MATCH_YOSHIMI_READY;
extern const MatchCond::Matcher MATCH_YOSHIMI_PROMPT;




/**
 * A protocol to install and enable a MatchCond and then to block waiting
 * on that condition to be fulfilled by the ongoing output from the subprocess.
 * @remark #onCondition is the entry point for building a new matching condition;
 *         when terminating the build with MatchBuilder::activate(), this condition
 *         will be evaluated on each line and in case of a successful match
 *         the future will be unblocked.
 */
class MatchTask
    : util::NonCopyable
{
    std::atomic<bool> active_ = false;
    std::promise<void> promise_;
    std::unique_ptr<MatchCond> condition_;

    using Matcher = MatchCond::Matcher;

public:
    class MatchBuilder
        : util::MoveOnly
    {
        MatchTask& matchTask_;
        Matcher primary_;
        Matcher precond_;
        MaybeLogger logger_;

    public:
        MatchBuilder(MatchTask& managingTask, Matcher mainCondition)
            : matchTask_{managingTask}
            , primary_{mainCondition}
            , precond_{}
        { }

        MatchBuilder withPrecondition(Matcher preCond)
        {
            precond_ = move(preCond);
            return move(*this);
        }

        MatchBuilder logOutputInto(Progress& logger)
        {
            logger_ = logger;
            return move(*this);
        }

        /** terminal: establish and activate matching */
        std::future<void> activate();
    };
    friend std::future<void> MatchBuilder::activate();


    /** initiate the setup of a new active condition */
    MatchBuilder onCondition(Matcher primCond)
    {
        return MatchBuilder{*this, move(primCond)};
    }


    /** perform match (from Watcher thread) if this MatchTask is active */
    void evaluate(string const& outputLine);

    /** disable matching; mark as failure if active. */
    void deactivate();
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_MATCH_TASK_HPP_*/
