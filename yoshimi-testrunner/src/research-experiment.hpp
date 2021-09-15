/** @file research-experiment.hpp
 ** Temporary playground to try out some design alternatives.
 ** @warning should be included directly into Main.cpp
 ** @todo WIP as of 9/21
 ** @todo throw it away when done!
 ** 
 */


#ifndef TESTRUNNER_RESEARCH_EXPERIMENT_HPP_
#define TESTRUNNER_RESEARCH_EXPERIMENT_HPP_


#include "util/nocopy.hpp"
#include "util/error.hpp"
#include "util/utils.hpp"
//#include "Config.hpp"

//#include <string>
//#include <memory>
#include <utility>
#include <vector>
#include <tuple>

using std::vector;
using std::tuple;

using util::isnil;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;



template<class FUN, typename...ELMS>
void forEach(tuple<ELMS...>&& tuple, FUN fun)
{
    std::apply([&fun](auto&... elms)
                    {
                        (fun(elms), ...);
                    }
              ,tuple);
}


template<typename VAL>
struct Column : util::NonCopyable
{
    string header;
    vector<VAL> data;


    Column(string headerID)
        : header{headerID}
        , data{}
    { }

    VAL& get()
    {
        if (isnil(data))
            throw error::State("No rows in DataTable yet");
        return data.back();
    }

    operator VAL&()
    {
        return get();
    }

    template<typename X>
    VAL& operator=(X&& newVal)
    {
        return get() = std::forward<X>(newVal);
    }
};



template<class TAB>
class DataFile
    : public TAB
    , util::NonCopyable
{

public:
    DataFile()
    {
        forEach(TAB::allColumns(),
                [](auto& col)
                {
                    col.data.resize(col.data.size()+1);
                });
    }

    template<size_t i>
    decltype(auto) getCol()
    {
        return std::get<i>(TAB::allColumns());
    }

    template<size_t i>
    decltype(auto) getStorage()
    {
        return getCol<i>().data;
    }
    template<size_t i>
    string getHeader()
    {
        return getCol<i>().header;
    }
};




struct Storage
{
    Column<string> name{"theName"};
    Column<int>    count{"counter"};
    Column<double> x{"X value"};
    Column<double> y{"Y value"};

    auto allColumns(){ return std::tie(name,count,x,y); }
};



int run_experiment_test()
{
    using Dataz = DataFile<Storage>;

    Dataz daz;

    daz.name = "namberger";
    daz.x = 22.33;

    cout << daz.getHeader<0>()<<":"<<daz.getStorage<0>().back() <<"\n";
    cout << daz.getHeader<1>()<<":"<<daz.getStorage<1>().back() <<"\n";
    cout << daz.getHeader<2>()<<":"<<daz.getStorage<2>().back() <<"\n";
    cout << daz.getHeader<3>()<<":"<<daz.getStorage<3>().back() <<"\n";

    cerr << "Honk.\n\n" << endl;
    return 0;
}


#endif /*TESTRUNNER_RESEARCH_EXPERIMENT_HPP_*/
