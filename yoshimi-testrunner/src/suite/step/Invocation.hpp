/*
 *  Invocation  - actually launch the test operations
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


/** @file Invocation .hpp
 ** Launch the test subject (Yoshimi) and perform the test operations.
 ** This central TestStep uses the specific Scaffolding established for the test case
 ** to activate the actual sound calculation within Yoshimi. After completion of the
 ** actual test run, the Invocation step instance serves as information hub for the
 ** following evaluation, judgement and reporting steps.
 ** @see Scaffolding.hpp
 ** @see Conclusion.hpp
 ** @see TestStep.hpp
 **
 */


#ifndef TESTRUNNER_SUITE_STEP_INVOCATION_HPP_
#define TESTRUNNER_SUITE_STEP_INVOCATION_HPP_


#include "util/nocopy.hpp"
#include "util/format.hpp"
#include "suite/Result.hpp"
#include "suite/TestStep.hpp"
#include "suite/step/Scaffolding.hpp"

namespace suite{
namespace step {


/**
 * Launch Yoshimi and execute the test sound calculation.
 */
class Invocation
    : public TestStep
{
    Scaffolding& scaffolding_;
    bool performed_ = false;


    Result perform()  override
    {
        return scaffolding_.maybe("launchTest",
        [&] {
            Result res = scaffolding_.triggerTest();
            performed_ = (res.code != ResCode::MALFUNCTION);
            return res;
            });
    }

public:
    Invocation(Scaffolding& scaf)
        : scaffolding_{scaf}
    { }

    bool isPerformed()  const
    {
        return performed_
           and not scaffolding_.isBroken();
    }

    int getSampleRate()  const
    {
        return 48000; /////////////////////TODO get that from a setup step
    }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_INVOCATION_HPP_*/
