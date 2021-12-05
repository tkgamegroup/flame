//#include <flame/foundation/system.h>
//#include <flame/foundation/typeinfo.h>
//
//using namespace flame;
//
//struct SomeClass
//{
//    std::string a;
//    std::vector<int> b;
//    int c = 5;
//};
//
//template<unsigned N>
//struct FixedString 
//{
//    char buf[N + 1] {};
//
//    constexpr FixedString(const char (&str)[N])
//    {
//        for (unsigned i = 0; i != N; i++) 
//            buf[i] = str[i];
//    }
//
//    constexpr operator char const* () const { return buf; }
//};
//
//template<FixedString T>
//struct Foo 
//{
//    static constexpr char const* Name = T;
//
//    void hello() const
//    {
//        printf(Name);
//    }
//};
//
//int entry(int argc, char** args)
//{
//    Foo<"Hello!"> foo;
//    foo.hello();
//
//    SomeClass c;
//    c.a = "Hello World";
//    c.b.push_back(123);
//    c.b.push_back(456);
//    c.c = 5;
//    auto str = TypeInfo::serialize_t(&c);
//    return 0;
//}
//
//FLAME_EXE_MAIN(entry)

#include <iostream>
#include <typeinfo>
#include <vector>

struct Void {};

template<typename...> struct concat;

template<template<typename...> typename List, typename T>
struct concat<List<Void>, T>
{
    typedef List<T> type;
};

template<template<typename...> class List, typename...Types, typename T>
struct concat<List<Types...>, T>
{
    typedef List<Types..., T> type;
};

template<typename...> struct TypeList {};

template<>
struct TypeList<Void> {};
typedef TypeList<Void> TypelistVoid;
#define TYPE_LIST TypelistVoid

class Foo { };

typedef typename concat<TYPE_LIST, Foo>::type TypeListFoo;
#undef TYPE_LIST
#define TYPE_LIST TypeListFoo

class Bar { };

typedef typename concat<TYPE_LIST, Bar>::type TypeListBar;
#undef TYPE_LIST
#define TYPE_LIST TypeListBar

struct list_of_types {
    typedef TYPE_LIST type;
};

template<typename T, typename...Types>
inline void info(TypeList<T, Types...>) {
    std::cout << typeid(T).name() << std::endl;
    info(TypeList<Types...>());
}

//template<typename T>
//inline void info(TypeList<T>) {
//    std::cout << typeid(T).name() << std::endl;
//}

int main() {
    info(list_of_types::type());
    return 0;
}