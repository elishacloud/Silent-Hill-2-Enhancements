/**
* Copyright (C) 2014 Patrick Mours. All rights reserved.
* License: https://github.com/crosire/reshade#license
*
* Copyright (C) 2023 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#pragma once

#include <cassert>

template <typename T>
class com_ptr
{
public:
	com_ptr()
		: _object(nullptr) {}
	com_ptr(std::nullptr_t)
		: _object(nullptr) {}
	com_ptr(T *object, bool own = false)
		: _object(object)
	{
		if (!own && _object != nullptr)
			_object->AddRef();
	}
	com_ptr(const com_ptr<T> &ptr)
		: _object(nullptr) { reset(ptr._object); }
	com_ptr(com_ptr<T> &&ptr)
		: _object(nullptr) { operator=(std::move(ptr)); }
	~com_ptr() { reset(); }

	// Returns the stored pointer to the managed object.
	T *get() const { return _object; }

	// Returns the current COM reference count of the managed object.
	unsigned long ref_count() const
	{
		assert(_object != nullptr);
		return _object->AddRef(), _object->Release();
	}

	// Returns the stored pointer and releases ownership without decreasing the reference count.
	T *release()
	{
		T *const object = _object;
		_object = nullptr;
		return object;
	}

	// The new object to manage and take ownership and add a reference to.
	void reset(T *object = nullptr)
	{
		if (_object != nullptr)
			_object->Release();
		_object = object;
		if (_object != nullptr)
			_object->AddRef();
	}

	// Overloaded pointer operators which operate on the managed object.
	T &operator*() const
	{
		assert(_object != nullptr);
		return *_object;
	}
	T *operator->() const
	{
		assert(_object != nullptr);
		return _object;
	}

	// This should only be called on uninitialized objects, e.g. when passed into 'QueryInterface' or creation functions.
	T **operator&()
	{
		assert(_object == nullptr);
		return &_object;
	}

	com_ptr<T> &operator=(T *object)
	{
		reset(object);
		return *this;
	}
	com_ptr<T> &operator=(const com_ptr<T> &copy)
	{
		reset(copy._object);
		return *this;
	}
	com_ptr<T> &operator=(com_ptr<T> &&move)
	{
		// Clear the current object first
		if (_object != nullptr)
			_object->Release();

		_object = move._object;
		move._object = nullptr;

		return *this;
	}

	bool operator==(const T *rhs) const { return _object == rhs; }
	bool operator==(const com_ptr<T> &rhs) const { return _object == rhs._object; }
	friend bool operator==(const T *lhs, const com_ptr<T> &rhs) { return rhs.operator==(lhs); }
	bool operator!=(const T *rhs) const { return _object != rhs; }
	bool operator!=(const com_ptr<T> &rhs) const { return _object != rhs._object; }
	friend bool operator!=(const T *lhs, const com_ptr<T> &rhs) { return rhs.operator!=(lhs); }

	// Default operator used for sorting
	friend bool operator< (const com_ptr<T> &lhs, const com_ptr<T> &rhs) { return lhs._object < rhs._object; }

private:
	T *_object;
};

#include <functional> // std::hash

namespace std
{
	template <typename T>
	struct hash<com_ptr<T>>
	{
		size_t operator()(const com_ptr<T> &ptr) const
		{
			return std::hash<T *>()(ptr.get());
		}
	};
}
