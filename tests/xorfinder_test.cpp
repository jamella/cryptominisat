/*
 * CryptoMiniSat
 *
 * Copyright (c) 2009-2015, Mate Soos. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation
 * version 2.0 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
*/

#include "gtest/gtest.h"

#include <fstream>

#include "src/solver.h"
#include "src/xorfinder.h"
#include "src/solverconf.h"
#include "src/occsimplifier.h"
using namespace CMSat;
#include "test_helper.h"

struct xor_finder : public ::testing::Test {
    xor_finder()
    {
        must_inter.store(false, std::memory_order_relaxed);
        SolverConf conf;
        conf.doCache = false;
        s = new Solver(&conf, &must_inter);
        s->new_vars(30);
        occsimp = s->occsimplifier;
    }
    ~xor_finder()
    {
        delete s;
    }
    Solver* s = NULL;
    OccSimplifier* occsimp = NULL;
    std::atomic<bool> must_inter;
};

TEST_F(xor_finder, find_none)
{
    s->add_clause_outer(str_to_cl("1, 2"));
    s->add_clause_outer(str_to_cl("-1, -2"));

    occsimp->setup();
    XorFinder finder(occsimp, s);
    finder.find_xors();
    EXPECT_EQ(finder.xors.size(), 0U);
}

TEST_F(xor_finder, find_tri_1)
{
    s->add_clause_outer(str_to_cl("1, 2, 3"));
    s->add_clause_outer(str_to_cl("-1, -2, 3"));
    s->add_clause_outer(str_to_cl("-1, 2, -3"));
    s->add_clause_outer(str_to_cl("1, -2, -3"));

    occsimp->setup();
    XorFinder finder(occsimp, s);
    finder.find_xors();
    check_xors_contains(finder.xors, "1, 2, 3 = 1");
}

TEST_F(xor_finder, find_tri_2)
{
    s->add_clause_outer(str_to_cl("-1, 2, 3"));
    s->add_clause_outer(str_to_cl("1, -2, 3"));
    s->add_clause_outer(str_to_cl("1, 2, -3"));
    s->add_clause_outer(str_to_cl("-1, -2, -3"));

    occsimp->setup();
    XorFinder finder(occsimp, s);
    finder.find_xors();
    check_xors_eq(finder.xors, "1, 2, 3 = 0");
}

TEST_F(xor_finder, find_tri_3)
{
    s->add_clause_outer(str_to_cl("-1, 2, 3"));
    s->add_clause_outer(str_to_cl("1, -2, 3"));
    s->add_clause_outer(str_to_cl("1, 2, -3"));
    s->add_clause_outer(str_to_cl("-1, -2, -3"));

    s->add_clause_outer(str_to_cl("1, 2, 3"));
    s->add_clause_outer(str_to_cl("-1, -2, 3"));
    s->add_clause_outer(str_to_cl("-1, 2, -3"));
    s->add_clause_outer(str_to_cl("1, -2, -3"));

    occsimp->setup();
    XorFinder finder(occsimp, s);
    finder.find_xors();
    check_xors_contains(finder.xors, "1, 2, 3 = 0");
    check_xors_contains(finder.xors, "1, 2, 3 = 1");
}


TEST_F(xor_finder, find_4_1)
{
    s->add_clause_outer(str_to_cl("-1, 2, 3, 4"));
    s->add_clause_outer(str_to_cl("1, -2, 3, 4"));
    s->add_clause_outer(str_to_cl("1, 2, -3, 4"));
    s->add_clause_outer(str_to_cl("1, 2, 3, -4"));

    s->add_clause_outer(str_to_cl("1, -2, -3, -4"));
    s->add_clause_outer(str_to_cl("-1, 2, -3, -4"));
    s->add_clause_outer(str_to_cl("-1, -2, 3, -4"));
    s->add_clause_outer(str_to_cl("-1, -2, -3, 4"));

    occsimp->setup();
    XorFinder finder(occsimp, s);
    finder.find_xors();
    check_xors_eq(finder.xors, "1, 2, 3, 4 = 0;");
}

TEST_F(xor_finder, find_4_4)
{
    s->add_clause_outer(str_to_cl("-1, -2, 3, 4"));
    s->add_clause_outer(str_to_cl("1, -2, -3, 4"));
    s->add_clause_outer(str_to_cl("1, 2, -3, -4"));
    s->add_clause_outer(str_to_cl("-1, 2,  -3, 4"));
    s->add_clause_outer(str_to_cl("-1, 2,  3, -4"));
    s->add_clause_outer(str_to_cl("1, -2,  3, -4"));
    s->add_clause_outer(str_to_cl("-1, -2, -3, -4"));
    s->add_clause_outer(str_to_cl("1, 2, 3, 4"));

    occsimp->setup();
    XorFinder finder(occsimp, s);
    finder.find_xors();
    check_xors_eq(finder.xors, "1, 2, 3, 4 = 1");
}

/*
 * These tests only work if the matching is non-exact
 * i.e. if size is not checked for equality
 *
TEST_F(xor_finder, find_4_2)
{
    s->add_clause_outer(str_to_cl("-1, 2, 3, 4"));
    s->add_clause_outer(str_to_cl("1, -2, 3, 4"));
    s->add_clause_outer(str_to_cl("1, 2, -3, 4"));
    s->add_clause_outer(str_to_cl("1, 2, 3"));

    s->add_clause_outer(str_to_cl("1, -2, -3, -4"));
    s->add_clause_outer(str_to_cl("-1, 2, -3, -4"));
    s->add_clause_outer(str_to_cl("-1, -2, 3, -4"));
    s->add_clause_outer(str_to_cl("-1, -2, -3, 4"));

    occsimp->setup();
    XorFinder finder(occsimp, s);
    finder.find_xors();
    check_xors_eq(finder.xors, "1, 2, 3, 4 = 0;");
}

TEST_F(xor_finder, find_4_3)
{
    s->add_clause_outer(str_to_cl("-1, 2, 3, 4"));
    s->add_clause_outer(str_to_cl("1, -2, 3, 4"));
    s->add_clause_outer(str_to_cl("1, 2, -3, 4"));
    s->add_clause_outer(str_to_cl("1, 2, 3"));

    s->add_clause_outer(str_to_cl("1, -2, -3, -4"));
    s->add_clause_outer(str_to_cl("-1, 2, -3, -4"));
    s->add_clause_outer(str_to_cl("-1, -2, 3, -4"));
    s->add_clause_outer(str_to_cl("-1, -3, 4"));

    occsimp->setup();
    XorFinder finder(occsimp, s);
    finder.find_xors();
    check_xors_eq(finder.xors, "1, 2, 3, 4 = 0;");
}

TEST_F(xor_finder, find_5_2)
{
    s->add_clause_outer(str_to_cl("-1, -2, 3, 4, 5"));
    s->add_clause_outer(str_to_cl("-1, 2, -3"));
    s->add_clause_outer(str_to_cl("-1, 2, 3"));

    s->add_clause_outer(str_to_cl("1, -2, -3, 4, 5"));
    s->add_clause_outer(str_to_cl("1, -2, 3, -4, 5"));
    s->add_clause_outer(str_to_cl("1, -2, 3, 4, -5"));

    s->add_clause_outer(str_to_cl("1, 2, -3, -4, 5"));
    s->add_clause_outer(str_to_cl("1, 2, -3, 4, -5"));

    s->add_clause_outer(str_to_cl("1, 2, 3, -4, -5"));

    //

    s->add_clause_outer(str_to_cl("1, -2, -3, -4, -5"));
    s->add_clause_outer(str_to_cl("-1, 2, -3, -4, -5"));
    s->add_clause_outer(str_to_cl("-1, -2, 3, -4, -5"));
    s->add_clause_outer(str_to_cl("-1, -2, -3, 4, -5"));
    s->add_clause_outer(str_to_cl("-1, -2, -3, -4, 5"));

    s->add_clause_outer(str_to_cl("1, 2, 3, 4, 5"));

    occsimp->setup();
    XorFinder finder(occsimp, s);
    finder.find_xors();
    check_xors_eq(finder.xors, "1, 2, 3, 4, 5 = 1;");
}

TEST_F(xor_finder, find_4_5)
{
    s->add_clause_outer(str_to_cl("-1, -2, 3, 4"));
    s->add_clause_outer(str_to_cl("1, -2, -3, 4"));
    s->add_clause_outer(str_to_cl("1, 2, -3, -4"));
    s->add_clause_outer(str_to_cl("-1, 2,  -3, 4"));
    s->add_clause_outer(str_to_cl("-1, 2,  3, -4"));
    s->add_clause_outer(str_to_cl("1, -2,  3, -4"));
    s->add_clause_outer(str_to_cl("-1, -2, -3, -4"));
    s->add_clause_outer(str_to_cl("1, 2, 3"));

    s->add_clause_outer(str_to_cl("-1, 2, 3, 4"));
    s->add_clause_outer(str_to_cl("1, -2, 3, 4"));
    s->add_clause_outer(str_to_cl("1, 2, -3, 4"));
    s->add_clause_outer(str_to_cl("1, 2, 3"));

    s->add_clause_outer(str_to_cl("1, -2, -3, -4"));
    s->add_clause_outer(str_to_cl("-1, 2, -3, -4"));
    s->add_clause_outer(str_to_cl("-1, -2, 3, -4"));
    s->add_clause_outer(str_to_cl("-1, -3, 4"));

    occsimp->setup();
    XorFinder finder(occsimp, s);
    finder.find_xors();
    check_xors_eq(finder.xors, "1, 2, 3, 4 = 1; 1, 2, 3, 4 = 0");
}
*/

TEST_F(xor_finder, find_5_1)
{
    s->add_clause_outer(str_to_cl("-1, -2, 3, 4, 5"));
    s->add_clause_outer(str_to_cl("-1, 2, -3, 4, 5"));
    s->add_clause_outer(str_to_cl("-1, 2, 3, -4, 5"));
    s->add_clause_outer(str_to_cl("-1, 2, 3, 4, -5"));

    s->add_clause_outer(str_to_cl("1, -2, -3, 4, 5"));
    s->add_clause_outer(str_to_cl("1, -2, 3, -4, 5"));
    s->add_clause_outer(str_to_cl("1, -2, 3, 4, -5"));

    s->add_clause_outer(str_to_cl("1, 2, -3, -4, 5"));
    s->add_clause_outer(str_to_cl("1, 2, -3, 4, -5"));

    s->add_clause_outer(str_to_cl("1, 2, 3, -4, -5"));

    //

    s->add_clause_outer(str_to_cl("1, -2, -3, -4, -5"));
    s->add_clause_outer(str_to_cl("-1, 2, -3, -4, -5"));
    s->add_clause_outer(str_to_cl("-1, -2, 3, -4, -5"));
    s->add_clause_outer(str_to_cl("-1, -2, -3, 4, -5"));
    s->add_clause_outer(str_to_cl("-1, -2, -3, -4, 5"));

    s->add_clause_outer(str_to_cl("1, 2, 3, 4, 5"));

    occsimp->setup();
    XorFinder finder(occsimp, s);
    finder.find_xors();
    check_xors_eq(finder.xors, "1, 2, 3, 4, 5 = 1;");
}

TEST_F(xor_finder, find_6_0)
{
    s->add_clause_outer(str_to_cl("1, -7, -3, -4, -5, -9"));
    s->add_clause_outer(str_to_cl("-1, 7, -3, -4, -5, -9"));
    s->add_clause_outer(str_to_cl("-1, -7, 3, -4, -5, -9"));
    s->add_clause_outer(str_to_cl("1, 7, 3, -4, -5, -9"));
    s->add_clause_outer(str_to_cl("-1, -7, -3, 4, -5, -9"));
    s->add_clause_outer(str_to_cl("1, 7, -3, 4, -5, -9"));
    s->add_clause_outer(str_to_cl("1, -7, 3, 4, -5, -9"));
    s->add_clause_outer(str_to_cl("-1, 7, 3, 4, -5, -9"));
    s->add_clause_outer(str_to_cl("-1, -7, -3, -4, 5, -9"));
    s->add_clause_outer(str_to_cl("1, 7, -3, -4, 5, -9"));
    s->add_clause_outer(str_to_cl("1, -7, 3, -4, 5, -9"));
    s->add_clause_outer(str_to_cl("-1, 7, 3, -4, 5, -9"));
    s->add_clause_outer(str_to_cl("1, -7, -3, 4, 5, -9"));
    s->add_clause_outer(str_to_cl("-1, 7, -3, 4, 5, -9"));
    s->add_clause_outer(str_to_cl("-1, -7, 3, 4, 5, -9"));
    s->add_clause_outer(str_to_cl("1, 7, 3, 4, 5, -9"));
    s->add_clause_outer(str_to_cl("-1, -7, -3, -4, -5, 9"));
    s->add_clause_outer(str_to_cl("1, 7, -3, -4, -5, 9"));
    s->add_clause_outer(str_to_cl("1, -7, 3, -4, -5, 9"));
    s->add_clause_outer(str_to_cl("-1, 7, 3, -4, -5, 9"));
    s->add_clause_outer(str_to_cl("1, -7, -3, 4, -5, 9"));
    s->add_clause_outer(str_to_cl("-1, 7, -3, 4, -5, 9"));
    s->add_clause_outer(str_to_cl("-1, -7, 3, 4, -5, 9"));
    s->add_clause_outer(str_to_cl("1, 7, 3, 4, -5, 9"));
    s->add_clause_outer(str_to_cl("1, -7, -3, -4, 5, 9"));
    s->add_clause_outer(str_to_cl("-1, 7, -3, -4, 5, 9"));
    s->add_clause_outer(str_to_cl("-1, -7, 3, -4, 5, 9"));
    s->add_clause_outer(str_to_cl("1, 7, 3, -4, 5, 9"));
    s->add_clause_outer(str_to_cl("-1, -7, -3, 4, 5, 9"));
    s->add_clause_outer(str_to_cl("1, 7, -3, 4, 5, 9"));
    s->add_clause_outer(str_to_cl("1, -7, 3, 4, 5, 9"));
    s->add_clause_outer(str_to_cl("-1, 7, 3, 4, 5, 9"));

    occsimp->setup();
    XorFinder finder(occsimp, s);
    finder.find_xors();
    check_xors_eq(finder.xors, "1, 7, 3, 4, 5, 9 = 0;");
}

TEST_F(xor_finder, find_6_1)
{
    s->add_clause_outer(str_to_cl("-6, -7, -3, -4, -5, -9"));
    s->add_clause_outer(str_to_cl("6, 7, -3, -4, -5, -9"));
    s->add_clause_outer(str_to_cl("6, -7, 3, -4, -5, -9"));
    s->add_clause_outer(str_to_cl("-6, 7, 3, -4, -5, -9"));
    s->add_clause_outer(str_to_cl("6, -7, -3, 4, -5, -9"));
    s->add_clause_outer(str_to_cl("-6, 7, -3, 4, -5, -9"));
    s->add_clause_outer(str_to_cl("-6, -7, 3, 4, -5, -9"));
    s->add_clause_outer(str_to_cl("6, 7, 3, 4, -5, -9"));
    s->add_clause_outer(str_to_cl("6, -7, -3, -4, 5, -9"));
    s->add_clause_outer(str_to_cl("-6, 7, -3, -4, 5, -9"));
    s->add_clause_outer(str_to_cl("-6, -7, 3, -4, 5, -9"));
    s->add_clause_outer(str_to_cl("6, 7, 3, -4, 5, -9"));
    s->add_clause_outer(str_to_cl("-6, -7, -3, 4, 5, -9"));
    s->add_clause_outer(str_to_cl("6, 7, -3, 4, 5, -9"));
    s->add_clause_outer(str_to_cl("6, -7, 3, 4, 5, -9"));
    s->add_clause_outer(str_to_cl("-6, 7, 3, 4, 5, -9"));
    s->add_clause_outer(str_to_cl("6, -7, -3, -4, -5, 9"));
    s->add_clause_outer(str_to_cl("-6, 7, -3, -4, -5, 9"));
    s->add_clause_outer(str_to_cl("-6, -7, 3, -4, -5, 9"));
    s->add_clause_outer(str_to_cl("6, 7, 3, -4, -5, 9"));
    s->add_clause_outer(str_to_cl("-6, -7, -3, 4, -5, 9"));
    s->add_clause_outer(str_to_cl("6, 7, -3, 4, -5, 9"));
    s->add_clause_outer(str_to_cl("6, -7, 3, 4, -5, 9"));
    s->add_clause_outer(str_to_cl("-6, 7, 3, 4, -5, 9"));
    s->add_clause_outer(str_to_cl("-6, -7, -3, -4, 5, 9"));
    s->add_clause_outer(str_to_cl("6, 7, -3, -4, 5, 9"));
    s->add_clause_outer(str_to_cl("6, -7, 3, -4, 5, 9"));
    s->add_clause_outer(str_to_cl("-6, 7, 3, -4, 5, 9"));
    s->add_clause_outer(str_to_cl("6, -7, -3, 4, 5, 9"));
    s->add_clause_outer(str_to_cl("-6, 7, -3, 4, 5, 9"));
    s->add_clause_outer(str_to_cl("-6, -7, 3, 4, 5, 9"));
    s->add_clause_outer(str_to_cl("6, 7, 3, 4, 5, 9"));

    occsimp->setup();
    XorFinder finder(occsimp, s);
    finder.find_xors();
    check_xors_eq(finder.xors, "6, 7, 3, 4, 5, 9 = 1;");
}


TEST_F(xor_finder, clean_v1)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2, 3 = 0;");
    finder.clean_up_xors();
    EXPECT_EQ(finder.xors.size(), 0u);
}

TEST_F(xor_finder, clean_v2)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2, 3 = 0; 1, 4, 5, 6 = 0");
    finder.clean_up_xors();
    EXPECT_EQ(finder.xors.size(), 2u);
}

TEST_F(xor_finder, clean_v3)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2, 3 = 0; 1, 4, 5, 6 = 0; 10, 11, 12, 13 = 1");
    finder.clean_up_xors();
    EXPECT_EQ(finder.xors.size(), 2u);
}

TEST_F(xor_finder, clean_v4)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2, 3 = 0; 1, 4, 5, 6 = 0; 10, 11, 12, 13 = 1; 10, 15, 16, 17 = 0");
    finder.clean_up_xors();
    EXPECT_EQ(finder.xors.size(), 4u);
}

TEST_F(xor_finder, xor_1)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2, 3 = 1; 1, 4, 5, 6 = 0;");
    finder.xor_together_xors();
    check_xors_eq(finder.xors, "2, 3, 4, 5, 6 = 1;");
}

TEST_F(xor_finder, xor_2)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2, 3 = 0; 1, 4, 5, 6 = 0;");
    finder.xor_together_xors();
    check_xors_eq(finder.xors, "2, 3, 4, 5, 6 = 0;");
}

TEST_F(xor_finder, xor_3)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2, 3 = 0; 10, 4, 5, 6 = 0;");
    finder.xor_together_xors();
    check_xors_eq(finder.xors, "1, 2, 3 = 0; 10, 4, 5, 6 = 0;");
}

TEST_F(xor_finder, xor_4)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2, 3 = 0; 1, 4, 5, 6 = 0;"
        "1, 9, 10, 11 = 0;");
    finder.xor_together_xors();
    EXPECT_EQ(finder.xors.size(), 3u);
}

TEST_F(xor_finder, xor_5)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2, 3 = 0; 1, 4, 5, 6 = 0;"
        "1, 4, 10, 11 = 0;");
    finder.xor_together_xors();
    EXPECT_EQ(finder.xors.size(), 2u);
    check_xors_contains(finder.xors, "5, 6, 10, 11 = 0");
}

TEST_F(xor_finder, xor_6)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2 = 0; 1, 4= 0;"
        "6, 7 = 0; 6, 10 = 1");
    finder.xor_together_xors();
    EXPECT_EQ(finder.xors.size(), 2u);
    check_xors_eq(finder.xors, "2, 4 = 0; 7, 10 = 1");
}

TEST_F(xor_finder, xor_7)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2 = 0; 1, 2= 0;");
    finder.xor_together_xors();
    EXPECT_EQ(finder.xors.size(), 0u);
}

TEST_F(xor_finder, xor_8)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2 = 0; 1, 2 = 1;");
    finder.xor_together_xors();
    bool ret = finder.add_new_truths_from_xors();
    EXPECT_FALSE(ret);
}

TEST_F(xor_finder, xor_unit)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2 = 0; 1, 2, 3 = 1;");
    finder.xor_together_xors();
    bool ret = finder.add_new_truths_from_xors();
    EXPECT_TRUE(ret);
    EXPECT_EQ(finder.xors.size(), 0u);
}

TEST_F(xor_finder, xor_unit2)
{
    XorFinder finder(occsimp, s);
    s->add_clause_outer(str_to_cl("-3"));
    finder.xors = str_to_xors("1, 2 = 0; 1, 2, 3 = 1;");
    finder.xor_together_xors();
    bool ret = finder.add_new_truths_from_xors();
    EXPECT_FALSE(ret);
}

TEST_F(xor_finder, xor_binx)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2, 5 = 0; 1, 2, 3 = 0;");
    finder.xor_together_xors();
    bool ret = finder.add_new_truths_from_xors();
    EXPECT_TRUE(ret);
    EXPECT_EQ(finder.xors.size(), 0u);
    check_red_cls_eq(s, "5, -3; -5, 3");
}

TEST_F(xor_finder, xor_binx_inv)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2, 5 = 1; 1, 2, 3 = 0;");
    finder.xor_together_xors();
    bool ret = finder.add_new_truths_from_xors();
    EXPECT_TRUE(ret);
    EXPECT_EQ(finder.xors.size(), 0u);
    check_red_cls_eq(s, "-5, -3; 5, 3");
}

TEST_F(xor_finder, xor_binx_inv2)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2, 5 = 1; 1, 2, 3 = 1;");
    finder.xor_together_xors();
    bool ret = finder.add_new_truths_from_xors();
    EXPECT_TRUE(ret);
    EXPECT_EQ(finder.xors.size(), 0u);
    check_red_cls_eq(s, "5, -3; -5, 3");
}

TEST_F(xor_finder, xor_binx2_recur)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("1, 2, 5 = 0; 2, 3, 4, 5 = 0; 1, 4, 5 = 0;");
    finder.xor_together_xors();
    bool ret = finder.add_new_truths_from_xors();
    EXPECT_TRUE(ret);
    EXPECT_EQ(finder.xors.size(), 0u);
    check_red_cls_eq(s, "5, -3; -5, 3");
}

TEST_F(xor_finder, xor_binx3_recur)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("8, 9, 2 = 1; 8, 9, 1, 5 = 1; 2, 3, 4, 5 = 0; 1, 4, 5 = 0;");
    finder.xor_together_xors();
    bool ret = finder.add_new_truths_from_xors();
    EXPECT_TRUE(ret);
    finder.xor_together_xors();
    ret = finder.add_new_truths_from_xors();
    EXPECT_TRUE(ret);
    EXPECT_EQ(finder.xors.size(), 0u);
    check_red_cls_eq(s, "5, -3; -5, 3");
}

TEST_F(xor_finder, xor_recur_bug)
{
    XorFinder finder(occsimp, s);
    finder.xors = str_to_xors("3, 7, 9 = 0; 1, 3, 4, 5 = 1;");
    finder.xor_together_xors();
    check_xors_eq(finder.xors, "7, 9 , 1, 4, 5 = 1;");
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
