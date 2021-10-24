/*
 *  sound - sound file handling, reading and writing
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


/** @file sound.cpp
 ** Implementation of sound data handling, based on libSoundfile.
 **
 ** \par Sound data format
 ** All calculations are done in 32bit floating point, which also happens to be
 ** the internal data format used by Yoshimi. The sound probe calculated by the
 ** TestInvoker in Yoshimi is dumped into a RAW soundfile, with stereo channels
 ** interleaved. To read this probe data, we also need to know the sample rate
 ** configured when launching Yoshimi. Any sound files generated for persistent
 ** storage however are written in WAV format (RIFF, little endian with floats).
 **
 ** \par Measurements
 ** The comparison to the _baseline waveform_ is done by subtraction; a successful
 ** test invocation reproduces the baseline exactly, and thus the _residual_ is
 ** all zero. But when there are differences, we store the residual sound for
 ** further investigation, and we calculate a [RMS] to judge the energy level
 ** of the difference. However, since the concern is about _audibility_ of defects,
 ** we look for the maximum RMS obtained over a short integration window of 30ms.
 **
 ** \par Numerics
 ** Calculations done here are simplistic; the RMS window is unweighted (rectangular),
 ** meaning that for each data point we add a new value and we drop out a value at
 ** the end of the window, thereby combining data from both stereo channels. Numeric
 ** stability can be an issue for such accumulating calculations -- yet the precision
 ** of doubles is sufficient; summing one hour of sound at 96kHz yields numbers ~10^9,
 ** and 1 + 10^9 can easily be represented as exact double value.
 **
 */


#include "util/sound.hpp"
#include "util/error.hpp"
#include "util/format.hpp"

#include <sndfile.hh>
#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
#include <cmath>


namespace util {

using util::formatVal;
using std::string;
using std::move;
using std::log10;
using std::fabs;
using std::max;
using std::min;

using SampleVec = std::vector<float>;



struct SoundStat       ///< @internal raw aggregation results
{
    uint   rate;
    size_t frames;
    float  peak;
    double rmsAll;     ///< @note: actually squares (σ²)
    double rmsMax;
};

namespace { // Implementation details

    const double RMS_WINDOW_sec = double{30}/1000;
    const uint CHANNELS = 2; // Yoshimi TestInvoker always generates Stereo sound

    inline int validate(uint sampleRate)
    {
        if (0 < sampleRate and sampleRate < 1e6)
            return int(sampleRate);
        else
            throw error::State("Possibly invalid sample rate "+str(sampleRate));
    }


    /** construct a libSndfile Handle for reading a RAW file */
    SndfileHandle openSndfileRead(fs::path rawSound, uint sampleRate)
    {
        if (not hasExtRAW(rawSound))
            throw error::LogicBroken("Expecting a RAW soundfile written by Yoshimi.");
        if (not fs::exists(rawSound))
            throw error::LogicBroken("Could not find expected RAW soundfile \""+rawSound.string()+"\"");

        auto sndFormat = SF_FORMAT_RAW | SF_FORMAT_FLOAT;
        SndfileHandle sndFile{rawSound, SFM_READ, sndFormat, CHANNELS, validate(sampleRate)};
        if (not sndFile)
            throw error::State("Buffer allocation error while opening \""+rawSound.filename().string()+"\"");
        if (sndFile.error())
            throw error::State("Failed to open \""+rawSound.filename().string()+"\" for reading: '"
                              +sndFile.strError()+"'.");
        if (0 == sndFile.frames())
            throw error::State("Empty soundfile \""+rawSound.filename().string()+"\"");
        return sndFile;
    }


    /** construct a libSndfile Handle for reading a WAV file */
    SndfileHandle openSndfileRead(fs::path wavFile)
    {
        if (not hasExtWAV(wavFile))
            throw error::LogicBroken("Expecting a WAV soundfile.");
        if (not fs::exists(wavFile))
            throw error::LogicBroken("Could not find expected WAV soundfile \""+wavFile.string()+"\"");

        auto sndFormat = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
        SndfileHandle sndFile{wavFile, SFM_READ, sndFormat};
        if (not sndFile)
            throw error::State("Buffer allocation error while opening \""+wavFile.filename().string()+"\"");
        if (sndFile.error())
            throw error::State("Failed to open \""+wavFile.filename().string()+"\" for reading: '"
                              +sndFile.strError()+"'.");
        if (0 == sndFile.frames())
            throw error::State("Empty soundfile \""+wavFile.filename().string()+"\"");
        return sndFile;
    }


    /** construct a libSndfile Handle for writing a WAV file */
    SndfileHandle openSndfileWrite(fs::path target, uint sampleRate)
    {
        if (not hasExtWAV(target))
            throw error::LogicBroken("Expecting WAV file extension for writing \""+string{target}+"\".");

        auto sndFormat = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
        SndfileHandle sndFile{target, SFM_WRITE, sndFormat, CHANNELS, validate(sampleRate)};
        if (not sndFile)
            throw error::State("Buffer allocation error while creating \""+target.filename().string()+"\"");
        if (sndFile.error())
            throw error::State("Failed to open \""+target.filename().string()+"\" for writing: '"
                              +sndFile.strError()+"'.");
        return sndFile;
    }


    SampleVec readSoundData(SndfileHandle& src)
    {
        assert(src.channels() == CHANNELS);
        size_t sampleCnt = src.frames() * CHANNELS;
        SampleVec buffer(sampleCnt);
        size_t actuallyRead = src.read(buffer.data(), sampleCnt);
        if (actuallyRead != sampleCnt)
            throw error::State("Could not read the expected "+formatVal(sampleCnt)
                              +" samples from the soundfile; got only "+formatVal(actuallyRead));
        return buffer;
    }


    void writeSoundData(SampleVec const& samples, SndfileHandle dest)
    {
        assert(dest.channels() == CHANNELS);
        size_t actuallyWritten = dest.write(samples.data(), samples.size());
        if (actuallyWritten != samples.size())
            throw error::State("Could not write all "+formatVal(samples.size())
                              +" samples, only "+formatVal(actuallyWritten));
    }


    SampleVec buildDiff(SampleVec const& probe, SndfileHandle& baseline)
    {
        SampleVec buffer(move(readSoundData(baseline)));
        size_t diffSiz = min(probe.size(), buffer.size());
        for (size_t i=0; i < diffSiz; ++i)
            buffer[i] = probe[i] - buffer[i];
        for (size_t i=diffSiz; i < buffer.size(); ++i)
            buffer[i] = 0.0f;
        return buffer;
    }


    SoundStat calculateStats(SampleVec const& samples, uint smpPerSec)
    {
        size_t window = RMS_WINDOW_sec * smpPerSec * CHANNELS;

        auto sqr  = [](double val){ return val*val; };
        auto curr = [&](size_t i) { return samples[i]; };
        auto drop = [&](size_t i) { return i < window? 0.0 : samples[i-window]; };

        double movingAvg{0.0};
        SoundStat res{0, 0, 0.0, 0.0, 0.0};
        for (size_t i=0; i < samples.size(); ++i)
        {
            res.rmsAll += sqr(curr(i));
            movingAvg  += sqr(curr(i)) - sqr(drop(i));
            res.rmsMax  = max(res.rmsMax, movingAvg);
            res.peak    = max(res.peak, fabs(curr(i)));
        }
        res.rate    = smpPerSec;
        res.frames  = samples.size() / CHANNELS;
        res.rmsAll /= samples.size();
        res.rmsMax /= min(window, samples.size());
        return res;
    }
}//(End)Implementation namespace




/**
 * @internal PImpl to hold sound data buffer and statistics
 */
struct SoundData
    : util::NonCopyable
{
    SampleVec buffer;
    SoundStat stat;

    /** sound data from file */
    SoundData(SndfileHandle src)
        : buffer{move(readSoundData(src))}
        , stat{calculateStats(buffer, src.samplerate())}
    { }

    /** build sound data as diff between #probe and #baseline */
    SoundData(SoundData const& probe, SndfileHandle baseline)
        : buffer{move(buildDiff(probe.buffer, baseline))}
        , stat{calculateStats(buffer, baseline.samplerate())}
    { }
};
using PSoundData = std::unique_ptr<SoundData>;




SoundProbe::~SoundProbe() { } // emit dtors here...
SoundProbe::SoundProbe() { }


void SoundProbe::discardStorage()
{
    residual_.reset();
    probe_.reset();
}


void SoundProbe::loadProbe(fs::path rawSound, int sampleRate)
{
    probe_.reset(new SoundData{openSndfileRead(rawSound, sampleRate)});
    if (residual_) residual_.reset();
}


/** load the baseline file and then calculate the residual sound. */
void SoundProbe::buildDiff(fs::path baseline)
{
    if (not probe_)
        throw error::LogicBroken("Need to load a sound probe first.");
    SndfileHandle baselineFile = openSndfileRead(baseline);
    residual_.reset(new SoundData{*probe_, baselineFile});
}


/**
 * Basic sanity check after building a soundfile diff.
 * @return error message indicating a sanity check violation,
 *         or `std::nullopt` if everything looks valid.
 * @remark does not _judge_ the diff to assess test failure.
 */
OptString SoundProbe::checkDiffSane()  const
{
    if (not residual_)
        return OptString{"No Diff constructed"};
    if (0 == probe_->stat.frames)
        return OptString{"Empty sound probe"};
    if (0.0 == probe_->stat.peak)
        return OptString{"Mute sound probe"};
    if (probe_->stat.rate != residual_->stat.rate)
        return OptString{"Samplerate mismatch."
                         " Probe: "   +formatVal(probe_->stat.rate)
                        +" Baseline: "+formatVal(residual_->stat.rate)};
    if (probe_->stat.frames > residual_->stat.frames)
        return OptString{"Probe exceeds baseline by "
                        +formatVal(probe_->stat.frames - residual_->stat.frames)
                        +" samples ("
                        +formatVal(1000.0*(probe_->stat.frames - residual_->stat.frames) / probe_->stat.rate)
                        +"msec)"};
    if (probe_->stat.frames < residual_->stat.frames)
        return OptString{"Baseline exceeds probe by "
                        +formatVal(residual_->stat.frames - probe_->stat.frames)
                        +" samples ("
                        +formatVal(1000.0*(residual_->stat.frames - probe_->stat.frames)*1000 / probe_->stat.rate)
                        +"msec)"};
    return std::nullopt;
}


/**
 * Use the precomputed raw statistics values
 * to yield a RMS measurement characterising the diff.
 * The probe's RMS is used for reference, since a Synth
 * can not be expected to produce levelled output.
 * @return peak RMS values observed on the diff over a 30ms window,
 *         given in decibel relative to overall RMS of the probe.
 */
double SoundProbe::getDiffRMSPeak()  const
{
    if (not hasDiff())
        throw error::LogicBroken("Need to compute a diff first.");
    return 20/2*log10(residual_->stat.rmsMax/probe_->stat.rmsAll);
}   // note: raw values in stat.rmsXXX are squares (σ²)  √ ⟹ 1/2

double SoundProbe::getProbePeak()  const
{
    if (not probe_)
        throw error::LogicBroken("No sound probe loaded yet.");
    return 20*log10(probe_->stat.peak);
}       // 20 because dB relates powers (squared)

double SoundProbe::getDuration()  const  ///< @return seconds
{
    if (not probe_)
        throw error::LogicBroken("No sound probe loaded yet.");
    return double(probe_->stat.frames) / probe_->stat.rate;
}


/** write the probe sound data into a WAV file */
void SoundProbe::saveProbe(fs::path name)
{
    if (not probe_)
        throw error::LogicBroken("Nothing to write, no sound data loaded yet.");
    writeSoundData(probe_->buffer, openSndfileWrite(name, probe_->stat.rate));
}


/** write the calculated residual sound data into a WAV file */
void SoundProbe::saveResidual(fs::path name)
{
    if (not hasDiff())
        throw error::LogicBroken("Need to compute a diff first.");
    writeSoundData(residual_->buffer, openSndfileWrite(name, residual_->stat.rate));
}


/*///////////////////////////////////////////////////TODO
string SoundProbe::describeProbe()  const
{
    UNIMPLEMENTED("render descriptive statistics of the sound probe for the test report");
}


string SoundProbe::describeResidual()  const
{
    UNIMPLEMENTED("render descriptive statistics of the calculated residual for the test report");
}
*////////////////////////////////////////////////////TODO


}//(End)namespace util
