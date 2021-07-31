/*
 *  Scaffolding - prepare the subject for launching a test case
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


/** @file Scaffolding.cpp
 ** Implementation details of invoking Yoshimi and gathering result data.
 ** - launching as a subprocess requires to fork and connect STDIN / STDOUT
 ** - for the LV2 plugin setup we need to implement a minimalist LV2 host
 ** 
 ** @todo WIP as of 7/21
 ** @todo the LV2-plugin setup is future work as of 7/2021
 ** @see Scaffolding.hpp
 ** @see TestStep.hpp
 ** 
 */



#include "suite/step/Scaffolding.hpp"
#include "suite/step/Watcher.hpp"
#include "util/format.hpp"

//#include <string>

using util::formatVal;

namespace suite{
namespace step {


// Emit VTables and dtors here....
Scaffolding::~Scaffolding() { }
ExeLauncher::~ExeLauncher() { }


/** */
ExeLauncher::ExeLauncher(fs::path testSubject
                        ,fs::path topicPath
                        ,Progress& progress)
    : subject_{testSubject}
    , topicPath_{topicPath}
    , progressLog_{progress}
{ }


Result ExeLauncher::perform()
{
    progressLog_.indicateTest(topicPath_);
    if (not fs::exists(subject_))
        return Result{ResCode::MALFUNCTION, "Executable not found: "+formatVal(subject_)};

    progressLog_.indicateOutput("ExeLaucher: start Yoshimi subprocess...");
    subprocess_.reset(new Watcher(launchSubprocess(subject_)));
    return Result::OK();
}


int ExeLauncher::triggerTest()
{
    progressLog_.indicateOutput("TODO: trigger test in Yoshimi...");
    return int(ResCode::DEBACLE);
}


}}//(End)namespace suite::step