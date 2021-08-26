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
 ** @see step::ExeLauncher::triggerTest
 ** @see ExeCliMould::materialise
 **
 */



#include "Config.hpp"
#include "suite/step/PrepareScript.hpp"
#include "util/utils.hpp"

#include <string>
#include <regex>

using std::regex;
using std::smatch;
using std::string;
using std::regex_match;
using util::backwards;

namespace suite{
namespace step {

namespace {  // Regular Expressions for detecting relevant CLI commands...
    const regex PARSE_TEST_OUTPUT_SPEC{def::CLI_TEST_OUTPUT_PATTERN, regex::icase |regex::optimize};
    const regex PARSE_TEST_EXEC_TRIGGER{def::CLI_TEST_EXEC_PATTERN,  regex::icase |regex::optimize};

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


/**
 * Investigate / preprocess the script used to launch the test.
 * @remarks
 *  - for verifying the generated sound against a baseline,
 *    we need to know the name of the RAW file written by Yoshimi...
 *  - if the given test script explicitly gave a "`target <filename>`",
 *    we can pick out the required information by match.
 *  - otherwise, when output generation was not activated, while the
 *    test spec asks for `verifySound_`, a suitable CLI command has
 *    to be injected, right before the line triggering the test.
 *  - we need to handle the special cases where the script is packaged
 *    into a single line, e.g. `set test target <name> execute`
 */
Result PrepareTestScript::preprocess()
{
    if (not verifySound_) return Result::OK();

    smatch mat;
    DequeS& script = *this;
    for (string const& line : backwards(script))
        if (regex_match(line, mat, PARSE_TEST_OUTPUT_SPEC))
        {
            outFileSpec_ = mat[2];
            return Result::OK();
        }
    // Script defines no output file, but we need output to verify the sound...
   for (auto pos = begin(); pos != end(); ++pos)
       if (regex_match(*pos, mat, PARSE_TEST_EXEC_TRIGGER))
       {
           string outputSpec = mat[1].matched? def::CLI_ENTER_TEST_CONTEXT+" "+def::CLI_TEST_OUTPUT
                                             : def::CLI_DEFINITION        +" "+def::CLI_TEST_OUTPUT;
           outputSpec += " "+outFileSpec_;
           script.insert(pos, outputSpec); // insert missing output filename into the script
           return Result::OK();
       }

   return Result::Fail("Unable to find 'execute' trigger in test script.");
}



}}//(End)namespace suite::step
