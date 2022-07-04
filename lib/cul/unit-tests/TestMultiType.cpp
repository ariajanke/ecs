/****************************************************************************

    MIT License

    Copyright (c) 2021 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*****************************************************************************/

#include <common/MultiType.hpp>
#include <common/TestSuite.hpp>

#include <iostream>
#include <variant>

#include <cassert>

using namespace cul;

class Base {
public:
	virtual ~Base();
};

class A final : public Base {
public:
	A() { ++s_count; }
	A(const A  &) { ++s_count; }
	A(      A &&) { ++s_count; }
	A & operator = (const A  &) { return *this; }
	A & operator = (      A &&) { return *this; }
	~A() override { --s_count; }
	static int get_count() { return s_count; }
	static void reset_count() { s_count = 0; }
private:
	static int s_count;
};

class SideBase {
public:
	~SideBase();
};

class B final : public Base {
public:
};

class C final : public Base, public SideBase {
public:
};

/* static */ int A::s_count = 0;

Base::~Base() {}

SideBase::~SideBase() {}

static_assert(TypeList<A, B, C, int>::HasType<int>::k_value, "");
static_assert(TypeList<A, B, C, int>::HasType<  A>::k_value, "");
static_assert(TypeList<A, B, C, int>::HasType<  B>::k_value, "");
static_assert(TypeList<A, B, C, int>::HasType<  C>::k_value, "");

using ABCTypeList = TypeList<A, B, C>;

class D {};
class E {};
using ABCDETypeList = ABCTypeList::CombineWith<D, E>;

#define mark MACRO_MARK_POSITION_OF_CUL_TEST_SUITE

int main() {
    {
    using MyVariant = TypeList<A, B, C>::DefineWithListTypes<std::variant>::Type;
    MyVariant var { A() };
    assert(std::holds_alternative<A>(var));
    }
	// assert fails: the tests/utils for the tests are badly designed
	// test on the ts fails: the MultiType class has the issue
	using TestMt = MultiType<int, double, A, B>;
	
	ts::TestSuite suite;
	suite.start_series("MultiType");
    suite.hide_successes();
	
	// there's some assumption here that type_id will work...
	// default constructor
    mark(suite).test([] {
		TestMt a;
		return ts::test(a.type_id() == TestMt::k_no_type);
    });
	// value constructor
    mark(suite).test([] {
		A a;
		TestMt b(a);
		return ts::test(A::get_count() == 2);
    });
	A::reset_count();
	
	// copy constructor
    mark(suite).test([] {
		A a;
		TestMt b(a);
		TestMt c(b);
		return ts::test(A::get_count() == 3);
    });
	A::reset_count();
	
	// destructor
    mark(suite).test([] {
		A c;
		{
		A a;
		TestMt b(a);
		}
		return ts::test(A::get_count() == 1);
    });
	A::reset_count();
	
	// as_pointer (three cases, one const)
    mark(suite).test([] {
		TestMt a;
		return ts::test(!a.as_pointer<int>());
    });
    mark(suite).test([] {
		int a = 10;
		TestMt b(a);
		return ts::test(b.as_pointer<int>());
    });
    mark(suite).test([] {
		B a;
		TestMt b(a);
		const TestMt & c = b;
		return ts::test(!c.as_pointer<int>());
    });
	// as (two cases, one throws)
    mark(suite).test([] {
		int t = 10;
		TestMt b(t);
		return ts::test(b.as<int>() == 10);
    });
    mark(suite).test([] {
		bool did_throw = false;
		try {
			TestMt a;
			a.as<int>() = 10;
		} catch (...) {
			did_throw = true;
		}
		return ts::test(did_throw);
    });
	// reset
    mark(suite).test([] {
		B a;
		TestMt b(a);
		b.reset<int>(10);
		return ts::test(b.as<int>() == 10);
    });
	// unset
    mark(suite).test([] {
		A a;
		TestMt b(a);
		b.unset();
		return ts::test(A::get_count() == 1);
    });
	A::reset_count();
	
	// is_type
    mark(suite).test([] {
		A a;
		TestMt b(a);
		return ts::test(b.is_type<A>());
    });
    mark(suite).test([] {
		TestMt b;
		return ts::test(!b.is_type<A>());
    });
	// is_valid (two cases)
    mark(suite).test([] {
		A a;
		TestMt b(a);
		return ts::test(b.is_valid());
    });
    mark(suite).test([] {
		TestMt b;
		return ts::test(!b.is_valid());
    });
	// C++ won't let you static_cast from a class to an int (without 
	// operators at least)
	// set_by_type_id_and_upcast (two cases: A, and B)
    mark(suite).test([] {
		TestMt a;
		auto gv = a.set_by_type_id_and_upcast<Base>
			(TestMt::GetTypeId<A>::k_value);
		return ts::test(   gv.object_pointer && gv.upcasted_pointer
		                && A::get_count() == 1);
    });
	A::reset_count();
	
    mark(suite).test([] {
		// an upcast fails (returns nullptr) if the leaf type is not 
		// derived from Base, a bit different from static_cast
		// this allows that MultiType to exist where not all types have
		// have to inherit from a common base
		TestMt a;
		auto gv = a.set_by_type_id_and_upcast<Base>
			(TestMt::GetTypeId<int>::k_value);
		return ts::test(gv.object_pointer && !gv.upcasted_pointer);
    });
	// get_by_type_id_and_upcast
	// match ok
    mark(suite).test([] {
		B a;
		TestMt b(a);
		auto gv = b.get_by_type_id_and_upcast<Base>(TestMt::GetTypeId<B>::k_value);
		return ts::test(gv.object_pointer && gv.upcasted_pointer);
    });
	// I'm not sure what the proper thing to do here is... so I'm leaving the
	// previous behavior in place
#	if 0
	// type mismatch
	suite.do_test([]() {
		TestMt b(int(1));
		auto gv = b.get_by_type_id_and_upcast<Base>(TestMt::GetTypeId<B>::k_value);
		return ts::test(!gv.object_pointer && !gv.upcasted_pointer);
	});
#	endif
	// cast to base fails
    mark(suite).test([] {
		B a;
		TestMt b(a);
		auto gv = b.get_by_type_id_and_upcast<int>(TestMt::GetTypeId<B>::k_value);
		return ts::test(gv.object_pointer && !gv.upcasted_pointer);
    });
	// match ok, constant
    mark(suite).test([] {
		B a;
		TestMt b(a);
		const auto & c = b;
		
		auto gv = c.get_by_type_id_and_upcast<Base>(TestMt::GetTypeId<B>::k_value);
		return ts::test(gv.object_pointer && gv.upcasted_pointer);
    });
	// not sure how useful dynamic_cast_ and static_cast_ are for 
	// MultiType
	// I can think of a single instance...?
#	if 0
	// dynamic_cast_ (cross-cast with pointee, with null, to unrelated (null))
	suite.do_test([]() {
		TestMt b;
		b.reset<C>();
		return ts::test(false);
	});
	suite.do_test([]() {
		return ts::test(false);
	});
	suite.do_test([]() {
		return ts::test(false);
	});
#	endif
	// static_cast_ ???
	// test copying uninitialized MultiType (on that stores nothing) on to one with
	// an object on it
	// expected result: the second multitype should become "unset"
    mark(suite).test([] {
		TestMt a;
		TestMt c(a);
		return ts::test(!c.is_valid());
    });

    return 0;
}
