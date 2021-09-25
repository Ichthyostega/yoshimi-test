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

//#include <string>
#include <memory>
#include <tuple>


namespace suite {

class TimingData;
using PData = std::unique_ptr<TimingData>;

class Timings;
using PTimings = std::shared_ptr<Timings>;



/**
 * Interface: a single case of Timing measurement.
 */
class TimingTest
    : util::NonCopyable
{

protected:
    TimingTest() { }
public:
    virtual ~TimingTest() { }  ///< this is an interface

    /// Abstracted Data point: (samples,runtime,expense)
    using Point = std::tuple<double,double,double>;

    virtual Point getAveragedDataPoint(size_t avgPoints)  const  =0;
};



/**
 * Aggregated timing data for the complete Testsuite.
 * Provides methods to derive global statistics trends.
 */
class Timings
    : util::NonCopyable
{
    PData data_;

    Timings(fs::path, uint,uint,uint);
public:
   ~Timings();
    static PTimings setup(Config const&);

    void attach(TimingTest&);

    double calcPlatformModel(uint notes, size_t smps)  const;
    void fitNewPlatformModel();
    void saveData(bool includingCalibration);

    uint dataCnt()  const;
    bool isCalibrated()  const;
    string sumariseCalibration() const;

    /* config params */
    const fs::path suitePath;
    const uint timingsKeep;   ///< number of timing data points to retain in the time series
    const uint baselineAvg;   ///< number of past measurements to average when defining new baseline
    const uint baselineKeep;  ///< number of past baseline definitions to retain
};




}//(End)namespace suite
#endif /*TESTRUNNER_SUITE_TIMINGS_HPP_*/
