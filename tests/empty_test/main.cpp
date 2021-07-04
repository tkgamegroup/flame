struct Base
{
    virtual ~Base() {}
};

struct Fucker
{
    virtual int fuck(int a)
    {
        return a - a;
    }
};

int main()
{
    auto f = new Fucker;
    auto b = new Base;
    auto wtf1 = dynamic_cast<Fucker*>(f);
    auto wtf2 = dynamic_cast<Fucker*>(b);
    return 0;
}
