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
//#include "util/utils.hpp"
#include "Timings.hpp"
#include "suite/step/PathSetup.hpp"

#include <functional>
#include <vector>

namespace suite {

namespace {
    const size_t MILLISEC_per_NANOSEC = 1000*1000;
}

using step::FileNameSpec;

using util::Column;
using util::formatVal;



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
    Column<double>   sdevDelta{"Delta (sdev)"};            ///< standard deviation √σ² for this platform fit

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
    Column<double>   sdevDelta{"Delta (sdev)"};            ///< standard deviation √σ² against expected measurements

    auto allColumns()
    {   return std::tie(timestamp
                       ,points
                       ,socket
                       ,speed
                       ,avgDelta
                       ,maxDelta
                       ,sdevDelta
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

    uint dataCnt()  const
    {
        return testData_.size();
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
    double calcPlatformModel(uint, size_t smps)  const
    {
        return platform_.socket*MILLISEC_per_NANOSEC + smps * platform_.speed;
    }

    tuple<double,size_t> getModelTolerance() const
    {
        return {platform_.sdevDelta, platform_.points};
    }


    /**
     * Condition the raw timing measurement data as preparation for model fit.
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
        platform_.socket = socket;                         // socked denoted in ms
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

    void save(bool includingCalibration, uint timingsKeep, uint calibrationKeep)
    {
        statistic_.save(timingsKeep);
        if (not includingCalibration) return;
        platform_.save(calibrationKeep);
        modelFit_.save();
        for (TimingTest& test : testData_)
            test.recalc_and_save_current([this](uint notes, size_t samples)
                                         { return calcPlatformModel(notes,samples); });
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


double Timings::calcPlatformModel(uint notes, size_t smps)  const
{
    if (isCalibrated())
        return data_->calcPlatformModel(notes,smps);
    else
        return 0.0;
}

tuple<double,size_t> Timings::getModelTolerance() const
{
    return data_->getModelTolerance();
}

uint Timings::dataCnt()  const
{
    return data_->dataCnt();
}

bool Timings::isCalibrated()  const
{
    return data_->hasPlatformCalibration();
}



}//(End)namespace suite
