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


/** @file Timings.hpp
 ** Statistics context to collect timing data during Testsuite execution.
 ** Running individual tests typically also yields some timing data, which unfortunately
 ** is prone to fluctuations and depends on the execution environment. More robust findings
 ** can be distilled by combining several of these local measurements, yet such a process
 ** involves global aggregation and necessitates a (re-)calibration of platform speed
 ** factors -- supported by this global data holder used throughout the suite execution;
 ** actually, global statistics calculation can refer directly to the individual data
 ** points observed in the testcases, because each local TimingObservation
 ** hooks itself into this global Timings aggregator.
 ** 
 ** @todo WIP as of 9/21
 ** @see [setup and wiring](\ref setup::build(Config const&))
 ** @see TimingObservation.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_TIMINGS_HPP_
#define TESTRUNNER_SUITE_TIMINGS_HPP_


#include "Config.hpp"
#include "util/nocopy.hpp"

#include <functional>
#include <string>
#include <memory>
#include <array>


namespace suite {

class TimingData;
using PData = std::unique_ptr<TimingData>;

class Timings;
using PTimings = std::shared_ptr<Timings>;

using std::array;


/**
 * Interface: a single case of Timing measurement.
 */
class TimingTest
    : util::NonCopyable
{

protected:
    TimingTest(string testID)
        : testID{testID}
    { }

public:
    virtual ~TimingTest() { }  ///< this is an interface

    const string testID;

    /// Abstracted Data point: (samples,runtime,expense)
    using Point = std::tuple<double,double,double>;
    using PlatformFun = std::function<double(uint,size_t)>;

    virtual Point getAveragedDataPoint(size_t avgPoints)  const =0;
    virtual double getAveragedDelta(size_t avgPoints)     const =0;
    virtual void recalc_and_save_current(PlatformFun)           =0;
};



/**
 * Aggregated timing data for the complete Testsuite.
 * Provides methods to derive global statistics trends.
 */
class Timings
    : util::NonCopyable
{
    PData data_;

    Timings(fs::path, uint,uint,uint,uint);
public:
   ~Timings();
    static PTimings setup(Config const&);

    void attach(TimingTest&);

    double evalPlatformModel(uint notes, size_t smps)  const;
    void fitNewPlatformModel();
    void saveData(bool includingCalibration);

    size_t dataCnt()  const;
    bool isCalibrated()  const;
    string sumariseCalibration() const;
    double getModelTolerance() const;

    void calcSuiteStatistics();
    array<double,3> getDeltaStatistics()  const;

    struct SuiteStatistics
    {
        double currAvgDelta{0.0};
        double pastDeltaAvg{0.0};
        double pastDeltaSDev{0.0};

        uint shortTerm{0};
        uint longTerm{0};

        double gradientShortTerm{0.0};
        double corrShortTerm{0.0};
        double gradientLongTerm{0.0};
        double corrLongTerm{0.0};
    };
    SuiteStatistics suite;

    /* config params */
    const fs::path suitePath;
    const uint timingsKeep;   ///< number of timing data points to retain in the time series
    const uint baselineKeep;  ///< number of past baseline definitions to retain
    const uint baselineAvg;   ///< number of past measurements to average for baseline decisions
    const uint longtermAvg;   ///< number of past measurements to average for long term trends
};




}//(End)namespace suite
#endif /*TESTRUNNER_SUITE_TIMINGS_HPP_*/
