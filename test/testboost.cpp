/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "tokenize.h"
#include "checkboost.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestBoost : public TestFixture {
public:
    TestBoost() : TestFixture("TestBoost")
    { }

private:
    void run() {
        TEST_CASE(BoostForeachContainerModification)
    }

    void check(const std::string &code) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");
        settings.addEnabled("performance");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code.c_str());
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Check..
        CheckBoost checkBoost;
        checkBoost.runSimplifiedChecks(&tokenizer, &settings, this);
    }

    void BoostForeachContainerModification() {
        check("void f() {\n"
              "    vector<int> data;\n"
              "    BOOST_FOREACH(int i, data) {\n"
              "        data.push_back(123);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) BOOST_FOREACH caches the end() iterator. It's undefined behavior if you modify the container.\n", errout.str());

        check("void f() {\n"
              "    set<int> data;\n"
              "    BOOST_FOREACH(int i, data) {\n"
              "        data.insert(123);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) BOOST_FOREACH caches the end() iterator. It's undefined behavior if you modify the container.\n", errout.str());

        check("void f() {\n"
              "    set<int> data;\n"
              "    BOOST_FOREACH(const int &i, data) {\n"
              "        data.erase(123);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) BOOST_FOREACH caches the end() iterator. It's undefined behavior if you modify the container.\n", errout.str());

        // Check single line usage
        check("void f() {\n"
              "    set<int> data;\n"
              "    BOOST_FOREACH(const int &i, data)\n"
              "        data.clear();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) BOOST_FOREACH caches the end() iterator. It's undefined behavior if you modify the container.\n", errout.str());

        // Container returned as result of a function -> Be quiet
        check("void f() {\n"
              "    BOOST_FOREACH(const int &i, get_data())\n"
              "        data.insert(i);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestBoost)