/*
 *  PersistModelTrend - test step to store global statistics and model data persistently
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


/** @file PersistModelTrend.hpp
 ** Write global trend and model tables to storage, after calculation
 ** of suite statistics and possibly re-calibration of the platform model.
 ** 
 ** @todo WIP as of 9/21
 ** @see Timings.hpp
 ** @see TrendObservation.hpp
 ** @see PlatformCalibration.hpp
 ** @see Mould.cpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_PERSIST_MODEL_TREND_HPP_
#define TESTRUNNER_SUITE_STEP_PERSIST_MODEL_TREND_HPP_


#include "util/nocopy.hpp"
//#include "util/format.hpp"
#include "suite/TestStep.hpp"
//#include "suite/step/Invocation.hpp"
//#include "suite/step/OutputObservation.hpp"
//#include "suite/step/PathSetup.hpp"
#include "suite/Timings.hpp"
#include "Config.hpp"

//#include <memory>
//#include <string>
#include <iostream>////////////////TODO remove this
using std::cerr;
using std::endl;   ////////////////TODO remove this

namespace suite{
namespace step {

using util::str;



/**
 * Trigger saving of global statistics trend data,
 * and possibly also a newly calibrated platform model.
 */
class PersistModelTrend
    : public TestStep
{
    PTimings  timings_;
    bool calibrationMode_;


    Result perform()  override
    try {
        if (0 == timings_->dataCnt())
            return Result::Warn("No Timing observed; nothing to persist.");
        timings_->saveData(calibrationMode_);
        return Result::OK();

    }
    catch(error::State& writeFailure)
    {
        return Result{ResCode::MALFUNCTION,
                      string{"Unable to save global model and trends -- "}
                            + writeFailure.what()};
    }
    catch(fs::filesystem_error& fsErr)
    {
        return Result{ResCode::MALFUNCTION, fsErr.what()};
    }

public:
    PersistModelTrend(PTimings aggregator, bool cal)
        : timings_{aggregator}
        , calibrationMode_{cal}
    { }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_PERSIST_MODEL_TREND_HPP_*/
