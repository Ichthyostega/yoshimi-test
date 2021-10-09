/*
 *  PlatformCalibration - re-fit the platform timing model to current measurements
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


/** @file PlatformCalibration.hpp
 ** Establish a platform model of timing behaviour by linear regression.
 ** Verifying a given test's runtime measurements against a known baseline is
 ** challenging, since individual timings tend to fluctuate --- and they depend
 ** on the runtime environment. By introducing a _platform model,_ a generic factor
 ** to describe the platform and environment can be factored out of the data; what
 ** remains, should be reasonably conditioned and generic to be compared and judged.
 **
 ** # Model fit
 ** For the Yoshimi-testsuite we use a rather simplistic platform model, which can
 ** be fitted by *linear regression* with the actual timing data. To perform this
 ** _(re)-Calibration (activated through the commandline argument `--calibrate`),
 ** this dedicated TesttStep is injected at the end of the testsuite sequence.
 ** All individual tests (which are already completed at this point) have recorded
 ** a time series of timing measurement data, collected during this and the preceding
 ** Testsuite runs. Moreover, all these individual _data points_ are attached to a
 ** global aggregator, the suite::Timings. However, since the goal is to use this
 ** platform model to separate a generic from a specific effect within the observed
 ** data, this data needs to be preprocessed, to normalise out the specific local
 ** timing effects -- as far as they are known at this point. The data points after
 ** this normalisation steps can then be fitted linearly by means of statistics.
 ** 
 ** @todo WIP as of 9/21
 ** @see Timings.hpp
 ** @see TimingObservation.hpp
 ** @see TimingJudgement.hpp
 ** @see Mould.cpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_PLATFORM_CALIBRATION_HPP_
#define TESTRUNNER_SUITE_STEP_PLATFORM_CALIBRATION_HPP_


#include "util/nocopy.hpp"
#include "util/format.hpp"
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
 * Re-generate a linear regression on the collected timing data,
 * to be used as _platform model_ subsequently. Prior to the actual
 * fitting operation, the collected data must be normalised to focus
 * on the generic timing behaviour and level out the average local
 * _expense factor_ for each individual test case.
 */
class PlatformCalibration
    : public TestStep
{
    Progress& progressLog_;
    PTimings  timings_;


    Result perform()  override
    {
        if (not timings_->isCalibrated())
            progressLog_.note("Calibration: +++ establish new Platform Model +++");
        else
            progressLog_.note("Calibration: +++ re-fit Platform Model to current data +++");
        progressLog_.out("Calibration: preparing "+str(timings_->dataCnt())+" data points...");
        timings_->fitNewPlatformModel();
        progressLog_.note("Calibration: "+timings_->sumariseCalibration());
        return Result::OK();
    }

public:
    PlatformCalibration(Progress& log
                       ,PTimings aggregator
                       )
        : progressLog_{log}
        , timings_{aggregator}
    { }


private:
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_PLATFORM_CALIBRATION_HPP_*/
