// variant, a type class that mimics javascript varying behaviours as much as possible.
// - rlyeh, 2013. MIT licensed ~~ listening to Howlin' Rain - Phantom In The Valley.

/* notes to self:
 * uint8  can safely store (u)int8
 * uint16 can safely store (u)int8,(u)int16
 * uint32 can safely store (u)int8,(u)int16,(u)int32
 * uint64 can safely store (u)int8,(u)int16,(u)int32, (u)int64
 * float  can safely store (u)int8,(u)int16,          float
 * double can safely store (u)int8,(u)int16,(u)int32, float,double
 * string can safely store const char *, and string; then uint64 and double (and subsequent types)
 * string is a super-container that stores anything
 * number -> upcasts to -> string -> that downcasts to -> number
 *
 * use .flat() to downcast a copy; .wide() to upcast a copy
 */

/*
 * - todo: replace union with boost::any(); add bool is<T>(), T&get<T>() and support for multiple types, or
 * - todo: replace union with moon9::nmap(); add missing cases, or
 * - todo: replace with kult::var()
 */

#pragma once

#include <cassert>

#include <limits>
#include <map>
#include <sstream>
#include <string>

template<typename number_t, typename string_t = std::string, typename string_alt = std::wstring >
class variant {

public:

    enum types : char {
        IS_NUMBER = 1,
        IS_STRING = 2
    };

    template<typename T>
    variant( const T &t ) : string(new string_t(std::to_string(t))), type(IS_STRING) {
        downcast();
    }
    template<size_t N>
    variant( const char (&str)[N] ) : string(new string_t(str)), type(IS_STRING) {
    }
    variant( const char *str ) : string(new string_t(str ? str : "")), type(IS_STRING) {
    }
    variant( char * const &str ) : string(new string_t(str ? str : "")), type(IS_STRING) {
    }
    variant( const string_t &t ) : string(new string_t(t)), type(IS_STRING) {
    }
    variant( const string_alt &t ) : string(new string_t(t.begin(), t.end())), type(IS_STRING) {
    }

    variant() : number(new number_t(defaults())), type(IS_NUMBER) {
    }
    variant( const number_t &t ) : number(new number_t(t)), type(IS_NUMBER) {
    }
    variant( const std::nullptr_t &t ) : number(new number_t(defaults())), type(IS_NUMBER) {
    }

    variant( const variant &copy ) : type(copy.type) {
        if( type == IS_NUMBER )
        /**/ number = new number_t(*copy.number);
        else string = new string_t(*copy.string);
    }

    ~variant() {
        if( type == IS_NUMBER )
        /**/ delete number, number = 0;
        else delete string, string = 0;
    }

    template<typename T>
    variant &operator=( const T &other ) {
        *this = variant( other );
        return *this;
    }
    variant &operator=( const variant &other ) {
        if( this != &other ) {

            if( type == IS_NUMBER )
            /**/ delete number;
            else delete string;

            type = other.type;
            check();

            if( type == IS_NUMBER )
            /**/ number = new number_t(*other.number);
            else string = new string_t(*other.string);
        }
        return *this;
    }

    bool downcast( number_t &n ) const {    // .flat()
        if( type == IS_NUMBER ) {
            n = *number;
            return true;
        }
        std::stringstream ss(*string);
        if( ss >> n && ss.peek() == EOF ) {
            return true;
        }
        // return default number value on fails
        n = invalid();
        return false;
    }

    bool downcast() {                       // .flat()
        // try to downcast string to number if possible
        if( type == IS_NUMBER ) {
            return true;
        }
        number_t n;
        if( downcast( n ) ) {
            type = IS_NUMBER;
            check();
            delete string;
            number = new number_t(n);
            return true;
        }
        return false;
    }

    bool upcast( string_t &s ) const {      // .wide()
        if( type == IS_STRING ) {
            s = *string;
            return true;
        }
        std::stringstream ss;
        if( ss << *number ) {
            s = ss.str();
            return true;
        }
        return false;
    }

    bool upcast() {                         // .wide()
        // try to upcast number to string if possible
        if( type == IS_STRING ) {
            return true;
        }
        string_t s;
        if( upcast( s ) ) {
            type = IS_STRING;
            check();
            delete number;
            string = new string_t(s);
            return true;
        }
        return false;
    }

    variant flat() const {
        number_t n;
        if( downcast(n) ) {
            return variant(n);
        }
        return *this;
    }
    variant wide() const {
        string_t s;
        if( upcast(s) ) {
            return variant(s);
        }
        return *this;
    }

    string_t str() const {
        string_t s;
        if( upcast(s) ) {
            return s;
        }
        // return default string value on fails
        return string_t();
    }

    number_t num() const {
        number_t n;
        if( downcast(n) ) {
            return n;
        }
        // return default number value on fails
        return invalid();
    }

    types get_type() const {
        return type;
    }

    // logical base
    /* todo:
     * - !
     */

#   define $operator(OPER, ELSE) \
    const bool operator OPER( const variant &t ) const { \
        number_t n; \
        auto &use = ( this->type == IS_STRING ? *this : t ); \
        if( this->type - t.type == 0 || use.downcast(n) ) \
            return str() OPER t.str(); \
        return ELSE; \
    }
    $operator(<, false)
    $operator(>, false)
    $operator(<=,false)
    $operator(>=,false)
    $operator(!=, true)
    $operator(==,false)
#   undef $operator

    // arithmetical base
    /* todo:
     * - ++ x2, -- x2
     * - ~
     */

#   define $operator(OPER, ELSE) \
    variant operator OPER( const variant &other ) const { \
        if( type == IS_NUMBER && other.type == IS_NUMBER ) \
            return variant( (*number) OPER (*other.number) ); \
        return ELSE; \
    } \
    template<typename T> \
    variant& operator OPER##=( const T &other ) { \
        return *this = *this OPER variant(other); \
    } \
    template<typename T> \
    variant operator OPER( const T &other ) const { \
        return *this OPER variant(other); \
    }
    $operator( +, this->str() + other.str() );
    $operator( -, invalid() );
    $operator( *, invalid() );
    $operator( /, invalid() );
    $operator( %, invalid() );
#   undef $operator

    variant operator-() const {
        number_t n;
        return downcast(n) ? -n : invalid();
    }

    const string_t *operator->() const {
        return string;
    }
    string_t *operator->() {
        return string;
    }

    const string_t &operator*() const {
        return *string;
    }
    string_t &operator*() {
        return *string;
    }

    static unsigned tests()
    {
        unsigned exec = 0;

#       ifdef test3
#           define TEST3(a,b,c) exec++, test3( a,b,c )
#       else
#           define TEST3(a,b,c) exec++, assert( (a) b (c) )
#       endif
#       ifdef miss3
#           define MISS3(a,b,c) exec++, miss3( a,b,c )
#       else
#           define MISS3(a,b,c) exec++, assert(! ((a) b (c)) )
#       endif

#       define PASS(a,op,b) \
            TEST3(variant(a),==,a); \
            TEST3(variant(a),==,variant(a)); \
            TEST3(variant(b),==,b); \
            TEST3(variant(b),==,variant(b)); \
            TEST3(variant(a),op,b); \
            TEST3(variant(a),op,variant(b));

#       define FAIL(a,op,b) \
            TEST3(variant(a),==,a); \
            TEST3(variant(a),==,variant(a)); \
            TEST3(variant(b),==,b); \
            TEST3(variant(b),==,variant(b)); \
            MISS3(variant(a),op,b); \
            MISS3(variant(a),op,variant(b));

#       define ALL_PASS(a,op1,b,op2,c) \
            PASS(variant(a) op1 b,op2,c); \
            PASS(variant(a) op1 b,op2,variant(c)); \
            PASS(variant(a) op1 variant(b),op2,c); \
            PASS(variant(a) op1 variant(b),op2,variant(c));

#       define ALL_FAIL(a,op1,b,op2,c) \
            FAIL(variant(a) op1 b,op2,c); \
            FAIL(variant(a) op1 b,op2,variant(c)); \
            FAIL(variant(a) op1 variant(b),op2,c); \
            FAIL(variant(a) op1 variant(b),op2,variant(c));

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

        // arith (like doubles)

        TEST3( -variant(3), ==, -3.0 );
        TEST3( -variant(-3), ==, 3.0 );

        TEST3( variant(3) + 1.0, ==, 4.0 );
        TEST3( variant(3) - 1.0, ==, 2.0 );
        TEST3( variant(3) * 1.0, ==, 3.0 );
        TEST3( variant(3) / 1.0, ==, 3.0 );

        TEST3( variant(3) + 100, ==, 103.0 );
        TEST3( variant(3) + 100.f, ==, 103.0 );
        TEST3( variant(3) + 100.120, ==, 103.120 );
        TEST3( variant(3) + 100.120L, ==, 103.120L );

        TEST3( (variant(300) += 100), ==, 400 );
        TEST3( (variant(300) -= 100), ==, 200 );
        TEST3( (variant(300) *= 100), ==, 30000 );
        TEST3( (variant(300) /= 100), ==, 3 );

        {
            number_t diff = variant(1.00000).num() - 1;
            TEST3( diff * diff, <, 1e-6 ); // test close enough
        }

        TEST3( variant(10), ==, 10 );
        TEST3( variant(10), ==, "10" );
        TEST3( variant("10"), ==, 10 );
        TEST3( variant("10"), ==, "10" );

        TEST3( variant( 10 ) +=  10,  ==,    20  );
        TEST3( variant( 10 ) += "10", ==, "1010" );
        TEST3( variant("10") +=  10,  ==, "1010" );
        TEST3( variant("10") += "10", ==, "1010" );

        TEST3( variant( 10 ).wide() += 10,  ==, "1010" );
        TEST3( variant( 10 ).wide() + "10", ==, "1010" );
        TEST3( variant("10").flat() +  10,  ==,    20  );
        TEST3( variant("10").flat() += 10,  ==,    20  );

        TEST3( variant(10) = variant(), ==, 0 );
        TEST3( variant(10) = variant(""), ==, "" );
        TEST3( variant(10) = variant(10), ==, 10 );
        TEST3( variant(10) = variant("10"), ==, "10" );
        TEST3( variant(10) = variant() + 10, ==, 10 );
        TEST3( variant(10) = variant("") + 10, ==, "10" );
        TEST3( variant(10) = variant(10) + 10, ==, 20 );
        TEST3( variant(10) = variant("10") + 10, ==, "1010" );

        // TESTS FROM CHROME V8 CONSOLE

        // str vs str -> cmpjs
        FAIL( "test",       <=,     "3" );
        PASS( "test",       >=,     "3" );
        FAIL( "test",       <,      "3" );
        PASS( "test",       >,      "3" );
        FAIL( "test",       ==,     "3" );
        PASS( "test",       !=,     "3" );

        PASS( "test123",  <=, "test123" );
        PASS( "test123",  >=, "test123" );
        FAIL( "test123",  <,  "test123" );
        FAIL( "test123",  >,  "test123" );
        PASS( "test123",  ==, "test123" );
        FAIL( "test123",  !=, "test123" );

        PASS( "test123",  <=, "test124" );
        FAIL( "test123",  >=, "test124" );
        PASS( "test123",  <,  "test124" );
        FAIL( "test123",  >,  "test124" );
        FAIL( "test123",  ==, "test124" );
        PASS( "test123",  !=, "test124" );

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
    ALL_FAIL( "3", +, "test", < ,          3  );
    ALL_FAIL( 3, +, "test",   < ,          3  );
    ALL_PASS( 3, +, "test",   ==,     "3test" );
        PASS( "3",            >=,          3  );
        PASS( "test",         ==,      "test" );
        PASS( 3,              ==,          3  );
    ALL_PASS( 3, +, "",       ==,         "3" );
    ALL_PASS( "" ,+, 3,       ==,         "3" );
    ALL_PASS( "x" ,+, 3,      ==,        "x3" );
    ALL_PASS( 3 ,+, "x",      ==,        "3x" );
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
    ALL_PASS( "3" ,+, 3,      ==,        "33" );
        PASS( "3",            ==,          3  );
    ALL_PASS( "3" ,+, 3,      <=,         33  );
    ALL_PASS( "3" ,+, 3,      >=,         33  );

        return exec;

#       undef ALL_PASS
#       undef ALL_FAIL
#       undef FAIL
#       undef PASS
#       undef TEST3
#       undef MISS3
    }

private:

    union {
        number_t *number;
        string_t *string;
    };

    types type;

    void check() const {
        assert( type == IS_NUMBER || type == IS_STRING );
    }

    number_t defaults() const {
        return number_t(double(0));
    }
    number_t invalid() const {
        // return default value on fails
        return std::numeric_limits<double>::quiet_NaN();
    }
};

template<typename number_t, typename string_t, typename string_alt>
static inline std::ostream &operator<<( std::ostream &os, const variant<number_t,string_t,string_alt> &v ){
    return os << v.str(), os;
}

#if 0
#include <type_traits>
static_assert( std::is_pod< variant<double> >::value == true, "oops!" );
static_assert( std::is_trivial< variant<double> >::value == true, "oops!" );
#endif
