template<typename T>
struct Property
{
    T v;
    T (*getter)();

    operator T()
    {
        return getter();
    }
};

int main() 
{
    Property<int> a;
    int b = a;
    return 0;
}
