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
#include "util/file.hpp"
#include "util/data.hpp"
#include "util/error.hpp"
#include "util/format.hpp"
#include "util/statistic.hpp"
#include "util/utils.hpp"
#include "Timings.hpp"
#include "suite/step/PathSetup.hpp"

#include <functional>
#include <cassert>
#include <vector>
#include <tuple>

namespace suite {

namespace {
    const size_t MILLISEC_per_NANOSEC = 1000*1000;
}

using std::tie;
using std::make_tuple;

using step::FileNameSpec;

using util::isnil;
using util::Column;
using util::formatVal;
using util::backwards;



/**
 * Data storage for the local platform model (with history).
 * @remarks
 *  - the *platform model* is fitted by linear regression with all timing measurements within the Testsuite
 *    and thus yields a simplified (linear) prediction of the runtime of a test based on the number of samples.
 *  - when invoking the testsuite with argument `--calibrate`, a new fit is computed. A history of preview
 *    platform model fits is retained in the file `Suite-plattform.csv` within the testsuite root; this file
 *    is only valid for a given installation (machine, OS) and should not be checked into Git.
 *  - to support external evaluations, we deliberately capture several contextual parameters alongside.
 */
struct TablePlatform
{
    Column<string>   timestamp{"Timestamp"};               ///< Timestamp of the `--calibrate` Testsuite run
    Column<size_t>      points{"Data points"};             ///< number of fitted data points (=test cases)
    Column<double>      socket{"Socket ms"};               ///< constant base costs per testcase
    Column<double>       speed{"Speed ns/smp"};            ///< crunch speed (time per computed sample)
    Column<double> correlation{"Correlation"};             ///< observed correlation coefficient between (x,y)
    Column<double>    maxDelta{"Delta (max)"};             ///< maximum (absolute) Δ for this platform fit
    Column<double>   sdevDelta{"Delta (sdev)"};            ///< standard deviation √Σσ² for this platform fit

    auto allColumns()
    {   return std::tie(timestamp
                       ,points
                       ,socket
                       ,speed
                       ,correlation
                       ,maxDelta
                       ,sdevDelta
                       );
    }
};


/**
 * Data storage to capture global statistics for each run.
 * @remarks
 *  - the purpose of this time series is to detect global trends
 *  - while the TablePlattform is based on averaged run times,
 *    here we capture the fit and errors for the current measurements
 *  - thus "delta" tracks the deviation from the expected baseline value
 *  - the global trend is a linear regression on the average delta values.
 */
struct TableStatistic
{
    Column<string>   timestamp{"Timestamp"};               ///< Timestamp of the `--calibrate` Testsuite run
    Column<size_t>      points{"Data points"};             ///< number of fitted data points (=test cases)
    Column<double>      socket{"Socket ms"};               ///< constant base costs per testcase (in milliseconds)
    Column<double>       speed{"Speed ns/smp"};            ///< crunch speed (time per computed sample)
    Column<double>    avgDelta{"Delta (avg)"};             ///< averaged signed Δ against expected measurements
    Column<double>    maxDelta{"Delta (max)"};             ///< maximum (absolute) Δ against expected measurements
    Column<double>   sdevDelta{"Delta (sdev)"};            ///< standard deviation √Σσ² against expected measurements
    Column<double>   tolerance{"Tolerance"};               ///< tolerance band (3·σ) by error propagation from measurements

    auto allColumns()
    {   return std::tie(timestamp
                       ,points
                       ,socket
                       ,speed
                       ,avgDelta
                       ,maxDelta
                       ,sdevDelta
                       ,tolerance
                       );
    }
};


/**
 * Data storage to document the platform model fit calculated by linear regression.
 * This data is computed and stored only when performing `--calibrate`; it can be used
 * to manually investigate the quality of the model fit. It documents the normalised
 * run times used for fitting, together with the prediction produced by this model
 * and the delta. Each data point corresponds to a single test case with timing
 * measurement, averaging the last measurement points.
 */
struct TableModelFit
{
    Column<double>     samples{"Samples count"};     ///< Predictor variable (x value) : samples for this test case
    Column<double>     runtime{"Runtime ms"};        ///< averaged real runtime value for this test case
    Column<double>    timeNorm{"Runtime(norm)"};     ///< runtime normalised with the expense factor
    Column<double>  prediction{"Runtime(model)"};    ///< runtime as predicted by the computed model for the given samples count
    Column<double>     expense{"Expense Factor"};    ///< weight factor: baseline expense factor defined for this case
    Column<double>       delta{"Delta"};             ///< delta of the actual normalised runtime against model prediction
    Column<string>      testID{"Test-ID"};           ///< ID of the testcase underlying this timing measurement

    auto allColumns()
    {   return std::tie(samples
                       ,runtime
                       ,timeNorm
                       ,prediction
                       ,expense
                       ,delta
                       ,testID
                       );
    }
};



using VecD = std::vector<double>;
using TestTable = std::vector<std::reference_wrapper<TimingTest>>;
using PlatformData = util::DataFile<TablePlatform>;
using StatisticData = util::DataFile<TableStatistic>;

using util::RegressionData;
using util::RegressionPoint;

using ModelFit = util::DataFile<TableModelFit>;


/**
 * PImpl: data holder and implementation details
 * for the suite::Timings aggregator.
 */
class TimingData
    : util::NonCopyable
{
    TestTable      testData_;
    PlatformData   platform_;
    StatisticData  statistic_;
    ModelFit       modelFit_;

public:
    TimingData(fs::path filePlatform
              ,fs::path fileStatistic
              ,fs::path fileRegression
              )
        : testData_{}
        , platform_{filePlatform}
        , statistic_{fileStatistic}
        , modelFit_{fileRegression}
    {
        testData_.reserve(def::EXPECTED_TEST_CNT);
    }

    void attach(TimingTest& singleTestcaseData)
    {
        testData_.push_back(singleTestcaseData);
    }

    size_t dataCnt()  const
    {
        return testData_.size();
    }
    size_t timeSeriesSize()  const
    {
        return statistic_.size();
    }

    bool hasPlatformCalibration()  const
    {
        return not platform_.empty();
    }

    /**
     * @note using a simple linear model based on sample count only
     * @todo by means of a multi variable linear regression, we could
     *       factor in the typical NoteOn / NoteOff expenses.
     */
    double evalPlatformModel(uint, size_t smps)  const
    {
        return platform_.socket*MILLISEC_per_NANOSEC + smps * platform_.speed;
    }

    double getPlatformErrorSDev() const
    {
        return platform_.sdevDelta;
    }

    array<double,3> getDeltaStatistics()  const
    {
        return {statistic_.avgDelta
               ,statistic_.maxDelta
               ,statistic_.sdevDelta
               };
    }

    /**
     * Preprocess and condition the raw timing measurement data as preparation for model fit.
     * A DataPoint corresponds to a single test case, but averaged over a small number
     * of Testsuite runs [as configured](\ref Config::baselineAvg).
     * Moreover, the data point is normalised to the current _expense factor_ for this test case,
     * which means, the known specific expense for this situation is factored out, to focus on
     * modelling of the generic behaviour ("platform model"). However, we use this local
     * expense factor also as weight; deviations on very expensive computations will thus
     * count more for the linear regression used to build the platform model.
     */
    RegressionData preprocessRegressionData(uint avgPoints)
    {
        RegressionData data;
        data.reserve(testData_.size());
        for (TimingTest const& test : testData_)
        {
            auto [samples, runtime, expense] = test.getAveragedDataPoint(avgPoints);
            if (expense <= 0.0) expense = 1.0; // no baseline yet; use timing as-is, unweighted
            data.emplace_back(RegressionPoint{samples, runtime / expense, expense});
        }
        return data;
    }

    void buildPlatformModel(RegressionData points)
    {
        auto clearColumn = [size=points.size()](auto& col){
                               col.data.clear();
                               col.data.reserve(size);
                           };
        auto [socket, speed
             ,predictedPoints
             ,predictionDeltas
             ,correlation
             ,maxDelta
             ,sdevDelta]  = util::computeLinearRegression(points);

        // setup new platform model based on computed regression
        platform_.dupRow();
        platform_.socket = socket;                         // socket denoted in ms
        platform_.speed  = speed  * MILLISEC_per_NANOSEC;  // regression based on timings in ms
        platform_.correlation = correlation;
        platform_.maxDelta = maxDelta;
        platform_.sdevDelta = sdevDelta;

        // Mark new model with Timestamp of current Testsuite run
        platform_.timestamp = Config::timestamp;
        platform_.points = points.size();

        // capture data underlying the computed regression (for manual inspection)
        swap(modelFit_.prediction.data, predictedPoints);
        swap(modelFit_.delta.data,     predictionDeltas);
        clearColumn(modelFit_.samples);
        clearColumn(modelFit_.runtime);
        clearColumn(modelFit_.expense);
        clearColumn(modelFit_.timeNorm);
        clearColumn(modelFit_.testID);
        for (auto& p : points)
        {
            modelFit_.samples.data.push_back(p.x);
            modelFit_.expense.data.push_back(p.w);
            modelFit_.timeNorm.data.push_back(p.y);    // data for regression is normalised
            modelFit_.runtime.data.push_back(p.w*p.y); // reverse normalisation to get real data
        }
        for (TimingTest& test : testData_)
            modelFit_.testID.data.push_back(test.testID);
    }

    /**
     * capture current global timing statistics as a single time series data point.
     * @remark observing only actual delta against established baseline for each test.
     * @param avgPoints individual past measurements to pre-average for each test case data point
     * @return current delta averaged over all test cases
     */
    auto calcSuiteStatistics(uint avgPoints)
    {
        assert(not isnil(testData_));
        statistic_.dupRow();
        if (hasPlatformCalibration())
        {
            statistic_.socket = platform_.socket;
            statistic_.speed  = platform_.speed;
        }
        size_t n = statistic_.points = testData_.size();
        double avg=0.0, max=0.0, err=0.0;
        VecD deltas; deltas.reserve(n);
        for (TimingTest const& test : testData_)
        {
            auto [delta, tolerance] = test.getAveragedError(avgPoints);
            deltas.push_back(delta);
            avg += delta;
            err += tolerance*tolerance;    // error propagation; tolerance ~ 3·σ
            max = std::max(max, fabs(delta));
        }
        avg /= n;
        statistic_.avgDelta  = avg;
        statistic_.maxDelta  = max;
        statistic_.sdevDelta = util::sdev(deltas, avg);
        statistic_.tolerance = sqrt(err)/n;       // ~ 3·σ
        statistic_.timestamp = Config::timestamp; // current Testsuite run
        return make_tuple(avg, double{statistic_.tolerance});
    }

    /** calculate statistics over the past time series for the avgDelta */
    auto calcDeltaPastStatistics(uint avgPoints)
    {
        double movingAvg = util::averageLastN(statistic_.avgDelta.data, avgPoints);
        double pastSDev = util::sdevLastN(statistic_.avgDelta.data, avgPoints, movingAvg);
        return make_tuple(movingAvg, pastSDev);
    }

    /** calculate linear regression over the past time series ov avgDelta */
    auto calcDeltaTrend(uint avgPoints) const
    {
        return computeTimeSeriesLinearRegression(
                   util::lastN(statistic_.avgDelta.data, avgPoints));
    }

    /** find time span into the past without changes to the platform model */
    uint stablePlatformTimespan()  const
    {
        if (not hasPlatformCalibration())
            return timeSeriesSize();
        double anchor = platform_.speed;    // current platform model factor
        auto timeSeries = backwards(statistic_.speed.data);
        uint points = 0;
        for (auto p = begin(timeSeries);
             p != end(timeSeries) and *p == anchor;
             ++p
            )
            ++points;
        return points;
    }


    void save(bool includingCalibration, uint timingsKeep, uint calibrationKeep)
    {
        statistic_.save(timingsKeep);
        if (not includingCalibration) return;
        platform_.save(calibrationKeep);
        modelFit_.save();
        for (TimingTest& test : testData_)
            test.recalc_and_save_current([this](uint notes, size_t samples)
                                         { return evalPlatformModel(notes,samples); });
    }

    string sumariseCalibration()  const
    {
        return "socket=" +formatVal(double{platform_.socket})+"ms "
               "speed="  +formatVal(double{platform_.speed})+"ns/smp "
               "| corr: "+formatVal(double{platform_.correlation})+
               "  Δmax:" +formatVal(double{platform_.maxDelta})+"ms"
               " σ = "   +formatVal(double{platform_.sdevDelta})+"ms"
               ;
    }
};



// emit dtors here...
Timings::~Timings() { }

Timings::Timings(fs::path root
                ,uint keepT
                ,uint keepB
                ,uint baseline
                ,uint longterm)
    : data_{new TimingData{FileNameSpec(def::TIMING_SUITE_PLATFORM)
                            .enforceExt(def::EXT_DATA_CSV)
                          ,FileNameSpec(def::TIMING_SUITE_STATISTIC)
                            .enforceExt(def::EXT_DATA_CSV)
                          ,FileNameSpec(def::TIMING_SUITE_REGRESSION)
                            .enforceExt(def::EXT_DATA_CSV)
                          }}
    , suitePath{consolidated(root)}
    , timingsKeep{keepT}
    , baselineKeep{keepB}
    , baselineAvg{baseline}
    , longtermAvg{longterm}
{ }



/**
 * Prepare aggregator to collect and compute global timing statistics.
 */
PTimings Timings::setup(Config const& config)
{
    fs::path suiteRoot = consolidated(config.suitePath);
    fs::current_path(suiteRoot); // CWD to testsuite root

    return PTimings{new Timings(suiteRoot
                               ,config.timingsKeep
                               ,config.baselineKeep
                               ,config.baselineAvg
                               ,config.longtermAvg
                               )};
}


void Timings::attach(TimingTest& singleTestcaseData)
{
    data_->attach(singleTestcaseData);
}

void Timings::fitNewPlatformModel()
{
    data_->buildPlatformModel(
            data_->preprocessRegressionData(baselineAvg));
}

void Timings::calcSuiteStatistics()
{
    if (dataCnt() == 0)
        throw error::LogicBroken("No timing measurement performed yet.");

    tie(suite.currAvgDelta
       ,suite.tolerance) = data_->calcSuiteStatistics(baselineAvg);

    uint availData = data_->stablePlatformTimespan();
    suite.shortTerm = std::min(availData, baselineAvg);
    suite.longTerm  = std::min(availData, longtermAvg);

    tie(suite.pastDeltaAvg
       ,suite.pastDeltaSDev) = data_->calcDeltaPastStatistics(suite.shortTerm);
    tie(std::ignore // socket
       ,suite.gradientShortTerm
       ,suite.corrShortTerm) = data_->calcDeltaTrend(suite.shortTerm);
    tie(std::ignore // socket
       ,suite.gradientLongTerm
       ,suite.corrLongTerm)  = data_->calcDeltaTrend(suite.longTerm);
}


void Timings::saveData(bool includingCalibration)
{
    // tests have navigated down into the tree;
    // return to the Testsuite root prior to saving
    fs::current_path(suitePath);

    data_->save(includingCalibration, timingsKeep,baselineKeep);
}

string Timings::sumariseCalibration()  const
{
    return data_->sumariseCalibration();
}


double Timings::evalPlatformModel(uint notes, size_t smps)  const
{
    return isCalibrated()? data_->evalPlatformModel(notes,smps)
                         : 0.0;
}

/** @return stdev estimated by mean square error of model fitting */
double Timings::getModelTolerance() const
{                       // ±3σ covers 99% of all cases
    return isCalibrated()? 3 * data_->getPlatformErrorSDev()
                         : 0.0;
}

/** @return `(avgDelta,maxDelta,sdevDelta)` */
array<double,3> Timings::getDeltaStatistics()  const
{
    return data_->getDeltaStatistics();
}


size_t Timings::dataCnt()  const
{
    return data_->dataCnt();
}

bool Timings::isCalibrated()  const
{
    return data_->hasPlatformCalibration();
}



}//(End)namespace suite
