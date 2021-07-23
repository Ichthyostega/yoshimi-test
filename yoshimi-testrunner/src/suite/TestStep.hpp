/*
 *  TestStep - basic building block of the Testsuite
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


/** @file TestStep
 ** Interface of the fundamental Testsuite building block.
 ** The testsuite is a sequence of test steps, assembled and wired by a TestBuilder.
 ** Performing the testsuite equates to triggering each test step, capturing results.
 ** 
 ** @todo WIP as of 7/21
 ** @see Builder.hpp
 ** @see suite::step
 ** @see Stage::perform(Suite)
 ** 
 */


#ifndef TESTRUNNER_SUITE_TESTSTEP_HPP_
#define TESTRUNNER_SUITE_TESTSTEP_HPP_


#include "util/nocopy.hpp"
#include "suite/Result.hpp"

//#include <string>

namespace suite {


/**
 * Interface: elementary testsuite building block.
 */
class TestStep
    : util::NonCopyable
{
public:
    virtual ~TestStep() { }  ///< this is an interface

    virtual Result perform()  =0;
};


}//(End)namespace suite
#endif /*TESTRUNNER_SUITE_TESTSTEP_HPP_*/
