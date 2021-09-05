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
 ** @see Builder.hpp
 ** @see suite::step
 ** @see Stage::perform(Suite)
 ** @see ExeCliMould::materialise()
 **
 */


#ifndef TESTRUNNER_SUITE_TESTSTEP_HPP_
#define TESTRUNNER_SUITE_TESTSTEP_HPP_


#include "util/nocopy.hpp"
#include "suite/Result.hpp"

//#include <string>
#include <functional>
#include <optional>


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



/**
 * Reference to an _Optional Test Step._
 * @remarks allows to depend on another test step, which may or may not be defined.
 * @note    since C++ does not support covariance, a wildcard conversion operator to
 *          other optional test step types is provided, assuming the payload types are
 *          compatible; this allows to hold a `MaybeRef` of an interface and assign a
 *          MaybeRef of an concrete subtype.
 * @warning as always when dealing with _references,_ the referred-to object must be
 *          allocated and managed _elsewhere._ In the case of test steps, this is
 *          typically the step sequence defining the test suite itself.
 */
template<class STEP>
struct MaybeRef
    : std::optional<std::reference_wrapper<STEP>>
{
    using Wrapper = std::optional<std::reference_wrapper<STEP>>;
    using Wrapper::Wrapper;

    template<class X>
    operator MaybeRef<X>()
    {
        if (this->has_value())
            return MaybeRef<X>(this->value());
        else
            return std::nullopt;
    }

    STEP& operator*()  { return Wrapper::operator*().get(); }
    STEP* operator->() { return & Wrapper::operator*().get(); }
};



}//(End)namespace suite
#endif /*TESTRUNNER_SUITE_TESTSTEP_HPP_*/
