#include "SharedPtr.hpp"
#include <cstddef>
#include <exception>
#include <iostream>
#include <vector>

class Base
{
public:
	virtual ~Base() { std::cout << "Base destructor\n"; }
	virtual void print() const { std::cout << "Base::print()\n"; }
};

class Derived : public Base
{
public:
	~Derived() { std::cout << "Derived destructor\n"; }
	void print() const { std::cout << "Derived::print()\n"; }
};

void testSharedPtr()
{
	std::cout << "=== Testing shared_ptr Implementation ===\n\n";

	std::cout << "Test 1: Basic construction\n";
	{
		shared_ptr<int> p1(new int(37));
		std::cout << "Value: " << *p1 << ", Use count: " << p1.useCount() << std::endl;

		shared_ptr<int> p2 = p1;
		std::cout << "After copy - p1 use count: " << p1.useCount()
			<< ", p2 use count: " << p2.useCount() << std::endl;

		shared_ptr<int> p3;
		p3 = p1;
		std::cout << "After assignment - use count: " << p1.useCount() << std::endl;
	}

	std::cout << "Objects destroyed\n\n";

	std::cout << "Test 2: Reset functionality\n";
	{
		shared_ptr<int> p1(new int(103));
		std::cout << "Initial value: " << *p1 << std::endl;

		p1.reset(new int(227));
		std::cout << "After reset: " << *p1 << std::endl;

		p1.reset();
		std::cout << "After reset to null: " << (p1 ? "not null" : "null") << std::endl;
	}

	std::cout << std::endl;

	std::cout << "Test 3: Polymorphism and casting\n";
	{
		shared_ptr<Base> base_ptr(new Derived());
		base_ptr->print();

		shared_ptr<Derived> derived_ptr = dynamic_pointer_cast<Derived>(base_ptr);
		if (derived_ptr)
		{
			std::cout << "Dynamic cast successful\n";
			derived_ptr->print();
		}

			shared_ptr<Base> base_ptr2 = static_pointer_cast<Base>(derived_ptr);
			std::cout << "Use count after static case: " << base_ptr2.useCount() << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Test 4: Container usage\n";
	{
		std::vector<shared_ptr<int> > vec;

		for (int i = 0; i < 3; ++i)
			vec.push_back(shared_ptr<int>(new int (i * 10)));

		std::cout << "Vector contents: ";
		for (std::size_t i = 0; i < vec.size(); ++i)
			std::cout << *vec[i] << " ";
		std::cout << std::endl;

		shared_ptr<int> copy = vec[2];
		std::cout << "Reference count for vec[2] " << vec[2].useCount() << std::endl;
	}
	std::cout << std::endl;


	std::cout << "Test 5: Comparison operators\n";
	{
		shared_ptr<int> p1(new int(11));
		shared_ptr<int> p2 = p1;
		shared_ptr<int> p3(new int(23));
		shared_ptr<int> p4;

		std::cout << "p1 == p2: " << (p1 == p2) << std::endl;
		std::cout << "p1 == p3: " << (p1 == p3) << std::endl;
		std::cout << "p4 == 0 : " << (p4 == 0) << std::endl;
		std::cout << "p1 != 0 : " << (p1 != 0) << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Test 6: Execption safety\n";
	{
		try
		{
			shared_ptr<int> p;

			std::cout << "Null pointer test: " << (!p ? "null" : "not null") << std::endl;
		} catch (const std::exception& e)
		{
			std::cout << "Caught execption: " << e.what() << std::endl;
		}
		try
		{
			shared_ptr<int> p;

			std::cout << "Null pointer test: " << *p << std::endl;
		} catch (const std::exception& e)
		{
			std::cout << "Caught execption: " << e.what() << std::endl;
		}
	}

	std::cout << "\n=== All tests completed === \n";
}
