#ifndef ASSERTS_H
#define ASSERTS_H
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <iostream>

using namespace std;

namespace assertions {

	void assert_true(bool b) 
	{
		if (!b)
		{
			throw ("Unexpected state");
		}
	}

	void assert_fail(const char* msg) 
	{
		throw (msg);
	}

} // end namespace assertions

#ifndef ASSERT
#define ASSERT(b) assertions::assert_true(b);
#endif // ASSERT

#ifndef ASSERT_FAIL
#define ASSERT_FAIL(m) assertions::assert_fail(m);
#endif // ASSERT_FAIL

#endif // ASSERTS_H