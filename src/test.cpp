#include <list>
#include "./test.h"

namespace phtest
{

using namespace phil;


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
    const std::string NAME("compiled/kb");
    setup_kb(NAME, "basic", "null");

    EXPECT_EQ(4.0f, kb::knowledge_base_t::get_max_distance());
    EXPECT_EQ(NAME, kb::kb()->filename());
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
    const std::string NAME("compiled/kb");
    setup_kb(NAME, "cost", "null");
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


/** A fixture class for testing inference with Phillip. */
class PhillipTest :
        public phillip_main_t,
        public ::testing::Test
{
protected:
    virtual void SetUp() override
        {
            set_verbose(NOT_VERBOSE);
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

    void set_input(const std::string &name, const std::string &input_str)
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
            
            phillip_main_t::set_input(input);
        }
};


TEST_F(PhillipTest, Test1)
{
    setup_kb("compiled/kb", "basic", "null");
    insert_axioms(
        "(=> (^ (kill-v *e1) (dobj *e1 x)) (^ (die-v *e2) (nsubj *e2 x)))"
        "(=> (^ (hate-v *e1) (nsubj *e1 x) (dobj *e1 y))"
        "    (^ (kill-v *e2) (nsubj *e2 x) (dobj *e2 y)))");
    
    setup_phillip("a*", "weighted", "gurobi");
    set_input(
        "Test1",
        "(^ (hate-v E1) (nsubj E1 John) (dobj E1 Tom)"
        "   (die-v E2) (nsubj E2 Tom))");
    
    ASSERT_TRUE(check_validity());
    ASSERT_EQ(5, get_input()->obs.branches().size());
    
    pg::proof_graph_t *graph = new pg::proof_graph_t(this, "Test1");
    delete graph;
}


}
