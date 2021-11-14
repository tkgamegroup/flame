#include <string>

struct B
{
    virtual void do_sth() = 0;
};

struct A : B
{
    std::string str;
    std::wstring wstr;

    void do_sth() override
    {
        str = "emmm";
    }

    std::string get_str() const
    {
        return str;
    }

    void set_str(const std::string& v)
    {
        str = v;
    }
};

int main()
{
    A a;
    a.str = "Hello";
    auto s = a.get_str();
    a.set_str("");
    return 0;
}
