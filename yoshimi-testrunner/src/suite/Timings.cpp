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
#include "util/statistic.hpp"
//#include "util/utils.hpp"
#include "Timings.hpp"
#include "suite/step/PathSetup.hpp"

#include <functional>
#include <vector>

namespace suite {

using step::FileNameSpec;

using util::Column;


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
    Column<double>      socket{"Socket ns"};               ///< constant base costs per testcase
    Column<double>       speed{"Speed ns/smp"};            ///< crunch speed (time per computed sample)
    Column<double>    maxDelta{"Delta (max)"};             ///< maximum (absolute) Δ for this platform fit
    Column<double>   sdevDelta{"Delta (sdev)"};            ///< standard deviation √σ² for this platform fit

    auto allColumns()
    {   return std::tie(timestamp
                       ,points
                       ,socket
                       ,speed
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
    Column<double>      socket{"Socket ns"};               ///< constant base costs per testcase
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



using TestTable = std::vector<std::reference_wrapper<TimingTest>>;
using PlatformData = util::DataFile<TablePlatform>;
using StatisticData = util::DataFile<TableStatistic>;


/**
 * PImpl: data holder and implementation details
 * for the suite::Timings aggregator.
 */
class TimingData
    : util::NonCopyable
{
    TestTable     testData_;
    PlatformData  plattform_;
    StatisticData statistic_;

public:
    TimingData(fs::path filePlatform
              ,fs::path fileStatistic
              )
        : testData_{}
        , plattform_{filePlatform}
        , statistic_{fileStatistic}
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
        return not plattform_.empty();
    }

    /**
     * @note using a simple linear model based on sample count only
     * @todo by means of a multi variable linear regression, we could
     *       factor in the typical NoteOn / NoteOff expenses.
     */
    double calcPlatformModel(uint, size_t smps)  const
    {
        return plattform_.socket + smps * plattform_.speed;
    }
};



// emit dtors here...
Timings::~Timings() { }

Timings::Timings()
    : data_{new TimingData{FileNameSpec(def::TIMING_SUITE_PLATFORM)
                            .enforceExt(def::EXT_DATA_CSV)
                          ,FileNameSpec(def::TIMING_SUITE_STATISTIC)
                            .enforceExt(def::EXT_DATA_CSV)
                          }}
{ }



/**
 * Prepare aggregator to collect and compute global timing statistics.
 */
PTimings Timings::setup(Config const& config)
{
    // CWD to the Testsuite root
    fs::current_path(config.suitePath);

    return PTimings{new Timings};
}


void Timings::attach(TimingTest& singleTestcaseData)
{
    data_->attach(singleTestcaseData);
}

double Timings::calcPlatformModel(uint notes, size_t smps)  const
{
    if (data_->hasPlatformCalibration())
        return data_->calcPlatformModel(notes,smps);
    else
        return 0.0;
}

uint Timings::dataCnt()  const
{
    return data_->dataCnt();
}



}//(End)namespace suite
