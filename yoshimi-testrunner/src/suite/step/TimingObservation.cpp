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
#include "util/utils.hpp"
#include "util/nocopy.hpp"
#include "util/statistic.hpp"
#include "suite/step/TimingObservation.hpp"
#include "suite/Timings.hpp"
#include "Config.hpp"

#include <tuple>
#include <string>

namespace suite{
namespace step {

namespace {
    const size_t MILLISEC_per_NANOSEC = 1000*1000;
}

using util::isnil;
using util::backwards;
using util::averageLastN;

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
    Column<double>    platform{"Platform ms"};             ///< runtime predicted by platform model (in ms)
    Column<double>     expense{"Expense Factor"};          ///< baseline(expected value) for the expense
    Column<double> expenseCurr{"Expense Factor(current)"}; ///< `runtime == plattform·expenseCurr`
    Column<double>       delta{"Delta ms"};                ///< Δ of measured runtime against `plattform·expense`
    Column<double>        ma05{"Expense MA-5"};            ///< moving average of expense factor (last 5 measurements)
    Column<double>        ma10{"Expense MA-10"};
    Column<double>        ma50{"Expense MA-50"};

    auto allColumns()
    {   return std::tie(timestamp
                       ,runtime
                       ,samples
                       ,notes
                       ,platform
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
    Column<double>    platform{"Platform ms"};             ///< runtime predicted by platform model for this baseline
    Column<double>     expense{"Expense Factor"};          ///< expected value for the expense. This is the *actual baseline*.

    auto allColumns()
    {   return std::tie(timestamp
                       ,points
                       ,runtime
                       ,samples
                       ,notes
                       ,platform
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

    Point getAveragedDataPoint(size_t avgPoints)  const override
    {
        if (isnil(runtime_))
            throw error::LogicBroken("Attempt to extract test data prior to performing any measurements");

        avgPoints = ensureEquivalentDataPoints(avgPoints);
        return Point{double(runtime_.samples)
                    ,averageLastN(runtime_.runtime.data, avgPoints)
                    ,double(runtime_.expense)
                    };
    }


public:
    TimingTestData(fs::path fileRuntime, fs::path fileExpense)
        : runtime_{fileRuntime}
        , expense_{fileExpense}
    { }

    bool hasBaseline()  const
    {   return not expense_.empty(); }


    /**
     * Build up one data record based on the current timing measurement.
     * @param prediction the runtime as _predicted by the platform model_
     * @remark the *platform model* is fitted with all timing measurements
     *         within the Testsuite and thus yields a simplified (linear)
     *         prediction based on the number of samples. Individual tests
     *         are more or less expensive, which is captured by the current
     *         _expense factor._ For each test case an averaged expense factor
     *         is stored as *baseline* -- and thus deviations can be detected.
     */
    void calculatePoint(uint notes, size_t smps, double rawTime, double prediction)
    {
        runtime_.dupRow();
        runtime_.notes = notes;
        runtime_.samples = smps;
        runtime_.runtime = rawTime / MILLISEC_per_NANOSEC;

        runtime_.platform = prediction / MILLISEC_per_NANOSEC;
        runtime_.expense  = hasBaseline()? expense_.expense : 0.0;

        // apply the prediction model to factor out system dependency
        double expectedTime  = prediction * runtime_.expense;
        runtime_.expenseCurr = 0.0 < prediction?   rawTime / prediction : 0.0;
        runtime_.delta       = 0.0 < expectedTime? rawTime - expectedTime : 0.0;
        runtime_.delta      /= MILLISEC_per_NANOSEC;

        // moving averages
        runtime_.ma05 = averageLastN(runtime_.expenseCurr.data, 5);
        runtime_.ma10 = averageLastN(runtime_.expenseCurr.data, 10);
        runtime_.ma50 = averageLastN(runtime_.expenseCurr.data, 50);

        // Timestamp of current Testsuite run
        runtime_.timestamp = Config::timestamp;
    }

    void persistRuntimes(uint rows2keep)
    {
        runtime_.save(rows2keep);
    }

    void storeNewBaseline(uint baselineAvg, uint baselineKeep)
    {
        expense_.dupRow();
        // record contextual info
        expense_.points = baselineAvg;
        expense_.samples  = runtime_.samples;
        expense_.notes    = runtime_.notes;
        expense_.platform = runtime_.platform;

        // define new baseline: average of the last timing measurements
        expense_.runtime = averageLastN(runtime_.runtime.data, baselineAvg);
        expense_.expense = expense_.runtime / expense_.platform;

        // Timestamp of creating this new baseline
        expense_.timestamp = Config::timestamp;
        expense_.save(baselineKeep);
    }

    /**
     * Find out how much comparable measurement points are available for averaging.
     * @remark "comparable" here implies an equivalent measurement setup
     *         - the number of samples was the same for all data points
     *         - the baseline expense factor was not changed/adjusted,
     *           which implies that the user judges the situation as stable
     */
    size_t ensureEquivalentDataPoints(size_t limit)  const
    {
        size_t refSamples = runtime_.samples;
        double refExpense = runtime_.expense;
        auto samplesData = backwards(runtime_.samples.data);
        auto expenseData = backwards(runtime_.expense.data);
        size_t maxPoints = runtime_.size();

        auto samples = begin(samplesData);
        auto expense = begin(expenseData);
        uint points = 0;
        while (points < limit and points < maxPoints
               and samples != end(samplesData)
               and expense != end(expenseData)
               and *samples == refSamples
               and *expense == refExpense)
        {
            ++points;
            ++samples;
            ++expense;
        }
        assert (0 < points and points <= limit and points <= maxPoints);
        return points;
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

    double prediction = globalTimings_->calcPlatformModel(notes,smps);

    FileNameSpec& fileRuntime = pathSpec_[def::KEY_fileRuntime];
    FileNameSpec& fileExpense = pathSpec_[def::KEY_fileExpense];

    data_.reset(new TimingTestData(fileRuntime, fileExpense));
    data_->calculatePoint(notes,smps,runtime,prediction);

    globalTimings_->attach(*data_);
}


void TimingObservation::saveData(bool includingBaseline)
{
    data_->persistRuntimes(globalTimings_->timingsKeep);
    if (includingBaseline)
        data_->storeNewBaseline(globalTimings_->baselineAvg
                               ,globalTimings_->baselineKeep);
}



}}//(End)namespace suite::step
