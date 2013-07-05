#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <cassert>
#include <cstdlib>

#include <memory>

#include "numtype.h"

/**
 * @brief Essentially a custom implementation of standard vector.
 *
 * This way I can rely on very specific behaviors, regardless of compiler
 * or platform.
 *
 * The most important behavior is that reducing the size of an Array never
 * reduces its capacity or frees memory.  This is an important behavior for
 * several places where Arrays are used in Psychopath.
 */
template <class T>
class Array
{
	size_t size_ {0};
	size_t capacity_ {0};
	std::unique_ptr<T[]> data {nullptr};

public:
	Array() {}
	Array(size_t size): size_ {size}, capacity_ {size}, data {new T[size]} {}


	/**
	 * @brief Increase capacity of the array.
	 *
	 * This does _not_ shrink the capacity, only increases.
	 * If cap is less then the current capacity, it does nothing.
	 */
	void reserve(size_t cap) {
		if (cap <= capacity_)
			return;

		std::unique_ptr<T[]> data2 {new T[cap]};
		for (size_t i=0; i < size_; i++)
			data2[i] = data[i];
		data.swap(data2);

		capacity_ = cap;
	}

	/**
	 * @brief Resize the array.
	 *
	 * This does _not_ free any space.  The capacity is only increased,
	 * never decreased.
	 */
	void resize(size_t size) {
		reserve(size);
		size_ = size;
	}

	/**
	 * @brief Sets the array size to zero.
	 */
	void clear() {
		size_ = 0;
	}

	/**
	 * @brief Returns the current capacity of the array.
	 */
	const size_t &capacity() const {
		return capacity_;
	}

	/**
	 * @brief Returns the current size of the array.
	 */
	const size_t &size() const {
		return size_;
	}

	/**
	 * @brief Returns an iterator at the first element in the array.
	 */
	T *begin() const {
		if (size_ == 0)
			return nullptr;
		else
			return &(data[0]);
	}

	/**
	 * @brief Returns an iterator at the last element in the array.
	 */
	T *end() const {
		if (size_ == 0)
			return nullptr;
		else
			return &(data[size_-1]);
	}

	/**
	 * @brief Access to array elements.
	 */
	T& operator[](const size_t &n) {
		assert(n >= 0 && n < size_);
		return data[n];
	}
	/**
	 * @brief Access to array elements.
	 */
	const T& operator[](const size_t &n) const {
		assert(n >= 0 && n < size_);
		return data[n];
	}

	/**
	 * @brief Adds a new element to the end of the array.
	 */
	void push_back(const T& el) {
		resize(size_+1);
		data[size_-1] = el;
	}
};

#endif // ARRAY_HPP
