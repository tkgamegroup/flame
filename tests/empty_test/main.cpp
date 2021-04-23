struct A
{
    int i = 5;
    float f = 0.5f;
    char c = 'A';
};

int main()
{
    A a;
    auto& [p1, p2, p3] = a;
    return 0;
}
