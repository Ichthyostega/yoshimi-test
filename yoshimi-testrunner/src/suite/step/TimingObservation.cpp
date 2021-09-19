/*
 *  TimingObservation - extract timing information captured while performing the test run
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


/** @file TimingObservation.hpp
 ** Implementation details of a single timing measurement.
 ** After extracting the actual measurement value from the Yoshimi output,
 ** this raw data is postprocessed and combined with the expected expense factor
 ** and the effort predicted by the platform model for the given sample count of
 ** the actual test case. This data record is stored persistently into a time series
 ** for this test case, together with contextual information and the moving averages.
 ** 
 ** @todo WIP as of 9/21
 ** @see data.hpp maintaining CSV encoded time-series data
 ** 
 */


#include "util/data.hpp"
#include "util/file.hpp"
#include "util/nocopy.hpp"
#include "suite/step/TimingObservation.hpp"
#include "suite/Timings.hpp"
#include "Config.hpp"

#include <tuple>
//#include <string>
#include <iostream>////////////////TODO remove this
using std::cerr;
using std::endl;   ////////////////TODO remove this

namespace suite{
namespace step {

namespace {
    const size_t MILLISEC_per_NANOSEC = 1000*1000;
}


using util::Column;

/**
 * Data storage for runtime measurement time series.
 * @remark for sake of readability and to support external evaluations,
 *         we deliberately capture several contextual parameters alongside.
 */
struct TableRuntime
{
    Column<string>   timestamp{"Timestamp"};               ///< Timestamp of the Testsuite run
    Column<double>     runtime{"Runtime ms"};              ///< the actual timing measurement in milliseconds
    Column<size_t>     samples{"Samples count"};
    Column<uint>         notes{"Notes count"};
    Column<double>   plattform{"Plattform Factor"};        ///< runtime predicted by platform model
    Column<double>     expense{"Expense Factor"};          ///< baseline(expected value) for the expense
    Column<double> expenseCurr{"Expense Factor(current)"}; ///< `runtime == plattform·expenseCurr`
    Column<double>       delta{"Delta ms"};                ///< Δ of measured runtime against `plattform·expense`
    Column<double>        ma05{"Runtime MA-5"};            ///< moving average of runtime (last 5 measurements)
    Column<double>        ma10{"Runtime MA-10"};
    Column<double>        ma50{"Runtime MA-50"};

    auto allColumns()
    {   return std::tie(timestamp
                       ,runtime
                       ,samples
                       ,notes
                       ,plattform
                       ,expense
                       ,expenseCurr
                       ,delta
                       ,ma05
                       ,ma10
                       ,ma50
                       );
    }
};


/**
 * Data storage for the expected expense factor ("Baseline").
 * @remark Beyond the actual expense factor, a history of past settings
 *         is maintained, together with contextual information.
 */
struct TableExpense
{
    Column<string>   timestamp{"Timestamp"};               ///< Timestamp when setting this baseline
    Column<uint>        points{"Averaged points"};         ///> Number of past data points averaged into this baseline
    Column<double>     runtime{"Runtime(avg) ms"};         ///< averaged runtime used to define this baseline
    Column<size_t>     samples{"Samples count"};           ///< samples count of the underlying test
    Column<uint>         notes{"Notes count"};             ///< notes count of the underlying test
    Column<double>   plattform{"Plattform Factor"};        ///< runtime predicted by platform model for this baseline
    Column<double>     expense{"Expense Factor"};          ///< expected value for the expense. This is the *actual baseline*.

    auto allColumns()
    {   return std::tie(timestamp
                       ,points
                       ,runtime
                       ,samples
                       ,notes
                       ,plattform
                       ,expense
                       );
    }
};




/**
 * Extract relevant timing observations from captured behaviour.
 * Process the raw timing data into a time series, which can be
 * related to a _baseline value_ do derive a _current delta._
 */
class TimingTestData
    : public TimingTest
{
    using RuntimeData = util::DataFile<TableRuntime>;
    using ExpenseData = util::DataFile<TableExpense>;

    RuntimeData runtime_;
    ExpenseData expense_;


    /* === Interface: TimingTest === */


public:
    TimingTestData(fs::path fileRuntime, fs::path fileExpense)
        : runtime_{fileRuntime}
        , expense_{fileExpense}
    { }

    /**
     * build up one data record based on the current timing measurement.
     */
    void calculatePoint(uint notes, size_t smps, double rawTime)
    {
        runtime_.dupRow();
        runtime_.notes = notes;
        runtime_.samples = smps;
        runtime_.runtime = rawTime / MILLISEC_per_NANOSEC;
        /////////////////////////////////////////////////////TODO derive current expense and Δ
    }

    void persistRuntimes()
    {
        runtime_.save(); /////////////TODO limit number of past data retained ⟹ Config-Param "runtimeKeep"
    }
private:
};






// emit ctor/dtors here to keep the TimingTestData-PImpl private
TimingObservation::~TimingObservation() { }


TimingObservation::TimingObservation(Invocation& invocation
                                    ,OutputObservation& output
                                    ,suite::PTimings aggregator
                                    ,PathSetup& pathSetup)
    : theTest_{invocation}
    , pathSpec_{pathSetup}
    , output_{output}
    , globalTimings_{aggregator}
    , data_{}
{ }



void TimingObservation::calculateDataRecord()
{
    double runtime = *output_.getRuntime();
    uint   notes   = *output_.getNotesCnt();
    size_t smps    = *output_.getSamples();
    cerr << "+++ Runtime="<< runtime / (1000*1000) << " ms  ("<<notes<<"|"<<smps<<"smps)"<<endl; ///////////////////TODO: debugging code
    // individual timing measurement successfully observed

    FileNameSpec& fileRuntime = pathSpec_[def::KEY_fileRuntime];
    FileNameSpec& fileExpense = pathSpec_[def::KEY_fileExpense];

    data_.reset(new TimingTestData(fileRuntime, fileExpense));
    data_->calculatePoint(notes,smps,runtime);

    globalTimings_->attach(*data_);
}


void TimingObservation::persistTimeSeries()
{
    data_->persistRuntimes();
}



}}//(End)namespace suite::step
