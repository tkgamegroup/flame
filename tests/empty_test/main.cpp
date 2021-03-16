#include <typeinfo>
#include <stdio.h>

enum WTF
{

};

int main(int argc, char** args)
{
	printf(typeid(WTF).name());
	return 0;
}
