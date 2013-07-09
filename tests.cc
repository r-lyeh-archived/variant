#include <math.h>
#include <cmath>

#include <iostream>

#include "variant.hpp"
#include "petitsuite/petitsuite.hpp"

namespace {
    template<typename T>
    T abs(T a) {
        return a < 0 ? -a : a;
    }

    template<typename T, typename U>
    bool close(T a, U b) {
        return abs(b)-abs(a) < 1e-6 ? true : false;
    }

    template<typename variant>
    void tests() {

        // COMPILATION TESTS. THESE SHOULD COMPILE

        variant(nullptr);
        variant('a');
        variant(true);
        variant(100);
        variant(100L);
        variant(100LL);
        variant(100U);
        variant(100LU);
    //      variant(100LLU);
        variant(2.95f);
        variant(3.14159);
        variant(3.14159L);
        variant("hello");
        variant(std::string("hello"));

        variant("a") <= 3;
        variant("a") <= variant(3);

        test3( std::string("a"), <, std::string("b") );
        test3( std::string("b1"), <, std::string("b2") );
        test3( std::string("b10"), <, std::string("b100") );


        // arith (like doubles)

        test3( -variant(3), ==, -3.0 );
        test3( -variant(-3), ==, 3.0 );

        test3( variant(3) + 1.0, ==, 4.0 );
        test3( variant(3) - 1.0, ==, 2.0 );
        test3( variant(3) * 1.0, ==, 3.0 );
        test3( variant(3) / 1.0, ==, 3.0 );

        test3( variant(3) + 100, ==, 103.0 );
        test3( variant(3) + 100.f, ==, 103.0 );
        test3( variant(3) + 100.120, ==, 103.120 );
        test3( variant(3) + 100.120L, ==, 103.120L );

        test3( (variant(300) += 100), ==, 400 );
        test3( (variant(300) -= 100), ==, 200 );
        test3( (variant(300) *= 100), ==, 30000 );
        test3( (variant(300) /= 100), ==, 3 );

        test1( close(1,cos(variant(0).num())) );



        #define PASS(a,op,b) \
            test4(variant(a),==,variant(a), "equality test A"); \
            test4(variant(b),==,variant(b), "equality test B"); \
            test4(variant(a),op,b, "ind(a) over b"); \
            /*miss3(a,op,variant(b));*/ \
            test4(variant(a),op,variant(b), "ind(a) over ind(b)");

        #define FAIL(a,op,b) \
            miss4(variant(a),op,b, "ind(a) over b"); \
            /*test3(a,op,variant(b));*/ \
            miss4(variant(a),op,variant(b), "ind(a) over ind(b)");

        #define COMBPASS(a,op1,b,op2,c) \
            PASS(variant(a) op1 b,op2,c); \
            PASS(variant(a) op1 b,op2,variant(c)); \
            PASS(variant(a) op1 variant(b),op2,c); \
            PASS(variant(a) op1 variant(b),op2,variant(c));

        #define COMBFAIL(a,op1,b,op2,c) \
            FAIL(variant(a) op1 b,op2,c); \
            FAIL(variant(a) op1 b,op2,variant(c)); \
            FAIL(variant(a) op1 variant(b),op2,c); \
            FAIL(variant(a) op1 variant(b),op2,variant(c));

            // TESTS FROM CHROME V8 CONSOLE

            // str vs str -> cmpjs
            FAIL( "test",       <=,     "3" );
            PASS( "test",       >=,     "3" );
            FAIL( "test",       <,      "3" );
            PASS( "test",       >,      "3" );
            FAIL( "test",       ==,     "3" );
            PASS( "test",       !=,     "3" );

            // str vs num -> != only
            FAIL( "3t",         <=,     3 );
            FAIL( "3t",         >=,     3 );
            FAIL( "3t",         <,      3 );
            FAIL( "3t",         >,      3 );
            FAIL( "3t",         ==,     3 );
            PASS( "3t",         !=,     3 );

            FAIL( "test",       <=,     3 );
            FAIL( "test",       >=,     3 );
            FAIL( "test",       <,      3 );
            FAIL( "test",       >,      3 );
            FAIL( "test",       ==,     3 );
            PASS( "test",       !=,     3 );

            // cast vs num -> cmpjs probablemente
            PASS( "3",          <=,     3 );
            PASS( "3",          >=,     3 );
            FAIL( "3",          <,      3 );
            FAIL( "3",          >,      3 );
            PASS( "3",          ==,     3 );
            FAIL( "3",          !=,     3 );

            PASS( "",          <=,     "" );
            PASS( "",          >=,     "" );
            FAIL( "",          <,      "" );
            FAIL( "",          >,      "" );
            PASS( "",          ==,     "" );
            FAIL( "",          !=,     "" );



            FAIL( "3test",        <=,          3  );
            FAIL( "3test",        < ,          3  );
        COMBFAIL( "3", +, "test", < ,          3  );
        COMBFAIL( 3, +, "test",   < ,          3  );
        COMBPASS( 3, +, "test",   ==,     "3test" );
            PASS( "3",            >=,          3  );
            PASS( "test",         ==,      "test" );
            PASS( 3,              ==,          3  );
        COMBPASS( 3, +, "",       ==,         "3" );
        COMBPASS( "" ,+, 3,       ==,         "3" );
        COMBPASS( "x" ,+, 3,      ==,        "x3" );
        COMBPASS( 3 ,+, "x",      ==,        "3x" );
            PASS( "test",         <=,      "test" );
            PASS( "test",         >=,      "test" );
            PASS( "test",         < ,     "test1" );
            PASS( "test2",        >=,     "test1" );
            PASS( "test-2",       < ,     "test1" );
            PASS( "test+2",       < ,     "test1" );
            PASS( "test2",        >=,     "test1" );
            PASS( "test2",        >=,     "test1" );
            PASS( "test0",        < ,     "test1" );
            PASS( "test01",       < ,     "test1" );
        COMBPASS( "3" ,+, 3,      ==,        "33" );
            PASS( "3",            ==,          3  );
        COMBPASS( "3" ,+, 3,      <=,         33  );
        COMBPASS( "3" ,+, 3,      >=,         33  );
    }
}

int main() {
    typedef variant<double,std::string> var;

    tests<var>();
    std::cout << sizeof(var) << std::endl;

    return 0;
}
