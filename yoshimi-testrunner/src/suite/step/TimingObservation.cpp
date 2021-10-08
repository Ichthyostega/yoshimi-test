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
 ** @see data.hpp maintaining CSV encoded time-series data
 ** @see statistic.hpp
 ** @see suite::step::TimingJudgement
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
#include <vector>
#include <string>

namespace suite{
namespace step {

namespace {
    const size_t MILLISEC_per_NANOSEC = 1000*1000;
}

using std::min;
using util::isnil;
using util::backwards;
using util::averageLastN;
using util::computeTimeSeriesLinearRegression;

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
    Column<double> expenseCurr{"Expense Factor(current)"}; ///< `runtime == platform·expenseCurr`
    Column<double>       delta{"Delta ms"};                ///< Δ of measured runtime against `platform·expense`
    Column<double>      maTime{"MA Time short"};           ///< moving average of runtime (baselineAvg/2 points)
    Column<double>   tolerance{"Tolerance"};               ///< tolerance band based on 3·σ observed (over baselineAvg points)

    auto allColumns()
    {   return std::tie(timestamp
                       ,runtime
                       ,samples
                       ,notes
                       ,platform
                       ,expense
                       ,expenseCurr
                       ,delta
                       ,maTime
                       ,tolerance
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
        __requireMeasurementDone();
        avgPoints = ensureEquivalentDataPoints(avgPoints);
        return Point{double(runtime_.samples)
                    ,averageLastN(runtime_.runtime.data, avgPoints)
                    ,double(runtime_.expense)
                    };
    }

    Error getAveragedError(size_t avgPoints)  const override
    {
        __requireMeasurementDone();
        avgPoints = ensureEquivalentDataPoints(avgPoints);
        return std::make_tuple(averageLastN(runtime_.delta.data, avgPoints)
                              ,double{runtime_.tolerance});
    }

    void recalc_and_save_current(PlatformFun model) override
    {
        __requireMeasurementDone();
        recalcCurrentPoint(model(runtime_.notes, runtime_.samples));
        runtime_.save();
    }


public:
    TimingTestData(string testID, fs::path fileRuntime, fs::path fileExpense)
        : TimingTest{testID}
        , runtime_{fileRuntime}
        , expense_{fileExpense}
    { }

    bool hasBaseline()  const
    {   return not expense_.empty(); }

    void __requireMeasurementDone()  const
    {
        if (isnil(runtime_))
            throw error::LogicBroken("Attempt to extract test data prior to performing any measurements");
    }

    uint size()  const
    {
        return runtime_.size();
    }

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
    void calculatePoint(uint notes, size_t smps, double rawTime, double prediction, uint baselineAvg)
    {
        runtime_.dupRow();
        auto& r = runtime_;
        r.notes = notes;
        r.samples = smps;
        r.runtime = rawTime / MILLISEC_per_NANOSEC;

        r.platform = prediction / MILLISEC_per_NANOSEC; // all below in ms
        r.expense  = hasBaseline()? expense_.expense : 0.0;

        // apply the prediction model to factor out system dependency
        double expectedTime  = r.platform * r.expense;
        r.expenseCurr = 0.0 < prediction?   r.runtime / r.platform : 0.0;
        r.delta       = 0.0 < expectedTime? r.runtime - expectedTime : 0.0;

        // moving average used as reference to establish a tolerance band
        r.maTime = averageLastN(r.runtime.data, 5);
        r.tolerance = calcLocalTolerance(baselineAvg);

        // Timestamp of current Testsuite run
        r.timestamp = Config::timestamp;
    }

    /** adjust current runtime measurement to factor in a changed platform model */
    void recalcCurrentPoint(double prediction)
    {
        auto& r = runtime_;
        assert(0.0 < prediction);
        r.platform = prediction / MILLISEC_per_NANOSEC; // all below in ms
        r.expenseCurr = r.runtime / r.platform;
        double expectedTime  = r.platform * r.expense;
        r.delta = 0.0 < expectedTime? r.runtime - expectedTime : 0.0;
        r.maTime = averageLastN(r.runtime.data, 5);
    }

    void persistRuntimes(uint rows2keep)
    {
        runtime_.save(rows2keep);
    }

    void maybeStoreNewBaseline(uint baselineAvg, uint baselineKeep)
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
        if (isSignificantExpenseChange())
            expense_.save(baselineKeep);
    }

    array<double,4> getExpenseDeltaTolerance()  const
    {
        return {runtime_.runtime
               ,runtime_.expense
               ,runtime_.delta
               ,runtime_.tolerance};
    }

    array<double,3> calcDeltaTrend(uint n) const
    {
        return util::array_from_tuple(
                computeTimeSeriesLinearRegression(
                        util::lastN(runtime_.delta.data, n)));
    }

    /**
     * find timespan into the past without significant changes to the platform/environment.
     * @remark implemented by observing the runtime predicted by the platform model.
     * @return number of points while this prediction changed less than the local fluctuations
     */
    uint stablePlatformTimespan()  const
    {
        double anchor = runtime_.platform;     // current platform model prediction
        double tolerance = runtime_.tolerance; // local fluctuations
        auto platformData = backwards(runtime_.platform.data);
        uint points = 0;
        for (auto p = begin(platformData);
             p != end(platformData) and fabs(*p - anchor) <= tolerance;
             ++p
            )
            ++points;
        return points;
    }

private:
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

    /** determine the amplitude of local fluctuations */
    double calcLocalTolerance(size_t avgPoints) const
    {
        size_t siz = runtime_.size();
        assert (siz > 0);
        if (siz==1) // for the 1st value....
            return runtime_.delta; // just tolerate the actual delta

        avgPoints = std::min(avgPoints, siz);
        size_t oldest = siz - avgPoints;
        double variance = 0.0;
        for (size_t i=siz; oldest < i; --i)
        {   // use moving average of the /previous/ points as guess for "the actual" value
            double avgVal = i>1? runtime_.maTime.data[i-2] : runtime_.maTime.data[i-1];
            double delta = runtime_.runtime.data[i-1] - avgVal;
            variance += delta*delta;
        }
        variance /= avgPoints > 1? avgPoints-1 : 1;
        // divide by N-1 since it's a guess for the real variance
        return 3 * sqrt(variance);
    }

    /** determine if the current expense factor _differs significantly._
     * @return if using the previous expense would change the delta beyond
     *          1/3 of the current [local tolerance](\ref #calcLocalTolerance)
     */
    bool isSignificantExpenseChange()
    {
        size_t n = expense_.size();
        if (n < 2)
            return true; // significant if there is nothing to compare
        double newExpense = expense_.expense.data[n-1];
        double oldExpense = expense_.expense.data[n-2];
        // by definition: runtime(expected) = platform * expense
        double deltaChange = expense_.platform * (newExpense-oldExpense);
        return fabs(deltaChange) > runtime_.tolerance / 3;
    }
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

    double prediction = globalTimings_->evalPlatformModel(notes,smps);

    FileNameSpec& fileRuntime = pathSpec_[def::KEY_fileRuntime];
    FileNameSpec& fileExpense = pathSpec_[def::KEY_fileExpense];

    data_.reset(new TimingTestData(pathSpec_.getTestcaseID()
                                  ,fileRuntime,fileExpense));
    data_->calculatePoint(notes,smps,runtime,prediction
                         ,globalTimings_->baselineAvg);

    globalTimings_->attach(*data_);
}


void TimingObservation::saveData(bool includingBaseline)
{
    data_->persistRuntimes(globalTimings_->timingsKeep);
    if (includingBaseline)
        data_->maybeStoreNewBaseline(globalTimings_->baselineAvg
                               ,globalTimings_->baselineKeep);
}

uint TimingObservation::shortTermTimespan() const
{
    return min(data_->size(), globalTimings_->baselineAvg);
}

uint TimingObservation::longTermTimespan()  const
{
    return min(data_->stablePlatformTimespan()
              ,globalTimings_->longtermAvg);
}

/** @return current (runtime, expense, delta, tolerance) */
array<double,4> TimingObservation::getTestResults() const
{
    return data_->getExpenseDeltaTolerance();
}

/** linear regression over n delta values into the past
 * @return (socket,gradient,correlation) */
array<double,3> TimingObservation::calcDeltaTrend(uint n) const
{
    return data_->calcDeltaTrend(n);
}


}}//(End)namespace suite::step
