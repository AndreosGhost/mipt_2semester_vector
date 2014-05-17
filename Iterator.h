#pragma once

#include <iterator>
#include <stdexcept>
#include <typeinfo>
#include <cstddef>
#include "Vector.h"

#ifdef MEMORY_TRACE_MODE
#include "MemoryWatcher.h"
#endif

#if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 180000000
#define noexcept throw()
#endif

#ifdef DEBUG_MODE
#include <iostream>
#endif

template <typename T>
class Vector;

template <typename T, typename IteratorImpl, typename V>
class BaseIterator;

#pragma region exceptions

struct ExceptionWithMessage : public std::exception {
    const char* message;

    explicit ExceptionWithMessage (const char* msg) : message(msg) { }
    ExceptionWithMessage () : message(typeid(*this).name()) { }

    const char* what() const noexcept override {
        return message;
    }
};

struct InvalidIteratorException : public ExceptionWithMessage {
	explicit InvalidIteratorException (const char* msg) : ExceptionWithMessage(msg) { }
	InvalidIteratorException () : ExceptionWithMessage ("Invalid iterator") { }
};

struct DifferentIteratorDomainException : public ExceptionWithMessage {
	explicit DifferentIteratorDomainException (const char* msg) : ExceptionWithMessage(msg) { }
	DifferentIteratorDomainException () : ExceptionWithMessage ("Iterators are bound to different vectors") { }
};

struct IteratorOutOfRangeException : public std::out_of_range {
	explicit IteratorOutOfRangeException (const char* msg) : out_of_range(msg) { }
	IteratorOutOfRangeException () : out_of_range ("Iterator ptr out of range") { }
};

struct InvalidIteratorShiftException : public std::out_of_range {
	explicit InvalidIteratorShiftException (const char* msg) : out_of_range(msg) { }
	InvalidIteratorShiftException () : out_of_range ("Iterator shift will lead away from range") { }
};

#pragma endregion

///<summary>
///Links vector and valid/invalid existent iterators
///When vector is destroyed, this structure is destroyed only if no iterators are alive.
///When the last iterator dies, if the vector is already destroyed, this structure dies.
///</summary>
template <typename IteratorImpl, typename V>
class IteratorContainer {
private:
	#if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 180000000
	IteratorContainer (const IteratorContainer<IteratorImpl, V> &);
	IteratorContainer<IteratorImpl, V>& operator= (const IteratorContainer<IteratorImpl, V> &);
	#else
	IteratorContainer (IteratorContainer<IteratorImpl, V> &) = delete;
	IteratorContainer<IteratorImpl, V>& operator= (IteratorContainer<IteratorImpl, V> &) = delete;
	#endif

	template <typename T, typename IteratorImpl1, typename V1>
	friend class BaseIterator;

	template <typename T>
	friend class Vector;

	template <typename T>
	friend class Iterator;

	template <typename T>
	friend class ConstIterator;

	IteratorImpl *headIterator;
	V *vector;

	void addIterator (IteratorImpl *iter);
	void invalidateAll ();

	//is vector already destroyed
	bool isVectorDestroyed () { return !vector; }

public:
	explicit IteratorContainer (V *host) {
		headIterator = nullptr;
		vector = host;

		#ifdef DEBUG_MODE
		std::cerr << "IteratorContainer<" << typeid(IteratorImpl).name() << ">(V*)" << std::endl;
		#endif

		#ifdef MEMORY_TRACE_MODE
		watcher.onContainerHostCreated ();
		#endif
	}

	~IteratorContainer () {
		#ifdef DEBUG_MODE
		std::cerr << "~IteratorContainer<" << typeid(IteratorImpl).name() << ">()" << std::endl;
		#endif

		#ifdef MEMORY_TRACE_MODE
		watcher.onContainerDestroyed();
		#endif
	}
};

template <typename T>
class Iterator;

template <typename T>
class ConstIterator;

template <typename T, typename IteratorImpl, typename V>
IteratorImpl operator+ (ptrdiff_t offset, const BaseIterator<T, IteratorImpl, V> &iter);

//Base class for const and non-const iterator.
//When created within a real vector, registers itself at IteratorContainer.
template <typename T, typename IteratorImpl, typename V>
class BaseIterator : public std::iterator<std::random_access_iterator_tag, T> {
private:
	T *ptr; //ptr inside the vector
	IteratorContainer<IteratorImpl, V> *container; //all iterators of this type bound to the same vector
	IteratorImpl *next; //next iterator in double-linked list (also in container)
	IteratorImpl *prev; //prev iterator in double-linked list (also in container)

	friend class IteratorContainer<IteratorImpl, V>;

	template <typename T2, typename IteratorImpl2, typename V2>
	friend class BaseIterator;

	IteratorImpl* that() {
		return static_cast<IteratorImpl*>(this);
	}

	const IteratorImpl* that () const {
		return static_cast<const IteratorImpl*>(this);
	}

	//check if this and another are bound to the same vector
	//Attention: this->container and another->container must exist!
	template <typename T2, typename IteratorImpl2>
	void checkDomainEquality (const BaseIterator<T2, IteratorImpl2, V> &another) const {
		if (container->vector != another.container->vector) {
			throw DifferentIteratorDomainException();
		}
	}

	void selfRemoveFromContainer ();

protected:
	T* dataPointer () const { return ptr; }
	IteratorContainer<IteratorImpl, V>* iterContainer () const { return container; }

	bool isValid () const {
		return container && container->vector;
	}

	//check if our ptr is valid. Throws exception if not valid.
	void checkValidity () const {
		if (!isValid()) {
			throw InvalidIteratorException();
		}
	}

	BaseIterator (T *ptr, IteratorContainer<IteratorImpl, V> *container);

public:
	BaseIterator ();
	BaseIterator (const BaseIterator<T, IteratorImpl, V> &iter);

	~BaseIterator ();

	IteratorImpl& operator= (const BaseIterator<T, IteratorImpl, V> &iter);

	template <typename T2, typename IteratorImpl2>
	bool operator== (const BaseIterator<T2, IteratorImpl2, V> &iter) const;

	template <typename T2, typename IteratorImpl2>
	bool operator!= (const BaseIterator<T2, IteratorImpl2, V> &iter) const;

	T& operator* () const;
	T* operator-> () const;

	IteratorImpl& operator++ ();
	IteratorImpl operator++ (int);

	IteratorImpl& operator-- ();
	IteratorImpl operator-- (int);

	IteratorImpl operator+ (ptrdiff_t offset) const;
	IteratorImpl operator- (ptrdiff_t offset) const;

	template <typename T2, typename IteratorImpl2>
	ptrdiff_t operator- (const BaseIterator<T2, IteratorImpl2, V> &another) const;

	template <typename T2, typename IteratorImpl2>
	bool operator< (const BaseIterator<T2, IteratorImpl2, V> &another) const;

	template <typename T2, typename IteratorImpl2>
	bool operator<= (const BaseIterator<T2, IteratorImpl2, V> &another) const;

	template <typename T2, typename IteratorImpl2>
	bool operator> (const BaseIterator<T2, IteratorImpl2, V> &another) const;

	template <typename T2, typename IteratorImpl2>
	bool operator>= (const BaseIterator<T2, IteratorImpl2, V> &another) const;

	IteratorImpl& operator+= (ptrdiff_t offset);
	IteratorImpl& operator-= (ptrdiff_t offset);

	T& operator[] (ptrdiff_t offset) const;

	template <typename T2, typename IteratorImpl2, typename V2>
	friend IteratorImpl2 operator+ (ptrdiff_t offset, const BaseIterator<T2, IteratorImpl2, V2> &iter);
};

#pragma region IteratorContainer implementation

template <typename IteratorImpl, typename V>
void IteratorContainer<IteratorImpl, V>::invalidateAll () {
	vector = nullptr;

	if (!headIterator) {
		delete this;
	}
}

template <typename IteratorImpl, typename V>
void IteratorContainer<IteratorImpl, V>::addIterator (IteratorImpl *iter) {
	//iter becomes the head

	if (!headIterator) { //no chain presenter
		headIterator = iter;
		iter->next = nullptr;
		iter->prev = nullptr;
	}
	else { //replace the existing chain presenter
		iter->prev = nullptr;
		iter->next = headIterator;
		headIterator->prev = iter;
		headIterator = iter;
	}
}

#pragma endregion

#pragma region BaseIterator implementation

template <typename T, typename IteratorImpl, typename V>
BaseIterator<T, IteratorImpl, V>::BaseIterator (T* ptr, IteratorContainer<IteratorImpl, V>* container) {
	#ifdef DEBUG_MODE
	std::cerr << typeid(*that()).name() << "(T*, Container*)" << std::endl;
	#endif

	prev = next = nullptr;
	this->ptr = ptr;
	this->container = container;
	this->container->addIterator (that());

	#ifdef MEMORY_TRACE_MODE
	watcher.onBaseIteratorPtrCreated();
	#endif
}

template <typename T, typename IteratorImpl, typename V>
BaseIterator<T, IteratorImpl, V>::BaseIterator () {
	#ifdef DEBUG_MODE
	std::cerr << typeid(*that()).name() << "()" << std::endl;
	#endif

	prev = next = nullptr;
	ptr = nullptr;
	container = nullptr;

	#ifdef MEMORY_TRACE_MODE
	watcher.onBaseIteratorDefCreated();
	#endif
}

template <typename T, typename IteratorImpl, typename V>
BaseIterator<T, IteratorImpl, V>::BaseIterator (const BaseIterator<T, IteratorImpl, V> &iter) {
	#ifdef DEBUG_MODE
	std::cerr << typeid(*that()).name() << "(const &)" << std::endl;
	#endif
	
	prev = next = nullptr;
	ptr = iter.ptr;
	container = iter.container;
	if (container) {
		container->addIterator(that());
	}

	#ifdef MEMORY_TRACE_MODE
	watcher.onBaseIteratorCopyCreated();
	#endif
}

template <typename T, typename IteratorImpl, typename V>
BaseIterator<T, IteratorImpl, V>::~BaseIterator () {
	#ifdef DEBUG_MODE
	std::cerr << "~" << typeid(*that()).name() << "()" << std::endl;
	#endif

	selfRemoveFromContainer();

	#ifdef MEMORY_TRACE_MODE
	watcher.onBaseIteratorDestroyed();
	#endif
}

template <typename T, typename IteratorImpl, typename V>
void BaseIterator<T, IteratorImpl, V>::selfRemoveFromContainer () {
	//fixing neighbours' links
	if (prev) {
		prev->next = next;
	}
	if (next) {
		next->prev = prev;
	}

	//fixing chain presenter link
	if (!prev && container) {
		container->headIterator = next;
	}

	prev = next = nullptr;

	//killing container if vector is dead and no iterators left.
	if (container && container->isVectorDestroyed() && !container->headIterator) {
		delete container;
	}
	container = nullptr;
}

template <typename T, typename IteratorImpl, typename V>
inline IteratorImpl& BaseIterator<T, IteratorImpl, V>::operator= (const BaseIterator<T, IteratorImpl, V> &iter) {
	ptr = iter.ptr;

	if (container != iter.container) {
		if (container) {
			selfRemoveFromContainer();
		}

		container = iter.container;
		if (container) {
			container->addIterator(that());
		}
	}

	return *that();
}

template <typename T, typename IteratorImpl, typename V>
template <typename T2, typename IteratorImpl2>
bool BaseIterator<T, IteratorImpl, V>::operator== (const BaseIterator<T2, IteratorImpl2, V> &iter) const {
	checkValidity();
	iter.checkValidity();
	checkDomainEquality(iter);

	return ptr == iter.ptr;
}

template <typename T, typename IteratorImpl, typename V>
template <typename T2, typename IteratorImpl2>
inline bool BaseIterator<T, IteratorImpl, V>::operator!= (const BaseIterator<T2, IteratorImpl2, V> &iter) const {
	return !operator==(iter);
}

template <typename T, typename IteratorImpl, typename V>
T& BaseIterator<T, IteratorImpl, V>::operator* () const {
	return (*this)[0];
}

template <typename T, typename IteratorImpl, typename V>
T* BaseIterator<T, IteratorImpl, V>::operator-> () const {
	checkValidity();

	if (ptr < container->vector->getDataEnd()) {
		return ptr;
	}
	else {
		throw IteratorOutOfRangeException();
	}
}

template <typename T, typename IteratorImpl, typename V>
IteratorImpl& BaseIterator<T, IteratorImpl, V>::operator++ () {
	checkValidity();
	if (ptr < container->vector->getDataEnd()) {
		++ptr;
		return *that();
	}
	else {
		throw InvalidIteratorShiftException();
	}
}

template <typename T, typename IteratorImpl, typename V>
IteratorImpl BaseIterator<T, IteratorImpl, V>::operator++ (int) {
	IteratorImpl clone(*that());

	++*this;

	return clone;
}

template <typename T, typename IteratorImpl, typename V>
IteratorImpl& BaseIterator<T, IteratorImpl, V>::operator-- () {
	checkValidity();
	if (ptr > container->vector->getDataBegin()) {
		--ptr;
		return *that();
	}
	else {
		throw InvalidIteratorShiftException();
	}
}

template <typename T, typename IteratorImpl, typename V>
IteratorImpl BaseIterator<T, IteratorImpl, V>::operator-- (int) {
	IteratorImpl clone(*that());

	--*this;

	return clone;
}

template <typename T, typename IteratorImpl, typename V>
inline IteratorImpl BaseIterator<T, IteratorImpl, V>::operator+ (ptrdiff_t offset) const {
	IteratorImpl iter(*that());
	iter += offset;
	return iter;
}

template <typename T, typename IteratorImpl, typename V>
inline IteratorImpl BaseIterator<T, IteratorImpl, V>::operator- (ptrdiff_t offset) const {
	return *this + (-offset);
}

template <typename T, typename IteratorImpl, typename V>
template <typename T2, typename IteratorImpl2>
ptrdiff_t BaseIterator<T, IteratorImpl, V>::operator- (const BaseIterator<T2, IteratorImpl2, V> &another) const {
	checkValidity();
	another.checkValidity();
	checkDomainEquality (another);

	return ptr - another.ptr;
}

template <typename T, typename IteratorImpl, typename V>
template <typename T2, typename IteratorImpl2>
bool BaseIterator<T, IteratorImpl, V>::operator< (const BaseIterator<T2, IteratorImpl2, V> &another) const {
	checkValidity();
	another.checkValidity();
	checkDomainEquality (another);

	return ptr < another.ptr;
}

template <typename T, typename IteratorImpl, typename V>
template <typename T2, typename IteratorImpl2>
inline bool BaseIterator<T, IteratorImpl, V>::operator<= (const BaseIterator<T2, IteratorImpl2, V> &another) const {
	return !(another < *that());
}

template <typename T, typename IteratorImpl, typename V>
template <typename T2, typename IteratorImpl2>
inline bool BaseIterator<T, IteratorImpl, V>::operator> (const BaseIterator<T2, IteratorImpl2, V> &another) const {
	return another < *that();
}

template <typename T, typename IteratorImpl, typename V>
template <typename T2, typename IteratorImpl2>
inline bool BaseIterator<T, IteratorImpl, V>::operator>= (const BaseIterator<T2, IteratorImpl2, V> &another) const {
	return !(*that() < another);
}

template <typename T, typename IteratorImpl, typename V>
IteratorImpl& BaseIterator<T, IteratorImpl, V>::operator+= (ptrdiff_t offset) {
	checkValidity();
	if (offset >= 0 ? ((container->vector->getDataEnd() - ptr) >= offset)
					: ((ptr - container->vector->getDataBegin()) >= -offset)) {
		ptr += offset;
		return *that();
	}
	else {
		throw InvalidIteratorShiftException();
	}
}

template <typename T, typename IteratorImpl, typename V>
inline IteratorImpl& BaseIterator<T, IteratorImpl, V>::operator-= (ptrdiff_t offset) {
	return *this += (-offset);
}

template <typename T, typename IteratorImpl, typename V>
inline T& BaseIterator<T, IteratorImpl, V>::operator[] (ptrdiff_t offset) const {
	checkValidity();

	if (offset >= 0 ? ((container->vector->getDataEnd() - ptr) > offset)
					: ((ptr - container->vector->getDataBegin()) >= -offset)) {
		return *(ptr + offset);
	}
	else {
		if ((container->vector->getDataEnd() - ptr) == offset) { //iterator pointers to the end
			throw IteratorOutOfRangeException();
		}
		else { //shifting by given offset leads away from range
			throw InvalidIteratorShiftException();
		}
	}
}

template <typename T, typename IteratorImpl, typename V>
IteratorImpl operator+ (ptrdiff_t offset, const BaseIterator<T, IteratorImpl, V> &iter) {
	return iter + offset;
}

#pragma endregion

//Iterator & ConstIterator

template <typename T>
class Iterator;

template <typename T>
class ConstIterator : public BaseIterator<const T, ConstIterator<T>, Vector<T>> {
private:
	friend class Vector<T>;
	friend class Iterator<T>;

	ConstIterator (T* ptr, IteratorContainer<ConstIterator<T>, Vector<T>> *container)
		: BaseIterator<const T, ConstIterator<T>, Vector<T>>(ptr, container) { }

public:
	ConstIterator (const Iterator<T> &iter) : BaseIterator<const T, ConstIterator<T>, Vector<T>> (iter) { }

	ConstIterator () { }
	ConstIterator (const BaseIterator<const T, ConstIterator, Vector<T>> &iter) : BaseIterator<const T, ConstIterator<T>, Vector<T>>(iter) { }
};

template <typename T>
class Iterator : public BaseIterator<T, Iterator<T>, Vector<T>> {
private:
	friend class ConstIterator<T>;
	friend class Vector<T>;

	Iterator (T *ptr, IteratorContainer<Iterator<T>, Vector<T>> *container) : BaseIterator<T, Iterator<T>, Vector<T>> (ptr, container) { }
public:
	Iterator () { }
	Iterator (const BaseIterator<T, Iterator, Vector<T>> &iter) : BaseIterator<T, Iterator<T>, Vector<T>>(iter) { }

	operator BaseIterator<const T, ConstIterator<T>, Vector<T>> () const {
		if (this->isValid()) {
			return ConstIterator<T>(this->dataPointer(), this->iterContainer()->vector->constIteratorContainer);
		}
		else {
			return ConstIterator<T>();
		}
	}
};
