#pragma once

#include <gtest/gtest.h>
#include <phillip.h>
#include <binary.h>


namespace phtest
{

using namespace phil;


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
    virtual void SetUp() override;

    void prepare_compile(
        const std::string &key_dist,
        const std::string &key_tab);

    void prepare_infer(
        const std::string &key_lhs,
        const std::string &key_ilp,
        const std::string &key_sol);

    void write(const std::string &prefix, unsigned char flags = 0x0f);
};


int insert_implications(const std::string &str);
int insert_inconsistencies(const std::string &str);
int insert_unification_postponements(const std::string &str);

lf::input_t make_input(const std::string &name, const std::string &input_str);

int count_observation_nodes(const pg::proof_graph_t *graph);
int count_hypothesis_nodes(const pg::proof_graph_t *graph);
int count_unification_nodes(const pg::proof_graph_t *graph);

int count_active_nodes(const ilp::ilp_solution_t &sol);
int count_active_observation_nodes(const ilp::ilp_solution_t &sol);
int count_active_hypothesis_nodes(const ilp::ilp_solution_t &sol);
int count_active_unification_nodes(const ilp::ilp_solution_t &sol);

int count_chaining_edges(const pg::proof_graph_t *graph);
int count_unifying_edges(const pg::proof_graph_t *graph);

int count_active_edges(const ilp::ilp_solution_t &sol);
int count_active_chaining_edges(const ilp::ilp_solution_t &sol);
int count_active_unifying_edges(const ilp::ilp_solution_t &sol);

bool exists_in(arity_t arity, const ilp::ilp_solution_t &sol);


extern const std::string KB_PATH;

}
