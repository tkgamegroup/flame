struct Bitmap
{
    struct Creator
    {
        virtual int operator()() = 0;
    };
    static Creator& creator;
};

struct BitmapCreatorImpl : Bitmap::Creator
{
    int operator()() override
    {
        return 123;
    }
}bitmap_creator;

Bitmap::Creator& Bitmap::creator = bitmap_creator;

int main()
{
    auto wtf = Bitmap::creator();
    return 0;
}
