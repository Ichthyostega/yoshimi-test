/*
 *  Stage - statefull environment used once for running the Testsuite
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


/** @file Stage.hpp
 ** Execution environment for performing the Testsuite.
 ** The testsuite is assembled by a TestBuilder, based on the test case definitions,
 ** resulting in a sequence of TestStep elements. These can then be invoked one by one
 ** on the \ref Stage, which is a one-way statefull environment, allowing to log any
 ** failures and to collect results.
 ** 
 ** @todo WIP as of 7/21
 ** @see Main.cpp usage
 ** @see Suite
 ** 
 */


#ifndef TESTRUNNER_STAGE_HPP_
#define TESTRUNNER_STAGE_HPP_


#include "util/nocopy.hpp"
#include "suite/TestLog.hpp"
#include "util/tee.hpp"
#include "Config.hpp"
#include "Suite.hpp"

//#include <string>
#include <memory>



namespace suite {
    class Report;

    using PReport = std::unique_ptr<Report>;
}


/**
 * Execution environment to perform a test suite once.
 */
class Stage
    : util::NonCopyable
{
    suite::TestLog results_;
    suite::PReport report_;

public:
    Stage(Config const& config);
   ~Stage();

    void perform(Suite& suite);
    void renderReport();
    suite::ResCode getReturnCode()  const;
};


#endif /*TESTRUNNER_STAGE_HPP_*/
