#pragma once

#include <gtest/gtest.h>
#include <phillip.h>
#include <binary.h>


namespace phtest
{

using namespace phil;


int insert_implications(const std::string &str)
{
    int n_imp(0);
    std::list<lf::logical_function_t> funcs;
            
    lf::parse(str, &funcs);
    for (auto func : funcs)
    {
        assert(func.is_operator(lf::OPR_IMPLICATION));

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


int count_observation_nodes(const pg::proof_graph_t *graph)
{
    int out(0);
    for (pg::node_idx_t i = 0; i < graph->nodes().size(); ++i)
        if (graph->node(i).type() == pg::NODE_OBSERVABLE)
            ++out;
    return out;
}


int count_hypothesis_nodes(const pg::proof_graph_t *graph)
{
    int out(0);
    for (pg::node_idx_t i = 0; i < graph->nodes().size(); ++i)
        if (graph->node(i).type() == pg::NODE_HYPOTHESIS)
            if (not graph->node(i).literal().is_equality())
                ++out;
    return out;
}


int count_unification_nodes(const pg::proof_graph_t *graph)
{
    int out(0);
    for (pg::node_idx_t i = 0; i < graph->nodes().size(); ++i)
        if (graph->node(i).type() == pg::NODE_HYPOTHESIS)
            if (graph->node(i).is_equality_node())
                ++out;
    return out;
}


int count_active_nodes(const ilp::ilp_solution_t &sol)
{
    int out(0);
    const pg::proof_graph_t *graph = sol.proof_graph();
    ilp::basic_solution_interpreter_t interpreter;
    
    for (pg::node_idx_t i = 0; i < graph->nodes().size(); ++i)
        if (interpreter.node_is_active(sol, i))
            ++out;

    return out;
}


int count_active_observation_nodes(const ilp::ilp_solution_t &sol)
{
    int out(0);
    const pg::proof_graph_t *graph = sol.proof_graph();
    ilp::basic_solution_interpreter_t interpreter;
    
    for (pg::node_idx_t i = 0; i < graph->nodes().size(); ++i)
        if (graph->node(i).type() == pg::NODE_OBSERVABLE)
            if (interpreter.node_is_active(sol, i))
                ++out;

    return out;
}


int count_active_hypothesis_nodes(const ilp::ilp_solution_t &sol)
{
    int out(0);
    const pg::proof_graph_t *graph = sol.proof_graph();
    ilp::basic_solution_interpreter_t interpreter;
    
    for (pg::node_idx_t i = 0; i < graph->nodes().size(); ++i)
        if (graph->node(i).type() == pg::NODE_HYPOTHESIS)
            if (not graph->node(i).is_equality_node())
                if (interpreter.node_is_active(sol, i))
                    ++out;

    return out;
}


int count_active_unification_nodes(const ilp::ilp_solution_t &sol)
{
    int out(0);
    const pg::proof_graph_t *graph = sol.proof_graph();
    ilp::basic_solution_interpreter_t interpreter;
    
    for (pg::node_idx_t i = 0; i < graph->nodes().size(); ++i)
        if (graph->node(i).type() == pg::NODE_HYPOTHESIS)
            if (graph->node(i).is_equality_node())
                if (interpreter.node_is_active(sol, i))
                    ++out;

    return out;
}


int count_chaining_edges(const pg::proof_graph_t *graph)
{
    int out(0);
    for (pg::edge_idx_t i = 0; i < graph->edges().size(); ++i)
        if (graph->edge(i).is_chain_edge())
            ++out;
    return out;
}


int count_unifying_edges(const pg::proof_graph_t *graph)
{
    int out(0);
    for (pg::edge_idx_t i = 0; i < graph->edges().size(); ++i)
        if (graph->edge(i).is_unify_edge())
            ++out;
    return out;
}


int count_active_edges(const ilp::ilp_solution_t &sol)
{
    int out(0);
    const pg::proof_graph_t *graph = sol.proof_graph();
    ilp::basic_solution_interpreter_t interpreter;
    
    for (pg::edge_idx_t i = 0; i < graph->edges().size(); ++i)
        if (interpreter.edge_is_active(sol, i))
            ++out;

    return out;
}


int count_active_chaining_edges(const ilp::ilp_solution_t &sol)
{
    int out(0);
    const pg::proof_graph_t *graph = sol.proof_graph();
    ilp::basic_solution_interpreter_t interpreter;
    
    for (pg::edge_idx_t i = 0; i < graph->edges().size(); ++i)
        if (graph->edge(i).is_chain_edge())
            if (interpreter.edge_is_active(sol, i))
                ++out;

    return out;
}


int count_active_unifying_edges(const ilp::ilp_solution_t &sol)
{
    int out(0);
    const pg::proof_graph_t *graph = sol.proof_graph();
    ilp::basic_solution_interpreter_t interpreter;
    
    for (pg::edge_idx_t i = 0; i < graph->edges().size(); ++i)
        if (graph->edge(i).is_unify_edge())
            if (interpreter.edge_is_active(sol, i))
                ++out;

    return out;
}


bool exists_in(arity_t arity, const ilp::ilp_solution_t &sol)
{
    ilp::basic_solution_interpreter_t interpreter;
    
    for (auto n : sol.proof_graph()->nodes())
        if (n.arity() == arity)
            if (interpreter.node_is_active(sol, n.index()))
                return true;

    return false;
}



}
