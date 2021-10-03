/*
 *  CleanUp - test step to discard intermediary storage
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


/** @file CleanUp.hpp
 ** Allow for explicit clean-up of intermediary resources after completion of
 ** a single test step. Especially after some low-level errors the subprocess
 ** might not have been discarded (or the LV2 plugin might still be loaded).
 ** Moreover, we hold the sound samples directly in memory during assessment
 ** of the test results; this data and any further test logs can be discarded,
 ** after the test results have been summarised.
 **
 ** @see Scaffolding.hpp
 ** @see util::SoundProbe
 ** @see Summary.hpp
 **
 */


#ifndef TESTRUNNER_SUITE_STEP_CLEAN_UP_HPP_
#define TESTRUNNER_SUITE_STEP_CLEAN_UP_HPP_


#include "suite/TestStep.hpp"
#include "suite/step/Scaffolding.hpp"
#include "suite/step/SoundObservation.hpp"
#include "suite/Progress.hpp"
#include "suite/Result.hpp"

#include <string>

namespace suite{
namespace step {

using std::string;


/**
 * Discard temporary resources after completing a test case.
 */
class CleanUp
    : public TestStep
{
    Scaffolding& scaffolding_;
    Progress& progressLog_;

    MaybeRef<SoundObservation> soundProbe_;


    Result perform()  override
    try {
        scaffolding_.cleanUp();
        progressLog_.clearLog();
        if (soundProbe_)
            soundProbe_->discardStorage();
        return Result::OK();
    }
    catch(std::exception& ex)
    {
        return Result::Warn("Failure in Clean-up: "+string{ex.what()});
    }
    catch(...)
    {
        return Result{ResCode::MALFUNCTION, "Unknown problem during resource clean-up."};
    }

public:
    CleanUp(Scaffolding& scaffolding
           ,MaybeRef<SoundObservation> sound
           ,Progress& progressLog)
        : scaffolding_{scaffolding}
        , progressLog_{progressLog}
        , soundProbe_{sound}
    { }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_CLEAN_UP_HPP_*/
