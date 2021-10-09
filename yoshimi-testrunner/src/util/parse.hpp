/*
 *  parse - helper to parse spec files
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


/** @file parse.hpp
 ** Parsing config and test spec files.
 ** For configuration and test definitions, a simple INI-File like syntax is used
 ** to define `key = value` bindings, returned as `std::map<string,string>`.
 ** @see parseSpec(fs::path) for details of the syntax
 ** @see Config::fromFile(fs::path)
 **
 */



#ifndef TESTRUNNER_UTIL_PARSE_HPP_
#define TESTRUNNER_UTIL_PARSE_HPP_


#include <filesystem>
#include <string>
#include <vector>
#include <map>

namespace util {

namespace {
    using MapS = std::map<std::string, std::string>;
    using VectorS = std::vector<std::string>;
}



/********************************************************************************//**
 * Parse a config or test specification file.
 *
 * # Syntax
 * The Syntax is line oriented and based on `key = value` associations,
 * similar to INI-Files. All results are collected into a `std::map<string,string>`.
 * Beyond this simple format, some additional features are supported
 * - Line comments can be started with the `'#'` sign
 * - Instead of the `'='` sign, key and value can also be separated by `':'`
 * - The keys may contain dots, allowing for a hierarchical structure..
 * - Definitions can be grouped in sections; a new section is started by a
 *   line with a `[sectionID]` and extends until the next section start.
 *   The keys of all definitions within a section are prefixed by `sectionID.`.
 * - Delimited blocks are opened by a line with just the `blockID` and closed
 *   by a corresponding line with `End-BlockID`. All lines between these markers
 *   are concatenated into the value associated with the key `"blockID"`, including
 *   trailing newline characters.
 *
 * @param path of the text file to parse
 * @return map with all `(key,value)` bindings.
 * @throws error::Misconfig on syntax error; might also throw I/O errors.
 */
MapS parseSpec(std::filesystem::path path);



/**
 * Split a commandline into argument tokens.
 * @remark similar to what a Unix shell does
 *  - split at whitespace
 *  - tokens can be quoted to retain whitespace
 *  - single and double quotes are supported
 *  - embedded quotes within a quoted argument
 *    must be escaped with `\`
 */
VectorS tokeniseCmdline(std::string argline);


}//(End)namespace util
#endif /*TESTRUNNER_UTIL_PARSE_HPP_*/
