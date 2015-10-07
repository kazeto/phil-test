#include <list>
#include "./test.h"

namespace phtest
{

using namespace phil;


const std::string KB_PATH = "compiled/kb";


TEST(Preliminary, Version)
{
    ASSERT_EQ("phil.3.16", phillip_main_t::VERSION);
}


TEST(UtilityTest, StringHash)
{
    string_hash_t x("x"), X("X"), xh("*x");
    string_hash_t u(string_hash_t::get_unknown_hash());
    
    EXPECT_TRUE(X.is_constant());
    EXPECT_TRUE(not x.is_constant());
    EXPECT_TRUE(u.is_unknown());
    EXPECT_TRUE(not x.is_unknown());
    EXPECT_TRUE(xh.is_hard_term());
    EXPECT_TRUE(not x.is_hard_term());
}


TEST(UtilityTest, Literal)
{
    literal_t p("p", "x", "y", true);
    literal_t np("p", "x", "y", false);
    literal_t eq("=", "x", "y", true);
    literal_t neq("=", "x", "y", false);

    EXPECT_TRUE(not p.is_equality());
    EXPECT_TRUE(eq.is_equality());
    EXPECT_TRUE(neq.is_equality());
    EXPECT_EQ("p/2", p.get_arity());
    EXPECT_EQ("!p/2", np.get_arity());
}


TEST(CompileKBTest, BasicDistance)
{
    phillip_main_t ph;
    ph.set_param("distance_provider", "basic");
    ph.set_param("category_table", "null");
    ph.set_param("kb_max_distance", "4.0");
    kb::knowledge_base_t::initialize(KB_PATH, &ph);
    kb::kb()->prepare_compile();

    EXPECT_EQ(4.0f, kb::kb()->get_max_distance());
    EXPECT_EQ(KB_PATH, kb::kb()->filename());
    ASSERT_TRUE(kb::kb()->is_writable());
    
    int n_imp = insert_implications(
        "(=> (dog-n x) (animal-n x))"
        "(=> (cat-n x) (animal-n x))"
        "(=> (^ (shot-v *e1) (dobj *e1 x)) (^ (die-v *e2) (nsubj *e2 x)))");
    int n_inc = insert_inconsistencies(
        "(xor (dog-n x) (cat-n x))");
    int n_uni = insert_unification_postponements(
        "(unipp (nsubj * .))"
        "(unipp (dobj * .))"
        "(unipp (iobj * .))");

    EXPECT_EQ(3, n_imp);
    EXPECT_EQ(1, n_inc);
    EXPECT_EQ(3, n_uni);

    kb::kb()->finalize();
    kb::kb()->prepare_query();

    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("dog-n/1"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("animal-n/1"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("shot-v/1"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("die-v/1"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("nsubj/2"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("dobj/2"));

    EXPECT_EQ(1.0f, kb::kb()->get_distance(0));
    EXPECT_EQ(-1.0f, kb::kb()->get_distance("dog-n/1", "cat-n/1"));
    EXPECT_EQ(1.0f, kb::kb()->get_distance("dog-n/1", "animal-n/1"));
    EXPECT_EQ(1.0f, kb::kb()->get_distance("dog-n/1", "animal-n/1"));

    EXPECT_TRUE(kb::kb()->find_unification_postponement("nsubj/2") != NULL);
    EXPECT_TRUE(kb::kb()->find_unification_postponement("dobj/2") != NULL);
    EXPECT_TRUE(kb::kb()->find_unification_postponement("iobj/2") != NULL);
}


TEST(CompileKBTest, CostBasedDistance)
{
    phillip_main_t ph;
    ph.set_param("distance_provider", "cost");
    ph.set_param("category_table", "null");
    ph.set_param("kb_max_distance", "4.0");
    kb::knowledge_base_t::initialize(KB_PATH, &ph);
    kb::kb()->prepare_compile();
    
    EXPECT_EQ(4.0f, kb::kb()->get_max_distance());
    EXPECT_EQ(KB_PATH, kb::kb()->filename());
    ASSERT_TRUE(kb::kb()->is_writable());
    
    int n_imp = insert_implications(
        "(=> (dog-n x) (animal-n x) :1.2)"
        "(=> (poodle-n x) (dog-n x) :2.0)"
        "(=> (cat-n x) (animal-n x) :1.2)");
    int n_inc = insert_inconsistencies(
        "(xor (dog-n x) (cat-n x))");

    EXPECT_EQ(3, n_imp);
    EXPECT_EQ(1, n_inc);

    kb::kb()->finalize();
    kb::kb()->prepare_query();

    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("dog-n/1"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("cat-n/1"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("poodle-n/1"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("animal-n/1"));

    EXPECT_EQ(1.2f, kb::kb()->get_distance(0));
    EXPECT_EQ(2.0f, kb::kb()->get_distance(1));
    EXPECT_EQ(1.2f, kb::kb()->get_distance(2));
    
    EXPECT_EQ(-1.0f, kb::kb()->get_distance("dog-n/1", "cat-n/1"));
    EXPECT_EQ(1.2f, kb::kb()->get_distance("dog-n/1", "animal-n/1"));
    EXPECT_EQ(1.2f, kb::kb()->get_distance("cat-n/1", "animal-n/1"));
    EXPECT_EQ(2.0f, kb::kb()->get_distance("dog-n/1", "poodle-n/1"));
    EXPECT_EQ(3.2f, kb::kb()->get_distance("poodle-n/1", "animal-n/1"));
}


TEST(CompileKBTest, BasicCategoryTable)
{
    phillip_main_t ph;
    ph.set_param("distance_provider", "basic");
    ph.set_param("category_table", "basic");
    ph.set_param("kb_max_distance", "4.0");
    kb::knowledge_base_t::initialize(KB_PATH, &ph);
    kb::kb()->prepare_compile();
    
    ASSERT_TRUE(kb::kb()->is_writable());
    
    int n_imp = insert_implications(
        "(=> (dog-n x) (animal-n x))"
        "(=> (poodle-n x) (dog-n x))"
        "(=> (cat-n x) (animal-n x))"
        "(=> (dog-n x) (^ (have-v e) (nsubj e x) (dobj e y) (tail-n y)))");

    EXPECT_EQ(4, n_imp);

    kb::kb()->finalize();
    kb::kb()->prepare_query();

    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("dog-n/1"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("animal-n/1"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("poodle-n/1"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("cat-n/1"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("tail-n/1"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("nsubj/2"));
    EXPECT_NE(kb::INVALID_AXIOM_ID, kb::kb()->search_arity_id("dobj/2"));

    const kb::category_table_t *tab = kb::kb()->category_table();

    EXPECT_TRUE(tab->do_target("dog-n/1"));
    EXPECT_TRUE(tab->do_target("animal-n/1"));
    EXPECT_TRUE(tab->do_target("poodle-n/1"));
    EXPECT_TRUE(tab->do_target("cat-n/1"));
    EXPECT_TRUE(tab->do_target("tail-n/1"));
    EXPECT_TRUE(tab->do_target("have-v/1"));
    EXPECT_FALSE(tab->do_target("nsubj/2"));
    EXPECT_FALSE(tab->do_target("dobj/2"));
    
    EXPECT_EQ(2.0f, tab->get("dog-n/1", "cat-n/1"));
    EXPECT_EQ(2.0f, tab->get("cat-n/1", "dog-n/1"));
    EXPECT_EQ(1.0f, tab->get("dog-n/1", "animal-n/1"));
    EXPECT_EQ(1.0f, tab->get("cat-n/1", "animal-n/1"));
    EXPECT_EQ(3.0f, tab->get("cat-n/1", "poodle-n/1"));
}


namespace os
{
const unsigned char lhs = 0x01;
const unsigned char ilp = 0x02;
const unsigned char sol = 0x04;
const unsigned char out = 0x08;
}


/** A fixture class for testing inference with Phillip. */
class PhillipTest :
        public phillip_main_t,
        public ::testing::Test
{
protected:
    virtual void SetUp() override
        {
            string_hash_t::reset_unknown_hash_count();
            set_verbose(NOT_VERBOSE);
            set_param("max_distance", "4.0");
        }

    /** Prepares for compiling KB. */
    void prepare_compile(
        const std::string &key_dist,
        const std::string &key_tab)
        {
            set_param("distance_provider", key_dist);
            set_param("category_table", key_tab);
            set_param("kb_max_distance", "4.0");
            kb::knowledge_base_t::initialize(KB_PATH, this);
            kb::kb()->prepare_compile();
        }

    void prepare_infer(
        const std::string &key_lhs,
        const std::string &key_ilp,
        const std::string &key_sol)
        {
            set_lhs_enumerator(
                bin::lhs_enumerator_library_t::instance()
                ->generate(key_lhs, this));
            set_ilp_convertor(
                bin::ilp_converter_library_t::instance()
                ->generate(key_ilp, this));
            set_ilp_solver(
                bin::ilp_solver_library_t::instance()
                ->generate(key_sol, this));
            kb::kb()->prepare_query();
        }

    void write(const std::string &prefix, unsigned char flags = 0x0f)
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
                get_solutions().front().print_graph(&fo);
                write_footer(&fo);
            }
        }
};


TEST_F(PhillipTest, DepthBasedEnumerator01)
{
    prepare_compile("basic", "null");
    insert_implications(
        "(=> (dog-n x) (animal-n x))"
        "(=> (cat-n x) (animal-n x))"
        "(=> (^ (kill-v *e1) (nsubj *e1 u) (dobj *e1 x))"
        "    (^ (die-v *e2) (nsubj *e2 x)))"
        "(=> (^ (hate-v *e1) (nsubj *e1 x) (dobj *e1 y))"
        "    (^ (kill-v *e2) (nsubj *e2 x) (dobj *e2 y)))");
    insert_unification_postponements(
        "(unipp (nsubj * .))"
        "(unipp (dobj * .))");

    prepare_infer("depth", "null", "null");
    ASSERT_TRUE(check_validity());

    infer(make_input(
        "Test1",
        "(^ (hate-v E1) (nsubj E1 John) (dobj E1 Tom)"
        "   (die-v E2) (nsubj E2 Tom) (animal-n A))"));
    write("log/depth", os::lhs);

    const pg::proof_graph_t *graph = get_latent_hypotheses_set();

    EXPECT_EQ(16, graph->nodes().size());
    EXPECT_EQ(6, count_observation_nodes(graph));
    EXPECT_EQ(8, count_hypothesis_nodes(graph));
    EXPECT_EQ(2, count_unification_nodes(graph));
    
    EXPECT_EQ(7, graph->edges().size());
    EXPECT_EQ(4, count_chaining_edges(graph));
    EXPECT_EQ(3, count_unifying_edges(graph));
}


TEST_F(PhillipTest, AStarBasedEnumerator01)
{
    prepare_compile("basic", "null");
    insert_implications(
        "(=> (dog-n x) (animal-n x))"
        "(=> (cat-n x) (animal-n x))"
        "(=> (^ (kill-v *e1) (nsubj *e1 u) (dobj *e1 x))"
        "    (^ (die-v *e2) (nsubj *e2 x)))"
        "(=> (^ (hate-v *e1) (nsubj *e1 x) (dobj *e1 y))"
        "    (^ (kill-v *e2) (nsubj *e2 x) (dobj *e2 y)))");
    insert_unification_postponements(
        "(unipp (nsubj * .))"
        "(unipp (dobj * .))");

    prepare_infer("a*", "null", "null");
    ASSERT_TRUE(check_validity());

    infer(make_input(
        "Test1",
        "(^ (hate-v E1) (nsubj E1 John) (dobj E1 Tom)"
        "   (die-v E2) (nsubj E2 Tom) (animal-n A))"));
    write("log/astar.01", os::lhs);

    const pg::proof_graph_t *graph = get_latent_hypotheses_set();    

    ASSERT_EQ(14, graph->nodes().size());
    EXPECT_EQ(6, count_observation_nodes(graph));
    EXPECT_EQ(6, count_hypothesis_nodes(graph));
    EXPECT_EQ(2, count_unification_nodes(graph));
    
    ASSERT_EQ(5, graph->edges().size());
    EXPECT_EQ(2, count_chaining_edges(graph));
    EXPECT_EQ(3, count_unifying_edges(graph));
}


TEST_F(PhillipTest, AStarBasedEnumerator02)
{
    prepare_compile("basic", "null");
    insert_implications(
        "(=> (criminal-j e1 x) (arrest-v e2 y x))"
        "(=> (kill-v e1 x y) (criminal-j e2 x))"
        "(=> (rob-v e1 x y) (criminal-j e2 x))"
        "(=> (murder-v e x y) (kill-v e x y))");
    
    lf::input_t input = make_input(
        "Test1",
        "(^ (arrest-v E1 Police man) (murder-v E2 John Tom) (rob-v E3 Bob Shop))");

    {
        set_param("max_distance", "3.0");
        prepare_infer("a*", "null", "null");
        ASSERT_TRUE(check_validity());
    
        infer(input);
        write("log/astar.02a", os::lhs);

        const pg::proof_graph_t *graph = get_latent_hypotheses_set();    

        ASSERT_EQ(13, graph->nodes().size());
        EXPECT_EQ(3, count_observation_nodes(graph));
        EXPECT_EQ(4, count_hypothesis_nodes(graph));
        EXPECT_EQ(6, count_unification_nodes(graph));
        
        ASSERT_EQ(6, graph->edges().size());
        EXPECT_EQ(4, count_chaining_edges(graph));
        EXPECT_EQ(2, count_unifying_edges(graph));
    }

    {
        set_param("max_distance", "2.0");
        prepare_infer("a*", "null", "null");
        ASSERT_TRUE(check_validity());

        infer(input);
        write("log/astar.02b", os::lhs);

        const pg::proof_graph_t *graph = get_latent_hypotheses_set();    

        ASSERT_EQ(8, graph->nodes().size());
        EXPECT_EQ(3, count_observation_nodes(graph));
        EXPECT_EQ(2, count_hypothesis_nodes(graph));
        EXPECT_EQ(3, count_unification_nodes(graph));
    
        ASSERT_EQ(3, graph->edges().size());
        EXPECT_EQ(2, count_chaining_edges(graph));
        EXPECT_EQ(1, count_unifying_edges(graph));
    }
}


TEST_F(PhillipTest, Weighted01)
{
    prepare_compile("basic", "null");
    insert_implications(
        "(=> (^ (kill-v *e1) (nsubj *e1 u) (dobj *e1 x))"
        "    (^ (die-v *e2) (nsubj *e2 x)))"
        "(=> (^ (hate-v *e1) (nsubj *e1 x) (dobj *e1 y))"
        "    (^ (kill-v *e2) (nsubj *e2 x) (dobj *e2 y)))");
    insert_unification_postponements(
        "(unipp (nsubj * .))"
        "(unipp (dobj * .))");
    
    prepare_infer("a*", "weighted", "gurobi");
    ASSERT_TRUE(check_validity());

    infer(make_input(
        "Test1",
        "(^ (hate-v E1) (nsubj E1 John) (dobj E1 Tom)"
        "   (die-v E2) (nsubj E2 Tom))"));
    write("log/weighted.01");

    const pg::proof_graph_t *graph = get_latent_hypotheses_set();
    const ilp::ilp_problem_t *prob = get_ilp_problem();
    const ilp::ilp_solution_t &sol = get_solutions().front();
    
    ASSERT_EQ(ilp::SOLUTION_OPTIMAL, sol.type());
    
    EXPECT_EQ(13, graph->nodes().size());
    EXPECT_EQ(5, count_observation_nodes(graph));
    EXPECT_EQ(6, count_hypothesis_nodes(graph));
    EXPECT_EQ(2, count_unification_nodes(graph));

    EXPECT_EQ(13, count_active_nodes(sol));
    EXPECT_EQ(5, count_active_observation_nodes(sol));
    EXPECT_EQ(6, count_active_hypothesis_nodes(sol));
    EXPECT_EQ(2, count_active_unification_nodes(sol));

    EXPECT_EQ(5, graph->edges().size());
    EXPECT_EQ(2, count_chaining_edges(graph));
    EXPECT_EQ(3, count_unifying_edges(graph));
    
    EXPECT_EQ(5, count_active_edges(sol));
    EXPECT_EQ(2, count_active_chaining_edges(sol));
    EXPECT_EQ(3, count_active_unifying_edges(sol));
}


TEST_F(PhillipTest, Weighted02)
{
    prepare_compile("basic", "null");
    insert_implications(
        "(=> (dog-n x :1.2) (animal-n x))"
        "(=> (cat-n x :1.4) (animal-n x))");
    insert_inconsistencies(
        "(xor (dog-n x) (cat-n x))");
    
    prepare_infer("a*", "weighted", "gurobi");
    ASSERT_TRUE(check_validity());

    infer(make_input(
        "Test2",
        "(^ (animal-n a :10) (cat-n c :20) (dog-n d :20))"));
    write("log/weighted.02");

    const pg::proof_graph_t *graph = get_latent_hypotheses_set();
    const ilp::ilp_problem_t *prob = get_ilp_problem();
    const ilp::ilp_solution_t &sol = get_solutions().front();
    
    ASSERT_EQ(ilp::SOLUTION_OPTIMAL, sol.type());
    
    EXPECT_EQ(8, graph->nodes().size());
    EXPECT_EQ(3, count_observation_nodes(graph));
    EXPECT_EQ(2, count_hypothesis_nodes(graph));
    EXPECT_EQ(3, count_unification_nodes(graph));

    EXPECT_EQ(5, count_active_nodes(sol));
    EXPECT_EQ(3, count_active_observation_nodes(sol));
    EXPECT_EQ(1, count_active_hypothesis_nodes(sol));
    EXPECT_EQ(1, count_active_unification_nodes(sol));

    EXPECT_EQ(4, graph->edges().size());
    EXPECT_EQ(2, count_chaining_edges(graph));
    EXPECT_EQ(2, count_unifying_edges(graph));
    
    EXPECT_EQ(2, count_active_edges(sol));
    EXPECT_EQ(1, count_active_chaining_edges(sol));
    EXPECT_EQ(1, count_active_unifying_edges(sol));

    EXPECT_TRUE(exists_in("dog-n/1", sol));
}


TEST_F(PhillipTest, Costed01)
{
    prepare_compile("basic", "null");
    insert_implications(
        "(=> (^ (kill-v *e1) (nsubj *e1 u) (dobj *e1 x))"
        "    (^ (die-v *e2) (nsubj *e2 x)))"
        "(=> (^ (hate-v *e1) (nsubj *e1 x) (dobj *e1 y))"
        "    (^ (kill-v *e2) (nsubj *e2 x) (dobj *e2 y)))");
    insert_unification_postponements(
        "(unipp (nsubj * .))"
        "(unipp (dobj * .))");

    set_param("cost_provider", "basic(10.0,-50.0,4.0)");
    prepare_infer("a*", "costed", "gurobi");
    ASSERT_TRUE(check_validity());

    infer(make_input(
        "Test1",
        "(^ (hate-v E1) (nsubj E1 John) (dobj E1 Tom)"
        "   (die-v E2) (nsubj E2 Tom))"));
    write("log/costed.01");

    const pg::proof_graph_t *graph = get_latent_hypotheses_set();
    const ilp::ilp_problem_t *prob = get_ilp_problem();
    const ilp::ilp_solution_t &sol = get_solutions().front();
    
    ASSERT_EQ(ilp::SOLUTION_OPTIMAL, sol.type());

    EXPECT_EQ(13, graph->nodes().size());
    EXPECT_EQ(5, count_observation_nodes(graph));
    EXPECT_EQ(6, count_hypothesis_nodes(graph));
    EXPECT_EQ(2, count_unification_nodes(graph));

    EXPECT_EQ(13, count_active_nodes(sol));
    EXPECT_EQ(5, count_active_observation_nodes(sol));
    EXPECT_EQ(6, count_active_hypothesis_nodes(sol));
    EXPECT_EQ(2, count_active_unification_nodes(sol));

    EXPECT_EQ(5, graph->edges().size());
    EXPECT_EQ(2, count_chaining_edges(graph));
    EXPECT_EQ(3, count_unifying_edges(graph));
    
    EXPECT_EQ(5, count_active_edges(sol));
    EXPECT_EQ(2, count_active_chaining_edges(sol));
    EXPECT_EQ(3, count_active_unifying_edges(sol));
}


TEST_F(PhillipTest, KBest01)
{
    prepare_compile("basic", "null");
    insert_implications(
        "(=> (dog-n x :1.3) (animal-n x))"
        "(=> (cat-n x :1.4) (animal-n x))");
    insert_inconsistencies(
        "(xor (dog-n x) (cat-n x))");

    set_param("max_sols_num", "5");
    set_param("sols_threshold", "5.0");
    set_param("sols_margin", "1");
    prepare_infer("a*", "weighted", "gurobi_kbest");
    ASSERT_TRUE(check_validity());

    infer(make_input(
        "Test2",
        "(^ (animal-n a :10) (cat-n c :20) (dog-n d :20))"));
    write("log/kbest.01");

    const pg::proof_graph_t *graph = get_latent_hypotheses_set();
    const ilp::ilp_problem_t *prob = get_ilp_problem();
    const std::vector<ilp::ilp_solution_t> &sols = get_solutions();

    ASSERT_EQ(2, sols.size());
    ASSERT_EQ(ilp::SOLUTION_OPTIMAL, sols.at(0).type());
    ASSERT_EQ(ilp::SOLUTION_OPTIMAL, sols.at(1).type());
    
    EXPECT_EQ(8, graph->nodes().size());
    EXPECT_EQ(3, count_observation_nodes(graph));
    EXPECT_EQ(2, count_hypothesis_nodes(graph));
    EXPECT_EQ(3, count_unification_nodes(graph));

    EXPECT_EQ(4, graph->edges().size());
    EXPECT_EQ(2, count_chaining_edges(graph));
    EXPECT_EQ(2, count_unifying_edges(graph));

    // --- TESTING THE 1ST SOLUTION
    
    EXPECT_EQ(5, count_active_nodes(sols.at(0)));
    EXPECT_EQ(3, count_active_observation_nodes(sols.at(0)));
    EXPECT_EQ(1, count_active_hypothesis_nodes(sols.at(0)));
    EXPECT_EQ(1, count_active_unification_nodes(sols.at(0)));
    
    EXPECT_EQ(2, count_active_edges(sols.at(0)));
    EXPECT_EQ(1, count_active_chaining_edges(sols.at(0)));
    EXPECT_EQ(1, count_active_unifying_edges(sols.at(0)));
    
    EXPECT_TRUE(exists_in("dog-n/1", sols.at(0)));
    EXPECT_EQ(33.0, sols.at(0).value_of_objective_function());

    // --- TESTING THE 2ND SOLUTION

    EXPECT_EQ(5, count_active_nodes(sols.at(1)));
    EXPECT_EQ(3, count_active_observation_nodes(sols.at(1)));
    EXPECT_EQ(1, count_active_hypothesis_nodes(sols.at(1)));
    EXPECT_EQ(1, count_active_unification_nodes(sols.at(1)));
    
    EXPECT_EQ(2, count_active_edges(sols.at(1)));
    EXPECT_EQ(1, count_active_chaining_edges(sols.at(1)));
    EXPECT_EQ(1, count_active_unifying_edges(sols.at(1)));
    
    EXPECT_TRUE(exists_in("cat-n/1", sols.at(1)));
    EXPECT_EQ(34.0, sols.at(1).value_of_objective_function());
}



}
