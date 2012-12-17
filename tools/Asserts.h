#pragma once

void assert_func(bool b) 
{
	if (!b)
	{
		throw ("Unexpected state");
	}
}

#define ASSERT void assert_func(bool b);