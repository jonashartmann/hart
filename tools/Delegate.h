#pragma once

/********************
* CALLBACK CLASS
********************/
template <typename Ret, typename Param0>
class Callback
{
public:
	virtual Ret invoke(Param0 param0) = 0;
};

/********************
* STATIC CALLBACK CLASS
********************/
template <typename Ret, typename Param0>
class StaticFunctionCallback : public Callback<Ret, Param0>
{
private:
	Ret (*func_)(Param0);

public:
	StaticFunctionCallback(Ret (*func)(Param0))
		: func_(func)
	{}

	virtual Ret invoke(Param0 param0)
	{
		return (*func_)(param0);
	}
};

/********************
* METHOD CALLBACK CLASS
********************/
template <typename Ret, typename Param0, typename T, typename Method>
class MethodCallback : public Callback<Ret, Param0>
{
private:
	void *object_;
	Method method_;

public:
	MethodCallback(void *object, Method method)
		: object_(object)
		, method_(method)
	{}

	virtual Ret invoke(Param0 param0)
	{
		T *obj = static_cast<T *>(object_);
		return (obj->*method_)(param0);
	}
};

/********************
* DELEGATE CLASS
********************/
template <typename Ret, typename Param0>
class Delegate
{
private:
	Callback<Ret, Param0> *callback_;

public:
	Delegate(Ret)
		:callback_(new StaticFunctionCallback<Ret, void>(func))
	{}

	Delegate(Ret (*func)(Param0))
		:callback_(new StaticFunctionCallback<Ret, Param0>(func))
	{}

	template <typename T, typename Method>
	Delegate(T *object, Method method)
		:callback_(new MethodCallback<Ret, Param0, T, Method>(object, method))
	{}

	~Delegate(void) { delete callback_; }

	Ret operator()(Param0 param0)
	{
		return callback_->invoke(param0);
	}
};
