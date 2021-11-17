#include <filesystem>

int main()
{
    auto a = std::filesystem::path("a\\b");
    auto b = std::filesystem::path("a/b");
    auto c = a == b;
    system("pause");
    return 0;
}
