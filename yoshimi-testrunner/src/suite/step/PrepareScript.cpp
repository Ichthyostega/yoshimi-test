/*
 *  PrepareScript - provide a preprocessed CLI script
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


/** @file PrepareScript.cpp
 ** Implementation details of preprocessing Yoshimi CLI scripts.
 ** 
 ** @todo WIP as of 8/21
 ** @see step::ExeLauncher::triggerTest
 ** @see ExeCliMould::materialise
 ** 
 */



#include "Config.hpp"
#include "suite/step/PrepareScript.hpp"
//#include "util/format.hpp"
//#include "util/utils.hpp"
//#include "util/regex.hpp"

//#include <utility>
#include <string>

//using std::move;
//using std::regex;
//using std::smatch;
using std::string;
//using util::replace;
//using util::formatVal;

namespace suite{
namespace step {

namespace {// Implementation helpers
///////////////TODO
}//(End) helpers



// Emit VTables and dtors here....
Script::~Script() { }


/**
 * @return regular expression pattern matching a text mark,
 *         to be expected in Yoshimi output when the script
 *         has successfully completed execution.
 * @note default implementation expects the Yoshimi prompt;
 *         for this to work the script...
 *         - must end at top level scope
 *         - must send a spurious newline to provoke that prompt,
 *           since Yoshimi blocks directly after the prompt,
 *           waiting for user input.
 */
string PrepareScript::markWhenSriptIsFinished()  const
{
    return def::YOSHIMI_PROMPT_PATTERN;
}


/**
 * @return a text mark to be expected in Yoshimi as a precondition
 *         before the script will terminate, to make the match unique.
 * @remark the default implementation uses the last line of the script,
 *         since Yoshimi echoes all received commands; after that, the
 *         next prompt line appearing on the CLI means Yoshimi is ready.
 */
string PrepareScript::markWhenScriptIsComplete() const
{
    return ".+>\\s*" + lastLine();
}


/**
 * @note especially for the actual test launch script we cannot expect the Yoshimi prompt,
 *       since the built-in TestInvoker will shutdown Yoshimi right away. Rather,
 *       we'll wait for the "`TEST::Complete`" marker from the TestInvoker.
 */
string PrepareTestScript::markWhenSriptIsFinished() const
{
    return "TEST::Complete.+";
}



void PrepareTestScript::preprocess()
{
    //////TODO actually preprocess Test Script: look for output
}



}}//(End)namespace suite::step
