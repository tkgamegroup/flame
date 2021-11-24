#include <typeinfo>

template <class T>
void abc(T v)
{
    auto wtf = typeid(T).name();
}

namespace Hello
{
    enum EEE
    {
        ABC
    };
}

using namespace Hello;

int main()
{
    EEE a = ABC;
    abc(a);
    return 0;
}
