#include <list>
#include "./test.h"

namespace phtest
{

using namespace phil;


const std::string KB_PATH = "compiled/kb";


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
    setup_kb(KB_PATH, "basic", "null", 4.0f);

    EXPECT_EQ(4.0f, kb::knowledge_base_t::get_max_distance());
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
    setup_kb(KB_PATH, "cost", "null", 4.0f);
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
    setup_kb(KB_PATH, "basic", "basic", 4.0f);
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

    EXPECT_TRUE(kb::kb()->do_target_on_category_table("dog-n/1"));
    EXPECT_TRUE(kb::kb()->do_target_on_category_table("animal-n/1"));
    EXPECT_TRUE(kb::kb()->do_target_on_category_table("poodle-n/1"));
    EXPECT_TRUE(kb::kb()->do_target_on_category_table("cat-n/1"));
    EXPECT_TRUE(kb::kb()->do_target_on_category_table("tail-n/1"));
    EXPECT_TRUE(kb::kb()->do_target_on_category_table("have-v/1"));
    EXPECT_FALSE(kb::kb()->do_target_on_category_table("nsubj/2"));
    EXPECT_FALSE(kb::kb()->do_target_on_category_table("dobj/2"));

    EXPECT_EQ(2.0f, kb::kb()->get_soft_unifying_cost("dog-n/1", "cat-n/1"));
    EXPECT_EQ(2.0f, kb::kb()->get_soft_unifying_cost("cat-n/1", "dog-n/1"));
    EXPECT_EQ(1.0f, kb::kb()->get_soft_unifying_cost("dog-n/1", "animal-n/1"));
    EXPECT_EQ(1.0f, kb::kb()->get_soft_unifying_cost("cat-n/1", "animal-n/1"));
    EXPECT_EQ(3.0f, kb::kb()->get_soft_unifying_cost("cat-n/1", "poodle-n/1"));
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

    void setup_phillip(
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
};


TEST_F(PhillipTest, DepthBasedEnumerator)
{
    setup_kb(KB_PATH, "basic", "null", 4.0f);
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

    setup_phillip("depth", "null", "null");
    ASSERT_TRUE(check_validity());

    infer(make_input(
        "Test1",
        "(^ (hate-v E1) (nsubj E1 John) (dobj E1 Tom)"
        "   (die-v E2) (nsubj E2 Tom) (animal-n A))"));

    const pg::proof_graph_t *graph = get_latent_hypotheses_set();    
    ASSERT_EQ(16, graph->nodes().size());
    ASSERT_EQ(7, graph->edges().size());

#define EXPECT_NODE(_idx, _lit, _dep) \
    EXPECT_EQ(_lit, graph->node(_idx).literal()); \
    EXPECT_EQ(_dep, graph->node(_idx).depth())
#define EXPECT_EDGE(_idx, _type, _axiom) \
    EXPECT_EQ(_type, graph->edge(_idx).type()); \
    EXPECT_EQ(_axiom, graph->edge(_idx).axiom_id())
    
    EXPECT_NODE(0, literal_t("hate-v", {"E1"}), 0);
    EXPECT_NODE(1, literal_t("nsubj", {"E1", "John"}), 0);
    EXPECT_NODE(2, literal_t("dobj", {"E1", "Tom"}), 0);
    EXPECT_NODE(3, literal_t("die-v", {"E2"}), 0);
    EXPECT_NODE(4, literal_t("nsubj", {"E2", "Tom"}), 0);
    EXPECT_NODE(5, literal_t("animal-n", {"A"}), 0);
    EXPECT_NODE(6, literal_t("dog-n", {"A"}), 1);
    EXPECT_NODE(7, literal_t("cat-n", {"A"}), 1);
    EXPECT_NODE(8, literal_t("kill-v", {"_u1"}), 1);
    EXPECT_NODE(9, literal_t("nsubj", {"_u1", "_u2"}), 1);
    EXPECT_NODE(10, literal_t("dobj", {"_u1", "Tom"}), 1);
    EXPECT_NODE(11, literal_t("hate-v", {"_u3"}), 2);
    EXPECT_NODE(12, literal_t("nsubj", {"_u3", "_u2"}), 2);
    EXPECT_NODE(13, literal_t("dobj", {"_u3", "Tom"}), 2);
    EXPECT_NODE(14, literal_t("=", {"E1", "_u3"}), -1);
    EXPECT_NODE(15, literal_t("=", {"John", "_u2"}), -1);

    EXPECT_EDGE(0, pg::EDGE_HYPOTHESIZE, 0);
    EXPECT_EDGE(1, pg::EDGE_HYPOTHESIZE, 1);
    EXPECT_EDGE(2, pg::EDGE_HYPOTHESIZE, 2);
    EXPECT_EDGE(3, pg::EDGE_HYPOTHESIZE, 3);
    EXPECT_EDGE(4, pg::EDGE_UNIFICATION, -1);
    EXPECT_EDGE(5, pg::EDGE_UNIFICATION, -1);
    EXPECT_EDGE(6, pg::EDGE_UNIFICATION, -1);

#undef EXPECT_NODE
#undef EXPECT_EDGE
}


TEST_F(PhillipTest, AStarBasedEnumerator)
{
    setup_kb(KB_PATH, "basic", "null", 4.0f);
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

    setup_phillip("a*", "null", "null");
    ASSERT_TRUE(check_validity());

    infer(make_input(
        "Test1",
        "(^ (hate-v E1) (nsubj E1 John) (dobj E1 Tom)"
        "   (die-v E2) (nsubj E2 Tom) (animal-n A))"));

    const pg::proof_graph_t *graph = get_latent_hypotheses_set();    
    ASSERT_EQ(14, graph->nodes().size());
    ASSERT_EQ(5, graph->edges().size());

#define EXPECT_NODE(_idx, _lit, _dep) \
    EXPECT_EQ(_lit, graph->node(_idx).literal()); \
    EXPECT_EQ(_dep, graph->node(_idx).depth())
#define EXPECT_EDGE(_idx, _type, _axiom) \
    EXPECT_EQ(_type, graph->edge(_idx).type()); \
    EXPECT_EQ(_axiom, graph->edge(_idx).axiom_id())
    
    EXPECT_NODE(0, literal_t("hate-v", {"E1"}), 0);
    EXPECT_NODE(1, literal_t("nsubj", {"E1", "John"}), 0);
    EXPECT_NODE(2, literal_t("dobj", {"E1", "Tom"}), 0);
    EXPECT_NODE(3, literal_t("die-v", {"E2"}), 0);
    EXPECT_NODE(4, literal_t("nsubj", {"E2", "Tom"}), 0);
    EXPECT_NODE(5, literal_t("animal-n", {"A"}), 0);
    EXPECT_NODE(6, literal_t("kill-v", {"_u1"}), 1);
    EXPECT_NODE(7, literal_t("nsubj", {"_u1", "_u2"}), 1);
    EXPECT_NODE(8, literal_t("dobj", {"_u1", "Tom"}), 1);
    EXPECT_NODE(9, literal_t("hate-v", {"_u3"}), 2);
    EXPECT_NODE(10, literal_t("nsubj", {"_u3", "_u2"}), 2);
    EXPECT_NODE(11, literal_t("dobj", {"_u3", "Tom"}), 2);
    EXPECT_NODE(12, literal_t("=", {"E1", "_u3"}), -1);
    EXPECT_NODE(13, literal_t("=", {"John", "_u2"}), -1);

    EXPECT_EDGE(0, pg::EDGE_HYPOTHESIZE, 2);
    EXPECT_EDGE(1, pg::EDGE_HYPOTHESIZE, 3);
    EXPECT_EDGE(2, pg::EDGE_UNIFICATION, -1);
    EXPECT_EDGE(3, pg::EDGE_UNIFICATION, -1);
    EXPECT_EDGE(4, pg::EDGE_UNIFICATION, -1);

#undef EXPECT_NODE
#undef EXPECT_EDGE
}


TEST_F(PhillipTest, Weighted)
{
    setup_kb(KB_PATH, "basic", "null", 4.0f);
    insert_implications(
        "(=> (^ (kill-v *e1) (nsubj *e1 u) (dobj *e1 x))"
        "    (^ (die-v *e2) (nsubj *e2 x)))"
        "(=> (^ (hate-v *e1) (nsubj *e1 x) (dobj *e1 y))"
        "    (^ (kill-v *e2) (nsubj *e2 x) (dobj *e2 y)))");
    insert_unification_postponements(
        "(unipp (nsubj * .))"
        "(unipp (dobj * .))");
    
    setup_phillip("a*", "weighted", "gurobi");
    ASSERT_TRUE(check_validity());

    infer(make_input(
        "Test1",
        "(^ (hate-v E1) (nsubj E1 John) (dobj E1 Tom)"
        "   (die-v E2) (nsubj E2 Tom))"));

    const pg::proof_graph_t *graph = get_latent_hypotheses_set();
    const ilp::ilp_problem_t *prob = get_ilp_problem();
    const ilp::ilp_solution_t &sol = get_solutions().front();
    
    ASSERT_EQ(13, graph->nodes().size());
    ASSERT_EQ(ilp::SOLUTION_OPTIMAL, sol.type());

#define EXPECT_EQ_LIT(idx, lit) EXPECT_EQ(lit, graph->node(idx).literal())
#define EXPECT_EQ_DEPTH(idx, dep) EXPECT_EQ(dep, graph->node(idx).depth())
#define EXPECT_NODE_ACTIVE(idx) \
    EXPECT_TRUE(sol.variable_is_active(prob->find_variable_with_node(idx)))
#define EXPECT_EDGE_ACTIVE(idx) \
    EXPECT_TRUE(sol.variable_is_active(prob->find_variable_with_edge(idx)))
    
    EXPECT_EQ_LIT(0, literal_t("hate-v", {"E1"}));
    EXPECT_EQ_LIT(1, literal_t("nsubj", {"E1", "John"}));
    EXPECT_EQ_LIT(2, literal_t("dobj", {"E1", "Tom"}));
    EXPECT_EQ_LIT(3, literal_t("die-v", {"E2"}));
    EXPECT_EQ_LIT(4, literal_t("nsubj", {"E2", "Tom"}));
    EXPECT_EQ_LIT(5, literal_t("kill-v", {"_u1"}));
    EXPECT_EQ_LIT(6, literal_t("nsubj", {"_u1", "_u2"}));
    EXPECT_EQ_LIT(7, literal_t("dobj", {"_u1", "Tom"}));
    EXPECT_EQ_LIT(8, literal_t("hate-v", {"_u3"}));
    EXPECT_EQ_LIT(9, literal_t("nsubj", {"_u3", "_u2"}));
    EXPECT_EQ_LIT(10, literal_t("dobj", {"_u3", "Tom"}));
    EXPECT_EQ_LIT(11, literal_t("=", {"E1", "_u3"}));
    EXPECT_EQ_LIT(12, literal_t("=", {"John", "_u2"}));
    
    EXPECT_EQ_DEPTH(0, 0);
    EXPECT_EQ_DEPTH(1, 0);
    EXPECT_EQ_DEPTH(2, 0);
    EXPECT_EQ_DEPTH(3, 0);
    EXPECT_EQ_DEPTH(4, 0);
    EXPECT_EQ_DEPTH(5, 1);
    EXPECT_EQ_DEPTH(6, 1);
    EXPECT_EQ_DEPTH(7, 1);
    EXPECT_EQ_DEPTH(8, 2);
    EXPECT_EQ_DEPTH(9, 2);
    EXPECT_EQ_DEPTH(10, 2);
    EXPECT_EQ_DEPTH(11, -1);
    EXPECT_EQ_DEPTH(12, -1);

    EXPECT_NODE_ACTIVE(0);
    EXPECT_NODE_ACTIVE(1);
    EXPECT_NODE_ACTIVE(2);
    EXPECT_NODE_ACTIVE(3);
    EXPECT_NODE_ACTIVE(4);
    EXPECT_NODE_ACTIVE(5);
    EXPECT_NODE_ACTIVE(6);
    EXPECT_NODE_ACTIVE(7);
    EXPECT_NODE_ACTIVE(8);
    EXPECT_NODE_ACTIVE(9);
    EXPECT_NODE_ACTIVE(10);
    EXPECT_NODE_ACTIVE(11);
    EXPECT_NODE_ACTIVE(12);

    EXPECT_TRUE(graph->edge(0).is_chain_edge());
    EXPECT_TRUE(graph->edge(1).is_chain_edge());
    EXPECT_TRUE(graph->edge(2).is_unify_edge());
    EXPECT_TRUE(graph->edge(3).is_unify_edge());
    EXPECT_TRUE(graph->edge(4).is_unify_edge());

    EXPECT_EDGE_ACTIVE(0);
    EXPECT_EDGE_ACTIVE(1);
    EXPECT_EDGE_ACTIVE(2);
    EXPECT_EDGE_ACTIVE(3);
    EXPECT_EDGE_ACTIVE(4);

#undef EXPECT_EQ_LIT
#undef EXPECT_EQ_DEPTH
#undef EXPECT_NODE_ACTIVE
#undef EXPECT_EDGE_ACTIVE
}


TEST_F(PhillipTest, Costed)
{
    setup_kb(KB_PATH, "basic", "null", 4.0f);
    insert_implications(
        "(=> (^ (kill-v *e1) (nsubj *e1 u) (dobj *e1 x))"
        "    (^ (die-v *e2) (nsubj *e2 x)))"
        "(=> (^ (hate-v *e1) (nsubj *e1 x) (dobj *e1 y))"
        "    (^ (kill-v *e2) (nsubj *e2 x) (dobj *e2 y)))");
    insert_unification_postponements(
        "(unipp (nsubj * .))"
        "(unipp (dobj * .))");

    set_param("cost_provider", "basic(10.0,-50.0,4.0)");
    setup_phillip("a*", "costed", "gurobi");
    ASSERT_TRUE(check_validity());

    infer(make_input(
        "Test1",
        "(^ (hate-v E1) (nsubj E1 John) (dobj E1 Tom)"
        "   (die-v E2) (nsubj E2 Tom))"));

    const pg::proof_graph_t *graph = get_latent_hypotheses_set();
    const ilp::ilp_problem_t *prob = get_ilp_problem();
    const ilp::ilp_solution_t &sol = get_solutions().front();
    
    ASSERT_EQ(13, graph->nodes().size());
    ASSERT_EQ(ilp::SOLUTION_OPTIMAL, sol.type());

#define EXPECT_EQ_LIT(idx, lit) EXPECT_EQ(lit, graph->node(idx).literal())
#define EXPECT_EQ_DEPTH(idx, dep) EXPECT_EQ(dep, graph->node(idx).depth())
#define EXPECT_NODE_ACTIVE(idx) \
    EXPECT_TRUE(sol.variable_is_active(prob->find_variable_with_node(idx)))
#define EXPECT_EDGE_ACTIVE(idx) \
    EXPECT_TRUE(sol.variable_is_active(prob->find_variable_with_edge(idx)))
    
    EXPECT_EQ_LIT(0, literal_t("hate-v", {"E1"}));
    EXPECT_EQ_LIT(1, literal_t("nsubj", {"E1", "John"}));
    EXPECT_EQ_LIT(2, literal_t("dobj", {"E1", "Tom"}));
    EXPECT_EQ_LIT(3, literal_t("die-v", {"E2"}));
    EXPECT_EQ_LIT(4, literal_t("nsubj", {"E2", "Tom"}));
    EXPECT_EQ_LIT(5, literal_t("kill-v", {"_u1"}));
    EXPECT_EQ_LIT(6, literal_t("nsubj", {"_u1", "_u2"}));
    EXPECT_EQ_LIT(7, literal_t("dobj", {"_u1", "Tom"}));
    EXPECT_EQ_LIT(8, literal_t("hate-v", {"_u3"}));
    EXPECT_EQ_LIT(9, literal_t("nsubj", {"_u3", "_u2"}));
    EXPECT_EQ_LIT(10, literal_t("dobj", {"_u3", "Tom"}));
    EXPECT_EQ_LIT(11, literal_t("=", {"E1", "_u3"}));
    EXPECT_EQ_LIT(12, literal_t("=", {"John", "_u2"}));
    
    EXPECT_EQ_DEPTH(0, 0);
    EXPECT_EQ_DEPTH(1, 0);
    EXPECT_EQ_DEPTH(2, 0);
    EXPECT_EQ_DEPTH(3, 0);
    EXPECT_EQ_DEPTH(4, 0);
    EXPECT_EQ_DEPTH(5, 1);
    EXPECT_EQ_DEPTH(6, 1);
    EXPECT_EQ_DEPTH(7, 1);
    EXPECT_EQ_DEPTH(8, 2);
    EXPECT_EQ_DEPTH(9, 2);
    EXPECT_EQ_DEPTH(10, 2);
    EXPECT_EQ_DEPTH(11, -1);
    EXPECT_EQ_DEPTH(12, -1);

    EXPECT_NODE_ACTIVE(0);
    EXPECT_NODE_ACTIVE(1);
    EXPECT_NODE_ACTIVE(2);
    EXPECT_NODE_ACTIVE(3);
    EXPECT_NODE_ACTIVE(4);
    EXPECT_NODE_ACTIVE(5);
    EXPECT_NODE_ACTIVE(6);
    EXPECT_NODE_ACTIVE(7);
    EXPECT_NODE_ACTIVE(8);
    EXPECT_NODE_ACTIVE(9);
    EXPECT_NODE_ACTIVE(10);
    EXPECT_NODE_ACTIVE(11);
    EXPECT_NODE_ACTIVE(12);

    EXPECT_TRUE(graph->edge(0).is_chain_edge());
    EXPECT_TRUE(graph->edge(1).is_chain_edge());
    EXPECT_TRUE(graph->edge(2).is_unify_edge());
    EXPECT_TRUE(graph->edge(3).is_unify_edge());
    EXPECT_TRUE(graph->edge(4).is_unify_edge());

    EXPECT_EDGE_ACTIVE(0);
    EXPECT_EDGE_ACTIVE(1);
    EXPECT_EDGE_ACTIVE(2);
    EXPECT_EDGE_ACTIVE(3);
    EXPECT_EDGE_ACTIVE(4);

#undef EXPECT_EQ_LIT
#undef EXPECT_EQ_DEPTH
#undef EXPECT_NODE_ACTIVE
#undef EXPECT_EDGE_ACTIVE
}


}
