#include "./phtest_util.h"

namespace phtest
{

using namespace phil;

const std::string KB_PATH = "compiled/kb";


void PhillipTest::SetUp()
{
    string_hash_t::reset_unknown_hash_count();
    set_verbose(NOT_VERBOSE);
    set_param("max_distance", "4.0");
}


void PhillipTest::prepare_compile(
    const std::string &key_dist,
    const std::string &key_tab)
{
    set_param("distance_provider", key_dist);
    set_param("category_table", key_tab);
    set_param("kb_max_distance", "4.0");
    
    kb::knowledge_base_t::initialize(KB_PATH, this);
    kb::kb()->prepare_compile();
}


void PhillipTest::prepare_infer(
    const std::string &key_lhs,
    const std::string &key_ilp,
    const std::string &key_sol)
{
    set_lhs_enumerator(
        bin::lhs_enumerator_library_t::instance()->generate(key_lhs, this));
    set_ilp_convertor(
        bin::ilp_converter_library_t::instance()->generate(key_ilp, this));
    set_ilp_solver(
        bin::ilp_solver_library_t::instance()->generate(key_sol, this));
    
    kb::kb()->prepare_query();
}


void PhillipTest::write(const std::string &prefix, unsigned char flags)
{
    if (flags & os::lhs)
    {
        std::ofstream fo(prefix + ".lhs.xml");
        write_header(&fo);
        get_latent_hypotheses_set()->print(&fo);
        write_footer(&fo);
    }
    
    if (flags & os::ilp)
    {
        std::ofstream fo(prefix + ".ilp.xml");
        write_header(&fo);
        get_ilp_problem()->print(&fo);
        write_footer(&fo);
    }
    
    if (flags & os::sol)
    {
        std::ofstream fo(prefix + ".sol.xml");
        write_header(&fo);
        get_solutions().front().print(&fo);
        write_footer(&fo);
    }
    
    if (flags & os::out)
    {
        std::ofstream fo(prefix + ".out.xml");
        write_header(&fo);
        if (get_training_result() != NULL)
        {
            get_training_result()->write(&fo);
            get_positive_answer().front().print_graph(&fo);
        }
        get_solutions().front().print_graph(&fo);
        write_footer(&fo);
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
