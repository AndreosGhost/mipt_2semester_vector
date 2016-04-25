#pragma once

#include <limits>
#include <stdexcept>
#include <memory>
#include "Iterator.h"

#if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 180000000
#define noexcept throw()
#endif

#ifdef MEMORY_TRACE_MODE
#include "MemoryWatcher.h"
#endif

#ifdef DEBUG_MODE
#include <iostream>
#endif

struct IndexOutOfRangeException : public std::out_of_range {
	explicit IndexOutOfRangeException (const char* msg) : out_of_range (msg) { }
};

struct InvalidOperationException : public ExceptionWithMessage {
	explicit InvalidOperationException (const char* msg) : ExceptionWithMessage (msg) { }
};

template <typename V, typename IteratorTag>
struct IterCtorSpecializer {
	template <typename Iterator>
	void performReserve (V *v, Iterator begin, Iterator end) { }
};

template <typename V>
struct IterCtorSpecializer <V, std::random_access_iterator_tag> {
	template <typename Iterator>
	void performReserve (V *v, Iterator begin, Iterator end) {
		v->reserve (end - begin);
	}
};

template <typename T>
class Vector {
private:
	T* memory_begin;
	T* memory_end;
	T* data_end;

	IteratorContainer<Iterator<T>, Vector<T>> *iteratorContainer; //modifiable iterators
	IteratorContainer<ConstIterator<T>, Vector<T>> *constIteratorContainer; //const iterators

	size_t getOptimalNewCapacity (size_t new_capacity) const {
		if (new_capacity <= capacity()) {
			return capacity();
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

		return new_capacity;
	}

	//Safely initializes iteratorContainer and constIteratorContainer
	void initContainers () {
		iteratorContainer = new IteratorContainer<iterator, Vector<T>> (this);
		try {
			constIteratorContainer = new IteratorContainer<const_iterator, Vector<T>> (this);
		}
		catch (...) { delete iteratorContainer; throw; }
	}

	//for BaseIterator
	const T *getDataEnd () const {
		return data_end;
	}

	//for BaseIterator
	const T *getDataBegin () const {
		return memory_begin;
	}

	//for internal use
	void invalidateIterators () {
		iteratorContainer->invalidateAll();
		constIteratorContainer->invalidateAll();

		initContainers();
	}

	template <typename T1, typename IteratorImpl, typename V>
	friend class BaseIterator;

	friend class Iterator<T>;

	friend class ConstIterator<T>;

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

	bool operator== (const Vector<T> &other) const;
	bool operator!= (const Vector<T> &other) const { return !(*this == other); }

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

	reverse_iterator rbegin() noexcept { return reverse_iterator (end()); }
	reverse_iterator rend() noexcept { return reverse_iterator (begin()); }

	//begin/end const iterators

	const_iterator begin() const noexcept { return cbegin(); }
	const_iterator end() const noexcept { return cend(); }

	const_iterator cbegin() const noexcept { return const_iterator (memory_begin, constIteratorContainer); }
	const_iterator cend() const noexcept { return const_iterator (data_end, constIteratorContainer); }

	//begin/end const reverse iterators

	const_reverse_iterator rbegin() const noexcept { return crbegin(); }
	const_reverse_iterator rend() const noexcept { return crend(); }

	const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator (cend()); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator (cbegin()); }
};

template <typename T>
Vector<T>::Vector () {
	#ifdef DEBUG_MODE
	std::cerr << "Vector()" << std::endl;
	#endif

	memory_begin = data_end = memory_end = nullptr;
	initContainers();

	#ifdef MEMORY_TRACE_MODE
	watcher.onVectorDefCreated ();
	#endif
}

template <typename T>
Vector<T>::Vector (const Vector<T> &other) {
	#ifdef DEBUG_MODE
	std::cerr << "Vector(const &)" << std::endl;
	#endif

	initContainers();

	try {
		memory_begin = static_cast<T*>(operator new[] (other.size() * sizeof(T)));
		memory_end = this->memory_begin + other.size();
		data_end = this->memory_begin;

		#ifdef MEMORY_TRACE_MODE
		watcher.onVectorCopyCreated ();
		watcher.onMemoryAllocated (std::distance (memory_begin, memory_end));
		#endif

		try {
			for (T *j = other.memory_begin; j < other.data_end; ++j) {
				push_back(*j);
			}
		}
		catch (...) {
			for (T* i = memory_begin; i < data_end; ++i) {
				i->~T();
			}
			#ifdef MEMORY_TRACE_MODE
			watcher.onMemoryDeallocated (std::distance(memory_begin, memory_end));
			#endif

			operator delete[] (memory_begin);

			throw;
		}
	}
	catch (...) {
		delete iteratorContainer;
		delete constIteratorContainer;

		throw;
	}
}

template <typename T>
Vector<T>::Vector (Vector<T> &&other) noexcept {
	#ifdef DEBUG_MODE
	std::cerr << "Vector(&&)" << std::endl;
	#endif

	memory_begin = other.memory_begin;
	data_end = other.data_end;
	memory_end = other.memory_end;
	iteratorContainer = other.iteratorContainer;
	constIteratorContainer = other.constIteratorContainer;

	other.memory_begin = other.data_end = other.memory_end = nullptr;
	other.iteratorContainer = nullptr;
	other.constIteratorContainer = nullptr;

	iteratorContainer->vector = this;
	constIteratorContainer->vector = this;

	#ifdef MEMORY_TRACE_MODE
	watcher.onVectorMoveCreated ();
	#endif
}

template <typename T>
template <typename InputIterator>
Vector<T>::Vector (InputIterator begin, InputIterator end) {
	#ifdef DEBUG_MODE
	std::cerr << "Vector(Iterators)" << std::endl;
	#endif

	memory_begin = data_end = memory_end = nullptr;
	initContainers();

	try {
		IterCtorSpecializer<Vector<T>, typename std::iterator_traits<InputIterator>::iterator_category>().performReserve(this, begin, end);

		try {
			for (InputIterator i = begin; i != end; ++i) {
				push_back(*i);
			}
		}
		catch (...) {
			for (T* i = memory_begin; i < data_end; ++i) {
				i->~T();
			}
		}
	}
	catch (...) {
		operator delete [] (memory_begin);
		delete iteratorContainer;
		delete constIteratorContainer;

		throw;
	}

	#ifdef MEMORY_TRACE_MODE
	watcher.onVectorIterCreated ();
	#endif
}

template <typename T>
Vector<T>::~Vector () noexcept {
	#ifdef DEBUG_MODE
	std::cerr << "~Vector()" << std::endl;
	#endif

	#ifdef MEMORY_TRACE_MODE
	watcher.onVectorDestroyed ();
	#endif

	if (iteratorContainer) {
		iteratorContainer->invalidateAll();
	}
	if (constIteratorContainer) {
		constIteratorContainer->invalidateAll();
	}

	if (!memory_begin) {
		return;
	}

	for (T* i = memory_begin; i < data_end; ++i) {
		i->~T();
	}

	#ifdef MEMORY_TRACE_MODE
	watcher.onMemoryDeallocated (std::distance (memory_begin, memory_end));
	#endif

	operator delete[] (memory_begin);
}

template <typename T>
Vector<T> &Vector<T>::operator= (const Vector<T> &other) {
	Vector<T> temp(other);
	this->swap(temp);
	return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator= (Vector<T> &&other) noexcept {
	Vector<T> temp;
	temp.swap(other);
	this->swap(temp);

	return *this;
}

template <typename T>
bool Vector<T>::operator== (const Vector<T> &other) const {
	if (size() != other.size()) {
		return false;
	}

	for (typename Vector<T>::const_iterator thisIt = cbegin(), otherIt = other.cbegin(), thisEnd = cend(); thisIt < thisEnd; ++thisIt, ++otherIt) {
		if (!(*thisIt == *otherIt)) { //operator== uses only operator==
			return false;
		}
	}

	return true;
}

template <typename T>
void Vector<T>::swap(Vector<T> &other) noexcept { // обмен с другим вектором такого же типа за O(1)
	this->invalidateIterators();
	other.invalidateIterators();

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

template <typename T>
void swap (Vector<T> &v1, Vector<T> &v2) {
	v1.swap(v2);
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
	size_t data_size = size();

	invalidateIterators();

	T* begin = static_cast<T*>(operator new[](sizeof(T) * new_capacity));

	#ifdef MEMORY_TRACE_MODE
	watcher.onMemoryAllocated (std::distance (begin, begin + new_capacity)); //uniform memory measure - std::distance
	#endif

	for (T *i = memory_begin, *j = begin; i < data_end; ++i, ++j) {
		new(j) T(std::move(*i));
		i->~T();
	}

	if (memory_begin) {
		#ifdef MEMORY_TRACE_MODE
		watcher.onMemoryDeallocated (std::distance (memory_begin, memory_end));
		#endif

		operator delete[] (memory_begin);
	}

	memory_begin = begin;
	memory_end = begin + new_capacity;
	data_end = begin + data_size;
}

template <typename T>
void Vector<T>::shrink_to_fit() {
	if (size() < capacity()) {
		Vector<T> temp(*this);
		this->swap(temp);
	}
}

template <typename T>
void Vector<T>::clear() noexcept {
	Vector<T> temp;
	this->swap(temp);
}

template <typename T>
inline T &Vector<T>::operator[](size_t index) {
	if (index >= size()) {
		throw IndexOutOfRangeException ("Index out of range");
	}

	return *(memory_begin + index);
}

template <typename T>
inline const T &Vector<T>::operator[](size_t index) const {
	if (index >= size()) {
		throw IndexOutOfRangeException ("Index out of range");
	}

	return static_cast<const T&>(*(memory_begin + index));
}

template <typename T>
void Vector<T>::push_back(const T &value) {
	reserve(getOptimalNewCapacity(size() + 1));

	if (size() == capacity()) {
		throw std::runtime_error ("No memory to place an element");
	}

	new(data_end)T(value);
	++data_end;
}

template <typename T>
void Vector<T>::push_back(T &&value) {
	reserve(getOptimalNewCapacity(size() + 1));

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
