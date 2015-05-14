#include <list>
#include <gtest/gtest.h>
#include <phillip.h>
#include <binary.h>


namespace phtest
{

using namespace phil;


TEST(UtilityClasses, StringHash)
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


TEST(UtilityClasses, Literal)
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


TEST(CompileKB, SetUp)
{
    kb::knowledge_base_t::setup("compiled/kb", 4.0f, 1, false);
    kb::kb()->set_distance_provider("basic");
    kb::kb()->set_category_table("null");

    EXPECT_EQ(4.0f, kb::knowledge_base_t::get_max_distance());
    EXPECT_EQ("test-kb", kb::kb()->filename());
}


TEST(CompileKB, Compile)
{
    std::string imp_str =
        "(=> (p1 x) (p2 x))"
        "(=> (p4 x) (^ (p2 *x) (p3 *x)))";
    std::list<lf::logical_function_t> imp_lf;

    lf::parse(imp_str, &imp_lf);
    kb::kb()->prepare_compile();
    ASSERT_TRUE(kb::kb()->is_writable());

    int n(0);
    for (auto imp : imp_lf)
    {
        std::string name = format("imp_%d", n++);
        kb::kb()->insert_implication(imp, name);
    }
    EXPECT_EQ(imp_lf.size(), kb::kb()->num_of_axioms());

    kb::kb()->finalize();
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

    void set_input(const std::string &name, const std::string &obs, const std::string &req)
        {
            std::list<lf::logical_function_t> obs_lf, req_lf;
            lf::parse(obs, &obs_lf);
            lf::parse(req, &req_lf);

            lf::input_t input;
            input.name = name;
            input.obs = obs_lf.front();
            if (not req_lf.empty())
                input.req = req_lf.front();

            phillip_main_t::set_input(input);
        }
};


TEST_F(PhillipTest, Test1)
{
    set_lhs_enumerator(bin::lhs_enumerator_library_t::instance()->generate("a*", this));
    set_ilp_convertor(bin::ilp_converter_library_t::instance()->generate("weighted", this));
    set_ilp_solver(bin::ilp_solver_library_t::instance()->generate("gurobi", this));
    set_input("Test1", "(^ (p1 x) (p2 y))", "");
    
    ASSERT_TRUE(check_validity());
    ASSERT_EQ(2, get_input()->obs.branches().size());
    
    pg::proof_graph_t *graph = new pg::proof_graph_t(this, "Test1");
    delete graph;
}


}
