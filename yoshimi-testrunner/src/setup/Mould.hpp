/*
 *  Mould - setup and wiring patterns for building test cases
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


/** @file Mould.hpp
 ** Setup and wiring of a generic test case.
 ** The testsuite is assembled by a TestBuilder, which picks a suitable mould for each
 ** individual test case, to establish a graph of properly wired suite::TestStep components.
 ** The resulting network of steps is then integrated and performed as a Testsuite.
 ** 
 ** @todo WIP as of 7/21
 ** @see Builder.hpp usage
 ** @see TestStep.hpp
 ** @see \ref Suite
 ** 
 */


#ifndef TESTRUNNER_SETUP_MOULD_HPP_
#define TESTRUNNER_SETUP_MOULD_HPP_


#include "util/nocopy.hpp"
#include "setup/Builder.hpp"
#include "suite/Progress.hpp"
#include "suite/Timings.hpp"

#include <functional>
#include <memory>
#include <string>


namespace setup {

using std::string;
using std::shared_ptr;
using std::reference_wrapper;

using suite::PTimings;
using suite::Progress;
using RProgress = std::reference_wrapper<Progress>;



/**
 * Framework and definition pattern for building a test case.
 */
class Mould
    : util::NonCopyable
{
protected:
    StepSeq   steps_;
    RProgress progressLog_{Progress::null()};
    PTimings  suiteTimings_;
    bool shallRecordBaseline_{false};

public:
    virtual ~Mould();  ///< this is an interface

    Mould& withProgress(Progress& logger)
    {
        progressLog_ = logger;
        return *this;
    }

    Mould& withTimings(PTimings timingDataHolder)
    {
        suiteTimings_ = timingDataHolder;
        return *this;
    }
    Mould& recordBaseline(bool indeed)
    {
        shallRecordBaseline_ = indeed;
        return *this;
    }

    /** prepare this Mould for the next generation cycle */
    virtual Mould& startCycle();

    /** terminal builder operation: trigger generation. */
    StepSeq generateStps(MapS const& spec)
    {
        materialise(spec);
        StepSeq generatedSteps;
        swap(generatedSteps, this->steps_);
        return generatedSteps;
    }


private:
    /**
     * Extension Point: build actual test steps,
     * according to the »blueprint« represented by this Mould
     * @param spec specific definition of a single test case.
     * @remark after invocation, #steps_ holds the planned actions.
     */
    virtual void materialise(MapS const& spec)  =0;
};



/** Entry point for the Builder: pick suitable Mould for testcase */
Mould& useMould_for(string testTypeID);


}//(End)namespace setup
#endif /*TESTRUNNER_SETUP_MOULD_HPP_*/
