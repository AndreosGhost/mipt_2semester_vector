#pragma once

#include <limits>
#include <stdexcept>
#include <memory>
#include "Iterator.h"

#if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 180021114
#define noexcept throw()
#endif

struct IndexOutOfRangeException : public std::exception {
	IndexOutOfRangeException () : exception () { }
	explicit IndexOutOfRangeException (const char* const &msg) : exception (msg) { }
};

struct InvalidOperationException : public std::exception {
	InvalidOperationException () : exception () { }
	explicit InvalidOperationException (const char* const &msg) : exception (msg) { }
};

template <typename T>
class Vector {
private:
	T* memory_begin;
	T* memory_end;
	T* data_end;

	mutable IteratorContainer<Iterator<T>, Vector<T>> *iteratorContainer; //modifiable iterators
	mutable IteratorContainer<ConstIterator<T>, Vector<T>> *constIteratorContainer; //const iterators

	//for BaseIterator
	const T *getDataEnd () const {
		return data_end;
	}

	//for BaseIterator
	const T *getDataBegin () const {
		return memory_begin;
	}

	//for casting iterator to const_iterator
	IteratorContainer<ConstIterator<T>, Vector<T>>* getConstIteratorContainer () const {
		return constIteratorContainer;
	}

	//for internal use
	void invalidateIterators () const {
		iteratorContainer->invalidateAll();
		constIteratorContainer->invalidateAll();
	}

	template <typename T, typename IteratorImpl, typename V>
	friend class BaseIterator;

	friend class Iterator<T>;

	template <typename T>
	friend class ConstIterator;

	template <typename IteratorImpl, typename V>
	friend class IteratorContainer;

public:
	typedef Iterator<T> iterator;
	typedef ConstIterator<T> const_iterator;
	typedef std::reverse_iterator<Iterator<T>> reverse_iterator;
	typedef std::reverse_iterator<ConstIterator<T>> const_reverse_iterator;

	typedef InvalidIteratorException invalid_iterator_exception;
	typedef DifferentIteratorDomainException different_iterator_domain_exception;
	typedef IteratorOutOfRangeException iterator_out_of_range_exception;
	typedef InvalidIteratorShiftException invalid_iterator_shift_exception;

	typedef InvalidOperationException invalid_operation_exception;
	typedef IndexOutOfRangeException index_out_of_range_exception;

	Vector ();	//default constructor
	Vector (const Vector<T> &other);	//copy constructor
	Vector (Vector<T> &&other) noexcept;	//move constructor

	template <typename InputIterator>
	Vector (InputIterator begin, InputIterator end);

	~Vector() noexcept;

	Vector<T>& operator= (const Vector<T> &other); // copy
	Vector<T>& operator= (Vector<T> &&other) noexcept; // move

	void swap(Vector<T> &other) noexcept; // обмен с другим вектором такого же типа за O(1)

	bool empty() const noexcept { return size() == 0; }

	size_t size() const noexcept;
	size_t max_size() const noexcept; // например, (максимальный size_t) / sizeof(T)
	size_t capacity() const noexcept;
	void reserve(size_t new_capacity);
	void shrink_to_fit();
	void clear() noexcept; // возможна реализация через swap

	T& operator[](size_t index);
	const T& operator[](size_t index) const;

	void push_back(const T &value);
	void push_back(T &&value);
	void pop_back();

	//begin/end iterators

	iterator begin() noexcept { return iterator (memory_begin, iteratorContainer); }
	iterator end() noexcept { return iterator (data_end, iteratorContainer); }

	//begin/end reverse iterators

	reverse_iterator rbegin() noexcept { return reverse_iterator (begin()); }
	reverse_iterator rend() noexcept { return reverse_iterator (end()); }

	//begin/end const iterators

	const_iterator begin() const noexcept { return cbegin(); }
	const_iterator end() const noexcept { return cend(); }

	const_iterator cbegin() const noexcept { return const_iterator (memory_begin, &constIteratorContainer); }
	const_iterator cend() const noexcept { return const_iterator (data_end, &constIteratorContainer); }

	//begin/end const reverse iterators

	const_reverse_iterator rbegin() const noexcept { return crbegin(); }
	const_reverse_iterator rend() const noexcept { return crend(); }

	const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator (cbegin()); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator (cend()); }
};

template <typename T>
Vector<T>::Vector () {
	if (DEBUG_MODE) {
		cerr << "Vector()" << endl;
	}

	memory_begin = data_end = memory_end = nullptr;

	iteratorContainer = new IteratorContainer<iterator, Vector<T>> (this);
	constIteratorContainer = new IteratorContainer<const_iterator, Vector<T>> (this);
}

template <typename T>
Vector<T>::Vector (const Vector<T> &other) {
	if (DEBUG_MODE) {
		cerr << "Vector(const &)" << endl;
	}

	iteratorContainer = new IteratorContainer<iterator, Vector<T>> (this);
	constIteratorContainer = new IteratorContainer<const_iterator, Vector<T>> (this);

	//no check other == this here: copy constructor is used in shrink_to_fit()

	try {
		memory_begin = static_cast<T*>(operator new[] (other.size() * sizeof(T)));
		memory_end = this->memory_begin + other.size();
		data_end = this->memory_begin;
	}
	catch (exception &exc) {
		operator delete[] (memory_begin);
		memory_begin = data_end = memory_end = nullptr;

		delete iteratorContainer;
		delete constIteratorContainer;

		throw exc;
	}

	try {
		for (T *j = other.memory_begin; j < other.data_end; ++j) {
			push_back(*j);
		}
	}
	catch (exception &exc) {
		for (T* i = memory_begin; i < data_end; ++i) {
			i->~T();
		}
		operator delete[] (memory_begin);
		memory_begin = data_end = memory_end = nullptr;

		delete iteratorContainer;
		delete constIteratorContainer;

		throw exc;
	}
}

template <typename T>
Vector<T>::Vector (Vector<T> &&other) noexcept {
	if (DEBUG_MODE)  {
		cerr << "Vector(&&)" << endl;
	}

	memory_begin = std::move(other.memory_begin);
	data_end = std::move(other.data_end);
	memory_end = std::move(other.memory_end);
	iteratorContainer = std::move(other.iteratorContainer);
	constIteratorContainer = std::move(other.constIteratorContainer);

	other.memory_begin = other.data_end = other.memory_end = nullptr;
	other.iteratorContainer = nullptr;
	other.constIteratorContainer = nullptr;

	iteratorContainer->vector = this;
	constIteratorContainer->vector = this;
}

template <typename T>
template <typename InputIterator>
Vector<T>::Vector (InputIterator begin, InputIterator end) {
	if (DEBUG_MODE)  {
		cerr << "Vector(Iterators)" << endl;
	}

	memory_begin = data_end = memory_end = nullptr;
	iteratorContainer = new IteratorContainer<iterator, Vector<T>> (this);
	constIteratorContainer = new IteratorContainer<const_iterator, Vector<T>> (this);


	if (typeid(std::iterator_traits<InputIterator>::iterator_category) == typeid(std::random_access_iterator_tag)) {
		reserve(end - begin);
	}

	for (InputIterator i = begin; i < end; ++i) {
		push_back(*i);
	}
}

template <typename T>
Vector<T>::~Vector () noexcept {
	if (DEBUG_MODE) {
		cerr << "~Vector()" << endl;
	}

	if (iteratorContainer != nullptr) {
		iteratorContainer->onVectorDestroy();
	}
	if (constIteratorContainer != nullptr) {
		constIteratorContainer->onVectorDestroy();
	}

	if (memory_begin == nullptr) {
		return;
	}

	for (T* i = memory_begin; i < data_end; ++i) {
		i->~T();
	}

	operator delete[] (memory_begin);
}

template <typename T>
Vector<T> &Vector<T>::operator= (const Vector<T> &other) {
	Vector<T> temp(other);
	this->swap(temp);
	return *this;
}

template <typename T>
Vector<T> &Vector<T>::operator= (Vector<T> &&other) noexcept {
	//as adviced at msdn
	if (*this == other) {
		return *this;
	}

	Vector<T> temp;
	temp.swap(other);
	this->swap(temp);

	return *this;
}

template <typename T>
void Vector<T>::swap(Vector<T> &other) noexcept { // обмен с другим вектором такого же типа за O(1)
	std::swap(memory_begin, other.memory_begin);
	std::swap(memory_end, other.memory_end);
	std::swap(data_end, other.data_end);

	//fixing links to vector inside containers
	iteratorContainer->vector = &other;
	constIteratorContainer->vector = &other;
	other.iteratorContainer->vector = this;
	other.constIteratorContainer->vector = this;

	//swapping pointers
	std::swap(iteratorContainer, other.iteratorContainer);
	std::swap(constIteratorContainer, other.constIteratorContainer);
}

namespace std {
	template <typename T>
	void swap (Vector<T> &v1, Vector<T> &v2) {
		v1.swap(v2);
	}
}

template <typename T>
size_t Vector<T>::size() const noexcept {
	return data_end - memory_begin;
}

template <typename T>
inline size_t Vector<T>::max_size() const noexcept {
	return std::numeric_limits<size_t>::max() / sizeof(T);
}

template <typename T>
size_t Vector<T>::capacity() const noexcept {
	return memory_end - memory_begin;
}

template <typename T>
void Vector<T>::reserve(size_t new_capacity) {
	if (new_capacity <= capacity()) {
		return;
	}
	if (new_capacity > max_size()) {
		throw std::runtime_error("too large capacity");
	}
	if (new_capacity < capacity() * 2) {
		if (capacity() >= max_size() / 2) {
			new_capacity = max_size();
		}
		else {
			new_capacity = capacity() * 2;
		}
	}
	size_t data_size = size();

	if (new_capacity <= data_size) {
		return;
	}

	invalidateIterators();

	T* begin = static_cast<T*>(operator new[](sizeof(T) * new_capacity));
	for (T *i = memory_begin, *j = begin; i < data_end; ++i, ++j) {
		new(j) T(std::move(*i));
		i->~T();
	}
	
	operator delete[] (memory_begin);

	memory_begin = begin;
	memory_end = begin + new_capacity;
	data_end = begin + data_size;
}

template <typename T>
void Vector<T>::shrink_to_fit() {
	if (size() < capacity()) {
		invalidateIterators();
		Vector<T> temp(*this);
		this->swap(temp);
	}
}

template <typename T>
void Vector<T>::clear() noexcept {
	invalidateIterators();
	Vector<T> temp;
	this->swap(temp);
}

template <typename T>
T &Vector<T>::operator[](size_t index) {
	if (index >= size()) {
		throw IndexOutOfRangeException ("Index out of range");
	}

	return *(memory_begin + index);
}

template <typename T>
inline const T &Vector<T>::operator[](size_t index) const {
	return static_cast<const T&>(*(memory_begin + index));
}

template <typename T>
void Vector<T>::push_back(const T &value) {
	reserve(size() + 1);

	if (size() == capacity()) {
		throw std::runtime_error ("No memory to place an element");
	}

	new(data_end)T(value);
	++data_end;
}

template <typename T>
void Vector<T>::push_back(T &&value) {
	reserve(size() + 1);

	if (size() == capacity()) {
		throw std::runtime_error ("No memory to place an element");
	}

	new(data_end)T(std::move(value));
	++data_end;
}

template<typename T>
void Vector<T>::pop_back() {
	if (data_end > memory_begin) {
		(data_end - 1)->~T();
		--data_end;
	}
	else {
		throw InvalidOperationException ("Cannot pop from empty vector");
	}
}