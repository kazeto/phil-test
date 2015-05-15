#pragma once

#include <gtest/gtest.h>
#include <phillip.h>
#include <binary.h>


namespace phil
{

void setup_kb(
    const std::string &filename,
    const std::string &key_dist,
    const std::string &key_table)
{
    phillip_main_t::set_verbose(NOT_VERBOSE);
    kb::knowledge_base_t::setup(filename, 4.0f, 1, false);
    kb::kb()->set_distance_provider(key_dist);
    kb::kb()->set_category_table(key_table);
    kb::kb()->prepare_compile();
}


void insert_axioms(const std::string &str)
{
    int n_imp(0), n_inc(0), n_uni(0);
    std::list<lf::logical_function_t> funcs;
            
    lf::parse(str, &funcs);
    for (auto func : funcs)
    {
        if(func.is_operator(lf::OPR_IMPLICATION))
        {
            std::string name = format("imp_%d", n_imp);
            kb::kb()->insert_implication(func, name);
            ++n_imp;
        }
        else if(func.is_operator(lf::OPR_INCONSISTENT))
        {
            kb::kb()->insert_inconsistency(func);
            ++n_inc;
        }
        else if(func.is_operator(lf::OPR_UNIPP))
        {
            kb::kb()->insert_unification_postponement(func);
            ++n_uni;
        }
    }
}


int insert_implications(const std::string &str)
{
    int n_imp(0);
    std::list<lf::logical_function_t> funcs;
            
    lf::parse(str, &funcs);
    for (auto func : funcs)
    {
        assert(func.is_operator(lf::OPR_IMPLICATION));

        std::string name = format("imp_%d", n_imp);
        kb::kb()->insert_implication(func, name);
        ++n_imp;
    }

    return n_imp;
}


int insert_inconsistencies(const std::string &str)
{
    int n_inc(0);
    std::list<lf::logical_function_t> funcs;
            
    lf::parse(str, &funcs);
    for (auto func : funcs)
    {
        assert(func.is_operator(lf::OPR_INCONSISTENT));

        kb::kb()->insert_inconsistency(func);
        ++n_inc;
    }

    return n_inc;
}


int insert_unification_postponements(const std::string &str)
{
    int n_uni(0);
    std::list<lf::logical_function_t> funcs;
            
    lf::parse(str, &funcs);
    for (auto func : funcs)
    {
        assert(func.is_operator(lf::OPR_UNIPP));

        kb::kb()->insert_unification_postponement(func);
        ++n_uni;
    }

    return n_uni;
}


}
