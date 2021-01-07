/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */

/*
 * This file is an unpleasant artifact of breaking backward compatibility with framework1.
 *
 * The purpose is to act as a wrapper around the names in framework1 and framework2 so that,
 * to the unit test, using framework2 is as simple as adding a single #define FRAMEWORK2,
 * and anything without that define will default to framework1.
 */

#if API_VER == 2
    #include "framework2.h"
    using Tester = framework2::Tester;

    void setup(Tester &);
    void shutdown(void);
    void testBringup(lc3::sim &);
    void testTeardown(lc3::sim &);

    int main(int argc, char * argv[])
    {
        framework2::setup = setup;
        framework2::shutdown = shutdown;
        framework2::testBringup = testBringup;
        framework2::testTeardown = testTeardown;

        framework2::main(argc, argv);
    }
#else
    #include "framework1.h"

    void setup(void);
    void shutdown(void);
    void testBringup(lc3::sim &);
    void testTeardown(lc3::sim &);

    int main(int argc, char * argv[])
    {
        std::cout << "\033[1;31m";
        std::cout << "********************************************************************************\n";
        std::cout << "*                             Deprecation Notice                               *\n";
        std::cout << "*                                                                              *\n";
        std::cout << "* Version 1 of the API is deprecated and no longer supported.  Please use the  *\n";
        std::cout << "* guide at the following URL to update this unit test:                         *\n";
        std::cout << "*   https://github.com/chiragsakhuja/lc3tools/blob/master/docs/UPGRADE1.md     *\n";
        std::cout << "*                                                                              *\n";
        std::cout << "********************************************************************************\n";
        std::cout << "\033[0m";

        framework1::setup = setup;
        framework1::shutdown = shutdown;
        framework1::testBringup = testBringup;
        framework1::testTeardown = testTeardown;

        framework1::main(argc, argv);
    }
#endif
