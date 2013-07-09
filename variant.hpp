// variant type class that tries to mimic javascript behaviour as much as possible.
// - rlyeh, 2013. MIT licensed ~~ listening to Howlin' Rain - Phantom In The Valley

/* notes to self:
 * uint64 can hold safely int64, uint64
 * double can hold safely int8,uint8,int16,uint16,int32,uint32,float,double
 * string can hold safely const char *, and string
 * variant wont be POD unless both number_t and string_t members are pointers (@todo: implement it)
 */

#pragma once

#include <cassert>

#include <limits>
#include <map>
#include <sstream>
#include <string>

template<typename number_t, typename string_t>
class variant {
public:

    number_t defaults() const {
        return 0;
    }
    number_t invalid() const {
        // return default value on fails
        return std::numeric_limits<double>::quiet_NaN();
    }

    number_t number;
    string_t string;

    enum types {
        IS_NUMBER = 1,
        IS_STRING = 2
    } type;

    variant() : number(0), type(IS_NUMBER) {
    }

    template<typename T>
    variant( const T &t ) : number(double(t)), string(), type(IS_NUMBER) {
    }
    variant( const number_t &t ) : number(t), string(), type(IS_NUMBER) {
    }
    variant( const std::nullptr_t &t ) : number(defaults()), string(), type(IS_NUMBER) {
    }

    template<size_t N>
    variant( const char (&str)[N] ) : number(defaults()), string(str), type(IS_STRING) {
    }
    variant( const char *str ) : number(defaults()), string(str ? str : ""), type(IS_STRING) {
    }
    variant( const string_t &t ) : number(defaults()), string(t), type(IS_STRING) {
    }

    template<typename T>
    variant &operator=( const T &other ) {
        *this = variant( other );
        return *this;
    }
    variant &operator=( const variant &other ) {
        if( this != &other ) {
            number = other.number;
            string = other.string;
            type = other.type;
            downcast();
        }
        return *this;
    }

    bool downcast( number_t &n ) const {
        if( type == IS_NUMBER ) {
            n = number;
            return true;
        }
        if( type == IS_STRING ) {
            std::stringstream ss(string);
            if( ss >> n && ss.peek() == EOF ) {
                return true;
            }
            // return default number value on fails
            n = invalid();
            return false;
        }
        assert( !"unreachable code!" );
        return false;
    }

    bool downcast() {
        // try to downcast string to number if possible
        if( type == IS_NUMBER ) {
            return true;
        }
        if( type == IS_STRING ) {
            if( downcast( number ) ) {
                type = IS_NUMBER;
                return true;
            }
            return false;
        }
        assert( !"unreachable code!" );
        return false;
    }

    bool upcast() {
        // try to upcast number to string if possible
        if( type == IS_STRING ) {
            return true;
        }
        if( type == IS_NUMBER ) {
            std::stringstream ss;
            if( ss << number ) {
                string = ss.str();
                type = IS_STRING;
                return true;
            }
            return false;
        }
        assert( !"unreachable code!" );
        return false;
    }

    string_t str() const {
        if( type == IS_STRING ) {
            return string;
        }
        if( type == IS_NUMBER ) {
            std::stringstream ss;
            ss << number;
            return ss.str();
        }
        assert( !"unreachable code!" );
        return std::string();
    }

    number_t num() const {
        if( type == IS_NUMBER ) {
            return number;
        }
        if( type == IS_STRING ) {
            // downcast implicitly
            variant copy = *this;
            if( copy.type == IS_NUMBER ) {
                return copy.number;
            }
            // return default number value on fails
            return invalid();
        }
        assert( !"unreachable code!" );
        return invalid();
    }

    types get_type() const {
        return type;
    }

    // logical base
    /* todo:
     * - !
     */

#   define $operator(OPER,RETURN) \
    const bool operator OPER( const variant &t ) const { \
        number_t n; \
        auto &use = ( this->type == IS_STRING ? *this : t ); \
        if( this->type - t.type == 0 || use.downcast(n) ) \
            return str() OPER t.str(); \
        return RETURN; \
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
        if( this->type == IS_NUMBER && other.type == IS_NUMBER ) \
            return variant( this->number OPER other.number ); \
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
};

template<typename number_t, typename string_t>
static inline std::ostream &operator<<( std::ostream &os, const variant<number_t,string_t> &v ){
    return os << v.str(), os;
}
