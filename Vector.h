#pragma once
#include "Vector.h"
#include <limits>
#include <stdexcept>
#include <memory>

template <typename T>
class Vector {
private:
	T* memory_begin;
	T* memory_end;
	T* data_end;

public:
	Vector ();	//default constructor
	Vector (const Vector<T> &other);	//copy constructor
	Vector (Vector<T> &&other);	//move constructor

	~Vector() throw();

	Vector<T> &operator= (const Vector<T> &other); // copy
	Vector<T> &operator= (Vector<T> &&other) throw(); // move

	void swap(Vector<T> &other) throw(); // обмен с другим вектором такого же типа за O(1)

	bool empty() const throw();
	ptrdiff_t size() const throw();
	ptrdiff_t max_size() const throw(); // например, (максимальный ptrdiff_t) / sizeof(T)
	ptrdiff_t capacity() const throw();
	void reserve(ptrdiff_t new_capacity);
	void shrink_to_fit();
	void clear() throw(); // возможна реализация через swap

	T &operator[](ptrdiff_t index);
	const T &operator[](ptrdiff_t index) const;


	void push_back(const T &value);
	void push_back(T &&value);
	void pop_back();

	//end of part 1

};

template <typename T>
Vector<T>::Vector () {
	this->memory_begin = static_cast<T*>(operator new[] (0));
	this->memory_end = this->data_end = this->memory_begin;
}

template <typename T>
Vector<T>::Vector (const Vector<T> &other) {
	//no check other == this here: copy constructor is used in shrink_to_fit()

	this->memory_begin = static_cast<T*>(operator new[] (other.size() * sizeof(T)));
	this->memory_end = this->memory_begin + other.size();
	this->data_end = this->memory_end;

	for (T *i = memory_begin, *j = other.memory_begin; i < data_end; ++i, ++j) {
		new(i) T(*j);
	}
}

template <typename T>
Vector<T>::Vector (Vector<T> &&other) {
	this->memory_begin = other.memory_begin;
	this->memory_end = other.memory_end;
	this->data_end = other.data_end;

	other.memory_begin = nullptr;
	other.memory_end = nullptr;
	other.data_end = nullptr;
}

template <typename T>
Vector<T>::~Vector () throw() {
	for (T* i = memory_begin; i < data_end; ++i) {
		i->~T();
	}

	operator delete[] (memory_begin);
}

template <typename T>
Vector<T> &Vector<T>::operator= (const Vector<T> &other) { //copy
	if (memory_begin == other.memory_begin) {
		return *this;
	}

	this->swap(Vector<T>(other));
	return *this;
}

template <typename T>
Vector<T> &Vector<T>::operator= (Vector<T> &&other) throw() { //move
	Vector<T> temp;
	temp.swap(other);
	this->swap(temp);

	return *this;
}

template <typename T>
void Vector<T>::swap(Vector<T> &other) throw() { // обмен с другим вектором такого же типа за O(1)
	std::swap(memory_begin, other.memory_begin);
	std::swap(memory_end, other.memory_end);
	std::swap(data_end, other.data_end);
}

template <typename T>
bool Vector<T>::empty() const throw() {
	return size() == 0;
}

template <typename T>
ptrdiff_t Vector<T>::size() const throw() {
	if (data_end == nullptr || memory_begin == nullptr) {
		return 0;
	}
	return data_end - memory_begin;
}

template <typename T>
ptrdiff_t Vector<T>::max_size() const throw() {
	return numeric_limits<ptrdiff_t>::max() / sizeof(T);
}

template <typename T>
ptrdiff_t Vector<T>::capacity() const throw() {
	return memory_end - memory_begin;
}

template <typename T>
void Vector<T>::reserve(ptrdiff_t new_capacity) {
	if (new_capacity <= memory_end - memory_begin) {
		return;
	}
	if (new_capacity > max_size()) {
		throw length_error("too large capacity");
	}
	if (new_capacity < capacity() * 2) {
		if (capacity() >= max_size() / 2) {
			new_capacity = max_size();
		}
		else {
			new_capacity = capacity() * 2;
		}
	}

	T* begin = static_cast<T*>(operator new[](sizeof(T) * new_capacity));

	for (T *i = memory_begin, *j = begin; i < data_end; ++i, ++j) {
		new(j) T(std::move(*i));
		i->~T();
	}

	ptrdiff_t data_size = size();

	operator delete[] (memory_begin);

	memory_begin = begin;
	memory_end = begin + new_capacity;
	data_end = begin + data_size;
}

template <typename T>
void Vector<T>::shrink_to_fit() {
	this->swap(Vector<T>(static_cast<const Vector<T>&> (*this)));
	/*ptrdiff_t new_capacity = size();

	T* begin = static_cast<T*>(operator new[] (sizeof(T) * new_capacity);

	for (T* i = memory_begin, j = begin; i < data_end; ++i, ++j) {
		new(j) T(std::move(*i));
		i->~T();
	}

	operator delete[] (memory_begin);

	memory_begin = begin;
	data_end = memory_end = begin + new_capacity;*/
}

template <typename T>
void Vector<T>::clear() throw() {
	this->swap(Vector<T>());
}

template <typename T>
T &Vector<T>::operator[](ptrdiff_t index) {
	return *(memory_begin + index);
}

template <typename T>
const T &Vector<T>::operator[](ptrdiff_t index) const {
	return static_cast<const T&>(*(memory_begin + index));
}

template <typename T>
void Vector<T>::push_back(const T &value) {
	this->reserve(this->size() + 1);
	new(data_end)T(value);
	++data_end;
}

template <typename T>
void Vector<T>::push_back(T &&value) {
	this->reserve(this->size() + 1);
	new(data_end)T(value);
	++data_end;
}

template<typename T>
void Vector<T>::pop_back() {
	if (data_end > memory_begin) {
		(data_end - 1)->~T();
		--data_end;
	}
}

template <typename T>
void swap (Vector<T> &v1, Vector<T> &v2) {
	v1.swap(v2);
}