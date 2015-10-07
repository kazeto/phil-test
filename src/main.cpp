#include <gtest/gtest.h>
#include <phillip.h>


int main(int argc, char **argv)
{
    // INITIALIZE PHILLIP
    phil::util::initialize();
    phil::phillip_main_t::set_verbose(phil::NOT_VERBOSE);

    // RUN TEST
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
