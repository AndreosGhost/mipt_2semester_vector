#pragma once

#include "Vector.h"
#include <iterator>
#include <stdexcept>

#define DEBUG_MODE false

template <typename T>
class Vector;

template <typename T, typename IteratorImpl, typename V>
class BaseIterator;

#pragma region exceptions

struct InvalidIteratorException : public std::exception {
	explicit InvalidIteratorException (const char* const &msg) : exception(msg) { }
	InvalidIteratorException () : exception ("Invalid iterator") { }
};

struct DifferentIteratorDomainException : public std::exception {
	explicit DifferentIteratorDomainException (const char* const &msg) : exception(msg) { }
	DifferentIteratorDomainException () : exception ("Iterators are bound to different vectors") { }
};

struct IteratorOutOfRangeException : public std::exception {
	explicit IteratorOutOfRangeException (const char* const &msg) : exception(msg) { }
	IteratorOutOfRangeException () : exception ("Iterator ptr out of range") { }
};

struct InvalidIteratorShiftException : public std::exception {
	explicit InvalidIteratorShiftException (const char* const &msg) : exception(msg) { }
	InvalidIteratorShiftException () : exception ("Iterator shift will lead away from range") { }
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
	IteratorContainer (IteratorContainer<IteratorImpl, V> &) {
		throw runtime_error ("Uncopiable");
	}
protected:
	template <typename T, typename IteratorImpl, typename V>
	friend class BaseIterator;

	template <typename T>
	friend class Vector;

	template <typename T>
	friend class Iterator;

	IteratorImpl *headIterator;
	V *vector;

public:
	IteratorContainer (V *host) {
		headIterator = nullptr;
		vector = host;
		if (DEBUG_MODE) {
			cout << "IteratorContainer<" << typeid(IteratorImpl).name() << ">(V*)" << endl;
		}
	}

	~IteratorContainer () {
		if (DEBUG_MODE) {
			cout << "~IteratorContainer<" << typeid(IteratorImpl).name() << ">()" << endl;
		}
	}

protected:
	void addIterator (IteratorImpl *iter);
	void invalidateAll ();

	//called by vector from its destructor
	void onVectorDestroy ();

	//is vector already destroyed
	bool isVectorDestroyed () { return vector == nullptr; }
};

//Base class for const and non-const iterator.
//When created within a real vector, registers itself at IteratorContainer.
template <typename T, typename IteratorImpl, typename V>
class BaseIterator : std::iterator<std::random_access_iterator_tag, T> {
protected:
	T *ptr; //ptr inside the vector
	IteratorContainer<IteratorImpl, V> *container; //all iterators of this type bound to the same vector
	IteratorImpl *next; //next iterator in double-linked list (also in container)
	IteratorImpl *prev; //prev iterator in double-linked list (also in container)
	bool valid;

	template <typename IteratorImpl, typename V>
	friend class IteratorContainer;

	BaseIterator (T *ptr, IteratorContainer<IteratorImpl, V> *container);

	IteratorImpl* that() {
		return static_cast<IteratorImpl*>(this);
	}

	const IteratorImpl* that () const {
		return static_cast<const IteratorImpl*>(this);
	}
	
	//check if our ptr is valid
	void checkValidity () const throw (InvalidIteratorException) {
		if (!valid) {
			throw InvalidIteratorException();
		}
	}

	//check if this and another are bound to the same vector
	void checkDomainEquality (const BaseIterator<T, IteratorImpl, V> &another) const throw (DifferentIteratorDomainException) {
		if (container->vector != another.container->vector) {
			throw DifferentIteratorDomainException();
		}
	}

public:
	BaseIterator ();
	BaseIterator (const BaseIterator<T, IteratorImpl, V> &iter);
	
	~BaseIterator ();

	IteratorImpl &operator= (const BaseIterator<T, IteratorImpl, V> &iter);

	template <typename IteratorImpl2>
	bool operator== (const BaseIterator<T, IteratorImpl2, V> &iter) const;

	template <typename IteratorImpl2>
	bool operator!= (const BaseIterator<T, IteratorImpl2, V> &iter) const;

	const T& operator* () const;
	const T* operator-> () const;

	T& operator* ();

	IteratorImpl operator++ ();
	IteratorImpl& operator++ (int);

	IteratorImpl operator-- ();
	IteratorImpl& operator-- (int);

	IteratorImpl operator+ (int offset) const;
	IteratorImpl operator- (int offset) const;

	ptrdiff_t operator- (const BaseIterator<T, IteratorImpl, V> &another) const;

	bool operator< (const BaseIterator<T, IteratorImpl, V> &another) const;
	bool operator<= (const BaseIterator<T, IteratorImpl, V> &another) const;
	bool operator> (const BaseIterator<T, IteratorImpl, V> &another) const;
	bool operator>= (const BaseIterator<T, IteratorImpl, V> &another) const;

	IteratorImpl& operator+= (int offset);
	IteratorImpl& operator-= (int offset);

	T& operator[] (int offset) const;
	void swap (BaseIterator<T, IteratorImpl, V> &another);
};

#pragma region IteratorContainer implementation

template <typename IteratorImpl, typename V>
void IteratorContainer<IteratorImpl, V>::onVectorDestroy () {
	vector = nullptr;
	invalidateAll();
	if (headIterator == nullptr) {
		delete this;
	}
}

template <typename IteratorImpl, typename V>
void IteratorContainer<IteratorImpl, V>::invalidateAll () {
	//as soon as newer iterators are in the head...
	
	IteratorImpl *it = headIterator;
	while (it != nullptr && it->valid) {
		it->valid = false;
		it = it->next;
	}
}

template <typename IteratorImpl, typename V>
void IteratorContainer<IteratorImpl, V>::addIterator (IteratorImpl *iter) {
	//iter becomes the head
	
	if (headIterator == nullptr) {
		headIterator = iter;
		iter->next = nullptr;
		iter->prev = nullptr;
	}
	else {
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
	if (DEBUG_MODE) {
		cout << typeid(*that()).name() << "(T*, Container*)" << endl;
	}

	this->ptr = ptr;
	this->container = container;
	this->container->addIterator (that());
	
	valid = true;
}

template <typename T, typename IteratorImpl, typename V>
BaseIterator<T, IteratorImpl, V>::BaseIterator () {
	if (DEBUG_MODE) {
		cout << typeid(*that()).name() << "()" << endl;
	}

	valid = false;
}

template <typename T, typename IteratorImpl, typename V>
BaseIterator<T, IteratorImpl, V>::BaseIterator (const BaseIterator<T, IteratorImpl, V> &iter) {
	if (DEBUG_MODE) {
		cout << typeid(*that()).name() << "(const &)" << endl;
	}
	
	ptr = iter.ptr;
	container = iter.container;
	container->addIterator(that());
	valid = true;
}

template <typename T, typename IteratorImpl, typename V>
BaseIterator<T, IteratorImpl, V>::~BaseIterator () {
	if (DEBUG_MODE) {
		cout << "~" << typeid(*that()).name() << "()" << endl;
	}

	if (prev != nullptr) {
		prev->next = this->next;
	}
	if (next != nullptr) {
		next->prev = this->prev;
	}
	if (prev == nullptr) {
		container->headIterator = next;
	}
	if (container->isVectorDestroyed() && container->headIterator == nullptr) {
		delete container;
	}
}

template <typename T, typename IteratorImpl, typename V>
inline IteratorImpl& BaseIterator<T, IteratorImpl, V>::operator= (const BaseIterator<T, IteratorImpl, V> &iter) {
	swap(IteratorImpl(iter));
	return *that();
}

template <typename T, typename IteratorImpl, typename V>
template <typename IteratorImpl2>
bool BaseIterator<T, IteratorImpl, V>::operator== (const BaseIterator<T, IteratorImpl2, V> &iter) const {
	checkValidity();
	iter.checkValidity();
	this->checkDomainEquality(iter);

	return ptr == iter.ptr;
}

template <typename T, typename IteratorImpl, typename V>
template <typename IteratorImpl2>
inline bool BaseIterator<T, IteratorImpl, V>::operator!= (const BaseIterator<T, IteratorImpl2, V> &iter) const {
	return !operator==(iter);
}

template <typename T, typename IteratorImpl, typename V>
const T& BaseIterator<T, IteratorImpl, V>::operator* () const {
	checkValidity();

	if (ptr < container->vector->getDataEnd()) {
		return static_cast<const T&>(*ptr);
	}
	else {
		throw IteratorOutOfRangeException();
	}
}

template <typename T, typename IteratorImpl, typename V>
T& BaseIterator<T, IteratorImpl, V>::operator* () {
	checkValidity();

	if (ptr < container->vector->getDataEnd()) {
		return *ptr;
	}
	else {
		throw IteratorOutOfRangeException();
	}
}

template <typename T, typename IteratorImpl, typename V>
const T* BaseIterator<T, IteratorImpl, V>::operator-> () const {
	checkValidity();

	if (ptr < container->vector->getDataEnd()) {
		return ptr;
	}
	else {
		throw IteratorOutOfRangeException();
	}
}

template <typename T, typename IteratorImpl, typename V>
IteratorImpl BaseIterator<T, IteratorImpl, V>::operator++ () {
	IteratorImpl clone(*that());
	
	this->operator++(1);

	return clone;
}

template <typename T, typename IteratorImpl, typename V>
IteratorImpl& BaseIterator<T, IteratorImpl, V>::operator++ (int) {
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
IteratorImpl BaseIterator<T, IteratorImpl, V>::operator-- () {
	IteratorImpl clone(*that());
	
	this->operator--(1);

	return clone;
}

template <typename T, typename IteratorImpl, typename V>
IteratorImpl& BaseIterator<T, IteratorImpl, V>::operator-- (int) {
	checkValidity();
	if (ptr > container->vector.getDataBegin()) {
		--ptr;
		return *that();
	}
	else {
		throw InvalidIteratorShiftException();
	}
}

template <typename T, typename IteratorImpl, typename V>
inline IteratorImpl BaseIterator<T, IteratorImpl, V>::operator+ (int offset) const {
	IteratorImpl iter(*that());
	iter += offset;
	return iter;
}

template <typename T, typename IteratorImpl, typename V>
inline IteratorImpl operator+ (int offset, const BaseIterator<T, IteratorImpl, V> &iter) {
	return iter.operator+ (offset);
}

template <typename T, typename IteratorImpl, typename V>
inline IteratorImpl BaseIterator<T, IteratorImpl, V>::operator- (int offset) const {
	return operator+ (-offset);
}

template <typename T, typename IteratorImpl, typename V>
ptrdiff_t BaseIterator<T, IteratorImpl, V>::operator- (const BaseIterator<T, IteratorImpl, V> &another) const {
	checkValidity();
	another.checkValidity();
	checkDomainEquality (another);

	return ptr - another.ptr;
}

template <typename T, typename IteratorImpl, typename V>
bool BaseIterator<T, IteratorImpl, V>::operator< (const BaseIterator<T, IteratorImpl, V> &another) const {
	checkValidity();
	another.checkValidity();
	checkDomainEquality (another);

	return ptr < another.ptr;
}

template <typename T, typename IteratorImpl, typename V>
inline bool BaseIterator<T, IteratorImpl, V>::operator<= (const BaseIterator<T, IteratorImpl, V> &another) const {
	return *that() == another || *that() < another;
}

template <typename T, typename IteratorImpl, typename V>
inline bool BaseIterator<T, IteratorImpl, V>::operator> (const BaseIterator<T, IteratorImpl, V> &another) const {
	return another < *that();
}

template <typename T, typename IteratorImpl, typename V>
inline bool BaseIterator<T, IteratorImpl, V>::operator>= (const BaseIterator<T, IteratorImpl, V> &another) const {
	return another == *that() || another < *that();
}

template <typename T, typename IteratorImpl, typename V>
IteratorImpl& BaseIterator<T, IteratorImpl, V>::operator+= (int offset) {
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
inline IteratorImpl& BaseIterator<T, IteratorImpl, V>::operator-= (int offset) {
	return operator+= (-offset);
}

template <typename T, typename IteratorImpl, typename V>
inline T& BaseIterator<T, IteratorImpl, V>::operator[] (int offset) const {
	return *(that() + offset);
}

template <typename T, typename IteratorImpl, typename V>
inline void BaseIterator<T, IteratorImpl, V>::swap (BaseIterator<T, IteratorImpl, V> &another) {
	std::swap(*that(), another);
}

#pragma endregion

//Iterator & ConstIterator

template <typename T>
class Iterator;

template <typename T>
class ConstIterator : public BaseIterator<const T, ConstIterator<T>, Vector<T>> {
protected:
	friend class Vector<T>;
	friend class Iterator<T>;
	
	
public:
	ConstIterator (T* ptr, IteratorContainer<ConstIterator<T>, Vector<T>> *container)
		: BaseIterator<const T, ConstIterator<T>, Vector<T>>(ptr, container) { }

	ConstIterator () { }
	ConstIterator (BaseIterator<T, ConstIterator, Vector<T>> &iter) : BaseIterator(iter) { }
};

template <typename T>
class Iterator : public BaseIterator<T, Iterator<T>, Vector<T>> {
protected:
	friend class ConstIterator<T>;
	friend class Vector<T>;

	Iterator (T *ptr, IteratorContainer<Iterator<T>, Vector<T>> *container) : BaseIterator (ptr, container) { }
public:
	Iterator () { }
	Iterator (const BaseIterator<T, Iterator, Vector<T>> &iter) : BaseIterator(iter) { }

	operator ConstIterator<T> () const {
		return ConstIterator<T>(ptr, container->vector->getConstIteratorContainer());
	}
};