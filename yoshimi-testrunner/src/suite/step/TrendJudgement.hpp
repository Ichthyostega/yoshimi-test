/*
 *  TrendJudgement - assess the overall testsuite statistics and trend
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


/** @file TrendJudgement.hpp
 ** Investigate the overall Testsuite statistics and raise alarm when detecting a trend.
 ** The observed standard derivation of the average delta values allows to define a corridor
 ** of ordinary fluctuation of measurements. Whenever a (linear) trend line points outside
 ** of that corridor during a short period of time, an alarm will be raised.
 ** 
 ** @todo WIP as of 9/21
 ** @see TrendObservation.hpp
 ** @see TimingJudgement.hpp
 ** @see Timings.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_TREND_JUDGEMENT_HPP_
#define TESTRUNNER_SUITE_STEP_TREND_JUDGEMENT_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"

//#include <string>

namespace suite{
namespace step {


/**
 * Step to assess the behaviour and decide about success or failure.
 */
class TrendJudgement
    : public TestStep
{

    Result perform()  override
    {
        return Result::Warn("UNIMPLEMENTED: TrendJudgment (global)");
    }

public:
    TrendJudgement()
    { }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_TREND_JUDGEMENT_HPP_*/
