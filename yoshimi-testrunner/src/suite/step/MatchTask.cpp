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


/** @file MatchTasks.cpp
 ** Implementation details of matching on the output lines (from the subprocess).
 ** @see ExeLauncher::triggerTest()
 **
 */



#include "Config.hpp"
#include "suite/step/MatchTask.hpp"
#include "util/error.hpp"

#include <regex>
#include <memory>
#include <cassert>
#include <exception>

using std::regex;
using std::smatch;
using std::make_unique;
using std::make_exception_ptr;
using std::future;
using std::promise;
using std::atomic;


namespace suite{
namespace step {

using Matcher = MatchCond::Matcher;

namespace { // text matching implementation details

    const regex YOSHIMI_SUCCESFULL_START{def::YOSHIMI_SUCCESFULL_START_PATTERN};
    const regex YOSHIMI_PROMPT_PATTERN  {def::YOSHIMI_PROMPT_PATTERN};


    Matcher buildMatcher_for(regex const& match2find)
    {
        return [&](string const& line)
                {
                    return regex_match(line, match2find);
                };
    }

}//(End) implementation details


const Matcher MATCH_YOSHIMI_READY  = buildMatcher_for(YOSHIMI_SUCCESFULL_START);
const Matcher MATCH_YOSHIMI_PROMPT = buildMatcher_for(YOSHIMI_PROMPT_PATTERN);




namespace { // handling the atomic flag...

    inline void enable(atomic<bool>& flag)
    {
        bool expectFalse{false};
        if (not flag.compare_exchange_strong(expectFalse, true, std::memory_order_acq_rel))
            throw error::LogicBroken{"Attempt to define a new MatchCond while "
                                     "an existing condition is still evaluated."};
    }

    inline bool disable(atomic<bool>& flag)
    {
        bool expectTrue{true};
        return flag.compare_exchange_strong(expectTrue, false, std::memory_order_acq_rel);
    }

    inline bool isEnabled(atomic<bool> const& flag)
    {
        return flag.load(std::memory_order_acquire);
    }
}//(End) impl details atomic flag




/**
 * @remark To activate matching on the conditions defined thus far within this builder,
 *         we construct a new MatchCond object to hold the state and a new promise to
 *         communicate the result on successful match. As final step, the atomic flag
 *         MatchTask::active_ is flipped; we use `std::memory_order_acq_rel`, which
 *         ensures a reliable hand-over of the new MatchCond. Quoting from the spec:
 *         "all memory writes (non-atomic and relaxed atomic) that _happened-before_
 *         the atomic store from the point of view of thread A, become _visible
 *         side-effects_ in thread B. That is, once the atomic load is completed,
 *         thread B is guaranteed to see everything thread A wrote to memory".
 * @return a future connected to the new promise; the Watcher thread will invoke
 *         MatchTask::evaluate() on each line of output, and fed a successful
 *         match into this promise to unblock the future.
 */
future<void> MatchTask::MatchBuilder::activate()
{
    matchTask_.condition_ = make_unique<MatchCond>(primary_,precond_,logger_);
    promise<void> newPromise;
    std::swap(matchTask_.promise_, newPromise);
    enable(matchTask_.active_);
    return matchTask_.promise_.get_future();
}



void MatchTask::evaluate(string const& outputLine)
{
    if (not isEnabled(active_)) return;
    assert(condition_);
    if (condition_->doCheck(outputLine))
    {   // condition fulfilled
        disable(active_);
        promise_.set_value(); // => signal successful match
    }
}


/**
 * @remark implements the actual matching logic; applied to each line of output:
 *   - if a precondition was given, attempt to fulfil the precondition first
 *   - then attempt to fulfil the main condition
 * @return the RegEx match performed on the given line,
 *     will be `empty()` when the match was not successful.
 */
bool MatchCond::doCheck(string const& line)
{
    if (logger_) logger_->get().out(line);
    if (precond_ and not fulfilledPrecond_)
    {
        fulfilledPrecond_ = precond_(line);
        if (not fulfilledPrecond_)
            return false; // bail out
    }
    return primary_(line);
}


/**
 * @warning very simplistic implementation; danger of races and exceptions from the
 *          promise and future when invoked concurrently to MatchTask::evaluate().
 * @remark  since only the Watcher thread invokes MatchTask::evaluate() from within
 *          the output retrieving loop, it is safe to invoke that function from
 *          the same thread _after the output has ended_ and prior to termination.
 */
void MatchTask::deactivate()
{
    if (disable(active_))
        promise_.set_exception(
            make_exception_ptr(error::FailedLaunch("Subject died while still expecting some output")));
}



}}//(End)namespace suite::step
