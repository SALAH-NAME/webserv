#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>

template<typename T>
class shared_ptr
{
private:
	T* _ptr;
	std::size_t* _refCount;

	// helpter to decrement reference and delete it
	void _release()
	{
		if (_refCount)
		{
			--(*_refCount);
			if (*_refCount == 0)
			{
				delete _ptr;
				delete _refCount;
			}
		}
	}

	// helper to increment reference
	void _acquire(T* p, std::size_t* count)
	{
		_ptr = p;
		_refCount = count;
		if (_refCount)
			++(*_refCount);
	}

public:
	shared_ptr() : _ptr(0), _refCount(0) {}

	// Constructor
	explicit shared_ptr(T* p) : _ptr(p), _refCount(0)
	{
		if (_ptr)
		{
			try
			{
				_refCount = new std::size_t(1);
			}
			catch (...)
			{
				delete _ptr;
				throw;
			}
		}
	}

	// Copy Constructor
	shared_ptr(const shared_ptr& other) : _ptr(0), _refCount(0)
	{
		_acquire(other._ptr, other._refCount);
	}

	// template Copy Constructor
	template<typename U>
	shared_ptr(const shared_ptr<U>& other) : _ptr(0), _refCount(0)
	{
		_acquire(other.get(), other.getRefCount());
	}

	// Destructor
	~shared_ptr()
	{
		_release();
	}

	// Assignment
	shared_ptr& operator=(const shared_ptr& other)
	{
		if (this != &other)
		{
			shared_ptr tmp(other);
			swap(tmp);
		}
		return *this;
	}

	// Template Assignment
	template<typename U>
	shared_ptr& operator=(const shared_ptr<U>& other)
	{
		shared_ptr tmp(other);
		swap(tmp);
		return *this;
	}

	// Reset to null
	void reset()
	{
		shared_ptr tmp;
		swap(tmp);
	}

	// Reset with new ptr
	void reset(T* p)
	{
		shared_ptr tmp(p);
		swap(tmp);
	}

	// swap
	void swap(shared_ptr& other)
	{
		std::swap(_ptr, other._ptr);
		std::swap(_refCount, other._refCount);
	}

	// // //
	T& operator*() const
	{
		if (!_ptr)
			throw std::runtime_error("Dereferencing null shared_ptr");
		return *_ptr;
	}

	T* operator->() const
	{
		if (!_ptr)
			throw std::runtime_error("Dereferencing null shared_ptr");
		return _ptr;
	}

	T* get() const
	{
		return _ptr;
	}

	// Get reference count
	std::size_t useCount() const
	{
		return _refCount ? *_refCount : 0;
	}

	bool unique() const
	{
		return useCount() == 1;
	}

	typedef T* shared_ptr::*bool_type;
	operator bool_type() const
	{
		return _ptr ? &shared_ptr::_ptr : 0;
	}

	bool operator!() const
	{
		return !_ptr;
	}

	template<typename U> friend class shared_ptr;

	template<typename U, typename O>
	friend shared_ptr<U> static_pointer_cast(const shared_ptr<O>& r);

	template<typename U, typename O>
	friend shared_ptr<U> dynamic_pointer_cast(const shared_ptr<O>& r);

	template<typename U, typename O>
	friend shared_ptr<U> const_pointer_cast(const shared_ptr<O>& r);

	std::size_t* getRefCount() const
	{
		return _refCount;
	}
};

template<typename T>
void swap(shared_ptr<T>& a, shared_ptr<T>& b)
{
	a.swap(b);
}

// Comparison operators
template<typename T, typename U>
bool operator==(const shared_ptr<T>& a, const shared_ptr<U>& b) {
    return a.get() == b.get();
}

template<typename T, typename U>
bool operator!=(const shared_ptr<T>& a, const shared_ptr<U>& b) {
    return !(a == b);
}

template<typename T, typename U>
bool operator<(const shared_ptr<T>& a, const shared_ptr<U>& b) {
    return a.get() < b.get();
}

template<typename T, typename U>
bool operator<=(const shared_ptr<T>& a, const shared_ptr<U>& b) {
    return !(b < a);
}

template<typename T, typename U>
bool operator>(const shared_ptr<T>& a, const shared_ptr<U>& b) {
    return b < a;
}

// Comparison with null pointer
template<typename T>
bool operator==(const shared_ptr<T>& p, std::size_t null_val) {
    return null_val == 0 && !p.get();
}

template<typename T>
bool operator==(std::size_t null_val, const shared_ptr<T>& p) {
    return null_val == 0 && !p.get();
}

template<typename T>
bool operator!=(const shared_ptr<T>& p, std::size_t null_val) {
    return !(p == null_val);
}

template<typename T>
bool operator!=(std::size_t null_val, const shared_ptr<T>& p) {
    return !(null_val == p);
}

// Static cast
template<typename T, typename U>
shared_ptr<T> static_pointer_cast(const shared_ptr<U>& r)
{
	T* p = static_cast<T*>(r.get());
	shared_ptr<T> result;
	if (p)
	{
		result._ptr = p;
		result._refCount = r._refCount;
		if (result._refCount)
			++(*result._refCount);
	}
	return result;
}

// Dynamic cast
template<typename T, typename U>
shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U>& r)
{
	T* p = dynamic_cast<T*>(r.get());
	shared_ptr<T> result;
	if (p)
	{
		result._ptr = p;
		result._refCount = r._refCount;
		if (result._refCount)
			++(*result._refCount);
	}
	return result;
}

// Const cast
template<typename T, typename U>
shared_ptr<T> const_pointer_cast(const shared_ptr<U>& r)
{
	T* p = const_cast<T*>(r.get());
	shared_ptr<T> result;
	if (p)
	{
		result._ptr = p;
		result._refCount = r._refCount;
		if (result._refCount)
			++(*result._refCount);
	}
	return result;
}

#endif // !SHARED_PTR_HPP

