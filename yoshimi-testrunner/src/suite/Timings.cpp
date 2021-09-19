/*
 *  Timings - test timing statistics and detection of global trends
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


/** @file Timings.cpp
 ** Implementation details of global statistics calculation.
 ** 
 ** @todo WIP as of 9/21
 **
 */


#include "Config.hpp"
#include "util/error.hpp"
#include "util/statistic.hpp"
//#include "util/utils.hpp"
#include "Timings.hpp"

#include <functional>
#include <vector>

namespace suite {

using TestTable = std::vector<std::reference_wrapper<TimingTest>>;


/**
 * PImpl: data holder and implementation details
 * for the suite::Timings aggregator.
 */
class TimingData
    : util::NonCopyable
{
    TestTable testData_;

public:
    TimingData()
        : testData_{}
    {
        testData_.reserve(def::EXPECTED_TEST_CNT);
    }

    void attach(TimingTest& singleTestcaseData)
    {
        testData_.push_back(singleTestcaseData);
    }
    uint dataCnt()
    {
        return testData_.size();
    }
};



// emit dtors here...
Timings::~Timings() { }

Timings::Timings()
    : data_{new TimingData}
{ }


/**
 * Setup the stage for performing a concrete test suite.
 * @param config parametrisation to control some aspects of the test run.
 */
PTimings Timings::setup(Config const& config)
{
    return PTimings{new Timings};
}


void Timings::attach(TimingTest& singleTestcaseData)
{
    data_->attach(singleTestcaseData);
}

uint Timings::dataCnt()
{
    return data_->dataCnt();
}



}//(End)namespace suite
