#include <iostream>

extern void test_all();

int main()
{
	std::cout << "Run tests...\n";
	test_all();
	std::cout << "Done.\n";
	return 0;
}
