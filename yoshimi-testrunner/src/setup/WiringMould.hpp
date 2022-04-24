/*
 *  WiringMould - helper/builder implementation for wiring steps within a Mould
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


/** @file WiringMould.hpp
 ** Implementation details of a builder to support wiring of test steps.
 ** @internal this is an _implementation header_ and shall only be included into mould.cpp
 **
 ** @remark The actual concrete Mould implementation has to provide a definition of the
 **         Mould::materialise() function, where the actual setup of the test steps is
 **         expected to happen. To simplify the setup and wiring of those steps, we provide
 **         a builder syntax, which creates and manages the TestStep instances by hidden
 **         side effect; the dependencies and collaboration between steps is established
 **         by direct references. However, some steps shall only be wired optionally; we
 **         can express this by wrapping into std::optional -- which can be encapsulated
 **         into a helper-builder \ref WiringMould::Conditional. This setup allows to
 **         wire optional steps _guarded by a condition_ -- typical example is the
 **         verification of sound, which happens only when enabled in the test spec.
 **
 ** @see Mould.cpp usage
 */


#ifndef TESTRUNNER_SETUP_WIRING_MOULD_HPP_
#define TESTRUNNER_SETUP_WIRING_MOULD_HPP_


#include "setup/Mould.hpp"
#include "suite/TestStep.hpp"

#include <memory>


namespace setup {

using suite::MaybeRef;


/**
 * Implementation mix-in to provide a step builder DSL
 * for wiring test steps within the concrete Mould implementation.
 */
class WiringMould
    : public Mould
{
    struct Conditional;

protected:
    /* == builder interface for the concrete Moulds == */
    template<class STEP, typename...ARGS>
    STEP& addStep(ARGS&& ...args);

    Conditional optionally(bool condition);
};



/** @internal helper builder to allow for optional/conditional steps */
struct WiringMould::Conditional
{
    bool  case_enabled;
    WiringMould& mould;

    template<class STEP, typename...ARGS>
    MaybeRef<STEP> addStep(ARGS&& ...args);
};

/** builder syntax to start defining an optional step */
inline WiringMould::Conditional WiringMould::optionally(bool condition)
{
    return { condition, *this };
}


/**
 * Build and add a concrete suite::TestStep subclass,
 * passing arguments (for Dependency Injection).
 * @return _reference_ to the new step, with concrete type.
 * @remark typically you'd store that reference locally and
 *         pass it as arguments to following steps (DI)
 */
template<class STEP, typename...ARGS>
inline STEP& WiringMould::addStep(ARGS&& ...args)
{
    auto step = std::make_unique<STEP>(std::forward<ARGS>(args)...);
    STEP& ref = *step;
    steps_.emplace_back(std::move(step));
    return ref;
}

/**
 * Conditionally build and add a concrete suite::TestStep.
 * @return a reference wrapped into a std::optional
 * @remark allows to express the dependency on an optional step,
 *         i.e. a step which may or may not be present in the test definition.
 */
template<class STEP, typename...ARGS>
inline MaybeRef<STEP> WiringMould::Conditional::addStep(ARGS&& ...args)
{
    return case_enabled? MaybeRef<STEP>{mould.addStep<STEP>(std::forward<ARGS>(args)...)}
                       : std::nullopt;
}


}//(End)namespace setup
#endif /*TESTRUNNER_SETUP_WIRING_MOULD_HPP_*/
