#pragma once

#include <gtest/gtest.h>
#include <phillip.h>
#include <binary.h>


namespace phtest
{

using namespace phil;


void setup_kb(
    const std::string &filename,
    const std::string &key_dist,
    const std::string &key_table,
    float max_dist)
{
    phillip_main_t::set_verbose(NOT_VERBOSE);
    kb::knowledge_base_t::setup(filename, max_dist, 1, false);
    kb::kb()->set_distance_provider(key_dist);
    kb::kb()->set_category_table(key_table);
    kb::kb()->prepare_compile();
}


int insert_implications(const std::string &str)
{
    int n_imp(0);
    std::list<lf::logical_function_t> funcs;
            
    lf::parse(str, &funcs);
    for (auto func : funcs)
    {
        assert(
            func.is_operator(lf::OPR_IMPLICATION) or
            func.is_operator(lf::OPR_PARAPHRASE));

        std::string name = util::format("imp_%d", n_imp);
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


lf::input_t make_input(const std::string &name, const std::string &input_str)
{
    std::list<lf::logical_function_t> input_lf;
    lf::parse(input_str, &input_lf);
            
    lf::input_t input;
    input.name = name;
            
    for (auto func : input_lf)
    {
        if (func.is_operator(lf::OPR_AND))
            input.obs = func;
        else if (func.is_operator(lf::OPR_REQUIREMENT))
            input.req = func;
    }
            
    return input;
}



}
