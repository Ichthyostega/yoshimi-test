/*
 *  Script - a preprocessed CLI script
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


/** @file Script.hpp
 ** Interface to represent a Yoshimi CLI script, possibly preprocessed.
 ** The \ref ExeLauncher, which is a specific Scaffolding for invoking
 ** the test launcher build into Yoshimi, manages a Yoshimi instance
 ** running as subprocess. A Script instance can be _invoked_ there.
 ** After launching a script, the testrunner typically needs to wait for
 ** Yoshimi to complete a task; especially loading some instruments can take
 ** a long time. The tokens to expect in the output obviously depend on the
 ** actual script -- usually we can wait for the last line of the script
 ** to be echoed in Yoshimi's output channel.
 ** 
 ** @todo WIP as of 8/21
 ** @see Scaffolding.hpp
 ** @see PrepareScript.hpp
 ** @see step::PrepareTestScript::perform
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_SCRIPT_HPP_
#define TESTRUNNER_SUITE_STEP_SCRIPT_HPP_


#include "util/error.hpp"
#include "util/utils.hpp"
//#include "util/format.hpp"
//#include "suite/Result.hpp"
//#include "suite/TestStep.hpp"
//#include "suite/step/Scaffolding.hpp"

#include <iostream>
#include <utility>
#include <string>
#include <deque>
//#include <iostream>////////TODO rly?
//using std::cerr;
//using std::endl;

using std::move;
using std::string;
using util::isnil;


namespace suite{
namespace step {

using dequeS = std::deque<string>;

namespace {//Implementation helper

    inline dequeS splitLines(string code)
    {
        dequeS lines;
        auto ss = std::stringstream{code};
        for (string line; std::getline(ss, line); )
            lines.push_back(line);
        return lines;
    }
}//(End)Implementation




/**
 * Interface: a CLI script for Yoshimi...
 * - provide the (preprocessed) script code
 * - get a marker to expect in Yoshimi output after execution
 */
class Script
    : protected dequeS
{
protected:
    virtual ~Script();   ///< this is an interface

    Script(string const& rawCode)
        : dequeS{move(splitLines(rawCode))}
    { }


    void __checkNonempty()  const
    {
        if (this->empty())
            throw error::State{"Empty CLI script."};
    }

    string const& lastLine()  const
    {
        __checkNonempty();
        return back();
    }

public:
    // Script can be iterated...
    using iterator = dequeS::const_iterator;
    iterator begin() const { return dequeS::begin(); }
    iterator end()   const { return dequeS::end();   }

    virtual string markWhenSriptIsFinished()  const =0;
    virtual string markWhenScriptIsComplete() const =0;
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_SCRIPT_HPP_*/
