/** @file research-experiment.hpp
 ** Temporary playground to try out some design alternatives.
 ** @warning should be included directly into Main.cpp
 ** @todo WIP as of 9/21
 ** @todo throw it away when done!
 ** 
 */


#ifndef TESTRUNNER_RESEARCH_EXPERIMENT_HPP_
#define TESTRUNNER_RESEARCH_EXPERIMENT_HPP_


#include "util/data.hpp"
//#include "Config.hpp"

//#include <string>
//#include <memory>

using util::Column;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;



struct Storage
{
    Column<string> name{"theName"};
    Column<int>    count{"counter"};
    Column<double> x{"X value"};
    Column<double> y{"Y value"};

    auto allColumns(){ return std::tie(name,count,x,y); }
};



int run_experiment_test()
try {
    using Dataz = util::DataFile<Storage>;

    Dataz daz("daz.csv");
    if (util::isnil(daz))
    {
        daz.newRow();
        daz.x = 123e-4;
        daz.y = -22;
        daz.name = "Namberger";
    }
    else
        daz.dupRow();

    // manipulate current row
    ++daz.count;
    daz.x += 1.1;
    daz.y /= 10.1;
    daz.name.get() += ".";

    cout << daz.dumpCSV() <<endl;
    daz.save(10);

    cerr << "Honk.\n\n" << endl;
    return 0;
}
catch (std::exception& auau)
{
    cerr << "AUA--->"<<auau.what()<<endl;
    return -1;
}


#endif /*TESTRUNNER_RESEARCH_EXPERIMENT_HPP_*/
