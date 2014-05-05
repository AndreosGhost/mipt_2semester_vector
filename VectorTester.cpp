#define MEMORY_TRACE_MODE

#include "MemoryWatcher.h"
#include "Vector.h"
#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <cstdlib>
#include <list>

using namespace std;

#pragma region classes

class IntIncapsulator {
protected:
	int value;
public:
	explicit IntIncapsulator (int val) : value(val) { }
	~IntIncapsulator () { }

	inline bool operator== (const IntIncapsulator &obj) const { return value == obj.value; }
	inline bool operator!= (const IntIncapsulator &obj) const { return !(*this == obj); }
	inline int getValue () const { return value; }
};

class TypeWithoutDefCtor : public IntIncapsulator {
public:
	explicit TypeWithoutDefCtor (int val) : IntIncapsulator(val) { }
	TypeWithoutDefCtor (const TypeWithoutDefCtor &another) : IntIncapsulator(another.value) { }
	TypeWithoutDefCtor& operator= (const TypeWithoutDefCtor &another) {
		value = another.value;
		return *this;
	}
};

class TypeUncopiable : public TypeWithoutDefCtor {
private:
    #if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 180000000
    TypeUncopiable (const TypeUncopiable &another);
	TypeUncopiable& operator= (const TypeUncopiable &another);
    #else
    TypeUncopiable (const TypeUncopiable &another) = delete;
	TypeUncopiable& operator= (const TypeUncopiable &another) = delete;
	#endif
public:
	TypeUncopiable () : TypeWithoutDefCtor(0) { }
	TypeUncopiable (int val) : TypeWithoutDefCtor(val) { }
	TypeUncopiable (TypeUncopiable &&another) : TypeWithoutDefCtor(another) { }
};

#pragma endregion

#pragma region utility

//used to in obligitary 'throw' construction when no exception is expected
//(in invalid operations tests, for example)
class ExceptionEmulator : public exception { };

void failTest () {
	cout << "Testing failed: please consider debugging" << endl;
	throw runtime_error ("Test failed");
}

//Calls 'action'. If exception of type 'Exception' is thrown, silently returns. If no - writes a message.
template <typename Exception>
void testException (function<void (void)> action, const char* actionName) {
	bool caughtSomething = false;

	try {
		action();
	}
	catch (const Exception &ex) {
		caughtSomething = true;
	}
	catch (...) {
		cout << "Error! Got unexpected exception in action: " << actionName << endl;
		cout << "Expected exception: " << typeid(Exception).name() << endl;
		caughtSomething = true;
		failTest();
	}
	if (!caughtSomething) {
		cout << "Error! Got no expected in action: " << actionName << endl;
		cout << "Expected exception: " << typeid(Exception).name() << endl;
		failTest();
	}
}

template <typename Number>
int random (Number a, Number b) {
	return a + static_cast<Number>(rand()) % (b - a + 1);
}

template <typename V1, typename V2>
bool areEqual (const V1 &a, const V2 &b) {
	if (a.size() != b.size()) {
		return false;
	}
	for (size_t sz = a.size(), i = 0; i < sz; ++i) {
		if (!(a[i] == b[i])) {
			return false;
		}
	}

	return true;
}

ostream& operator<< (ostream &s, const IntIncapsulator &obj) {
	s << obj.getValue();
	return s;
}

template <typename T>
ostream& operator<< (ostream &s, const vector<T> &vector) {
	s << "[";
	bool comma = false;

	for (size_t sz = vector.size(), i = 0; i < sz; ++i) {
		if (comma) {
			s << ", ";
		}
		comma = true;
		s << vector[i];
	}

	s << "]";

	return s;
}

template <typename T>
ostream& operator<< (ostream &s, const Vector<T> &vector) {
	s << "[";
	bool comma = false;

	for (size_t sz = vector.size(), i = 0; i < sz; ++i) {
		if (comma) {
			s << ", ";
		}
		comma = true;
		s << vector[i];
	}

	s << "]";

	return s;
}

#pragma endregion

#pragma region fillVector

//generic

template <typename T>
void fillVector (vector<T> &v, size_t length) {
	v.clear();

	for (size_t i = 0; i < length; ++i) {
		v.push_back(move(T(random(1, 100000))));
	}
}

template <typename T>
void fillVector (Vector<T> &v, size_t length) {
	v.clear();
	v.reserve(length);

	for (size_t i = 0; i < length; ++i) {
		v.push_back(move(T(random(1, 100000))));
	}
}

template <typename T>
void fillVector (Vector<T> &myVector, const vector<T> &sysVector) {
	myVector.clear();

	for (size_t i = 0, sz = sysVector.size(); i < sz; ++i) {
		myVector.push_back(move(T(sysVector[i].getValue())));
	}
}

template <typename T>
void fillVector (vector<T> &dst, const vector<T> &src) {
	dst.clear();

	for (size_t i = 0, sz = src.size(); i < sz; ++i) {
		dst.push_back(move(T(src[i].getValue())));
	}
}

//with int

template <>
void fillVector<int> (vector<int> &v, size_t length) {
	v.clear();

	for (size_t i = 0; i < length; ++i) {
		v.push_back(random(1, 100000));
	}
}

template <>
void fillVector<int> (Vector<int> &v, size_t length) {
	v.clear();

	for (size_t i = 0; i < length; ++i) {
		v.push_back(random(1, 100000));
	}
}

template <>
void fillVector<int> (vector<int> &dst, const vector<int> &src) {
	dst.clear();

	for (size_t i = 0, sz = src.size(); i < sz; ++i) {
		dst.push_back(src[i]);
	}
}

template <>
void fillVector<int> (Vector<int> &myVector, const vector<int> &sysVector) {
	myVector.clear();

	for (size_t i = 0, sz = sysVector.size(); i < sz; ++i) {
		myVector.push_back(sysVector[i]);
	}
}

//with Vector<int>

template <>
void fillVector<Vector<int>> (Vector<Vector<int>> &v, size_t length) {
	v.clear();

	for (size_t i = 0; i < length; ++i) {
		Vector<int> temp;
		fillVector(temp, random<size_t>(0, length));
		v.push_back(move(temp));
	}
}

template <>
void fillVector<Vector<int>> (vector<Vector<int>> &v, size_t length) {
	v.clear();

	for (size_t i = 0; i < length; ++i) {
		Vector<int> temp;
		fillVector(temp, random<size_t>(0, length));
		v.push_back(move(temp));
	}
}

template <>
void fillVector<Vector<int>> (Vector<Vector<int>> &myVector, const vector<Vector<int>> &sysVector) {
	myVector.clear();

	for (size_t i = 0, sz = sysVector.size(); i < sz; ++i) {
		myVector.push_back(sysVector[i]);
	}
}

template <>
void fillVector<Vector<int>> (vector<Vector<int>> &dst, const vector<Vector<int>> &src) {
	dst.clear();

	for (size_t i = 0, sz = src.size(); i < sz; ++i) {
		dst.push_back(src[i]);
	}
}

#pragma endregion

#pragma region test Vector

template <typename T>
void testCopyConstructor () {
	cout << endl << ">>>" << "testCopyConstructor()" << endl;

	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(0, 20));
	fillVector(myVector, sysVector);

	Vector<T> v_copy(myVector);

	if (!areEqual(sysVector, v_copy)) {
		cout << "error: bad copy constructor" << endl;
		cout << "sys. vector: " << sysVector << endl;
		cout << "src. vector: " << myVector << endl;
		cout << "copy vector: " << v_copy << endl;
		failTest();
	}
}

template <typename T>
void testMoveConstructor () {
	cout << endl << ">>>" << "testMoveConstructor()" << endl;

	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(0, 20));
	fillVector(myVector, sysVector);

	Vector<T> v_move(move(myVector));

	if (!areEqual(sysVector, v_move) || !myVector.empty()) {
		cout << "error: bad move constructor" << endl;
		cout << "sys. vector: " << sysVector << endl;
		cout << "src. vector: " << myVector << endl;
		cout << "move vector: " << v_move << endl;
		failTest();
	}
}

template <typename T>
void testIteratorConstructor () {
	cout << endl << ">>>" << "testIteratorConstructor()" << endl;

	//with std::vector
	vector<T> sysVector;

	fillVector(sysVector, random(0, 20));

	Vector<T> v_copy(sysVector.begin(), sysVector.end());

	if (!areEqual(sysVector, v_copy)) {
		cout << "error with std::vector: bad iterator constructor" << endl;
		cout << "sys. vector: " << sysVector << endl;
		cout << "copy vector: " << v_copy << endl;
		failTest();
	}

	//with std::list
	std::list<T> sysList (sysVector.begin(), sysVector.end());

	v_copy = Vector<T>(sysList.begin(), sysList.end());
	if (!areEqual(sysVector, v_copy)) {
		cout << "error with std::list: bad iterator constructor" << endl;
		cout << "sys. list: " << sysVector << endl;
		cout << "copy vector: " << v_copy << endl;
		failTest();
	}
}

template <typename T>
void testSetOperatorLValue () {
	cout << endl << ">>>" << "testSetOperatorLValue()" << endl;

	Vector<T> myVector1;
	Vector<T> myVector2;
	vector<T> sysVector1;
	vector<T> sysVector2;

	fillVector(sysVector1, random(0, 20));
	fillVector(myVector1, sysVector1);
	fillVector(sysVector2, random(0, 20));
	fillVector(myVector2, sysVector2);

	myVector1 = myVector2;
	if (!areEqual(sysVector2, myVector1) || !areEqual(sysVector2, myVector2)) {
		cout << "error: bad operator= lvalue" << endl;
		cout << "sys. src vector: " << sysVector2 << endl;
		cout << "sys. dst vector: " << sysVector1 << endl;
		cout << "my src. vector (must be empty): " << myVector2 << endl;
		cout << "my dst. vector (must be = sys src.): " << myVector1 << endl;
		failTest();
	}
}

template <typename T>
void testSetOperatorRValue () {
	cout << endl << ">>>" << "testSetOperatorRValue()" << endl;

	Vector<T> myVector1;
	Vector<T> myVector2;
	vector<T> sysVector1;
	vector<T> sysVector2;

	fillVector(sysVector1, random(0, 20));
	fillVector(myVector1, sysVector1);
	fillVector(sysVector2, random(0, 20));
	fillVector(myVector2, sysVector2);

	myVector1 = move(myVector2);
	if (!areEqual(sysVector2, myVector1) || !myVector2.empty()) {
		cout << "error: bad operator= rvalue" << endl;
		cout << "sys. src vector: " << sysVector2 << endl;
		cout << "sys. dst vector: " << sysVector1 << endl;
		cout << "my src. vector (must be empty): " << myVector2 << endl;
		cout << "my dst. vector (must be = sys src.): " << myVector1 << endl;
		failTest();
	}
}

template <typename T>
void testGetAndSet () {
	cout << endl << ">>>" << "testGetAndSet()" << endl;

	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(0, 20));
	fillVector(myVector, sysVector);

	if (!areEqual(sysVector, myVector)) {
		cout << "error: bad get&set [] operators" << endl;
		cout << "sys. vector: " << sysVector << endl;
		cout << "src. vector: " << myVector << endl;
		failTest();
	}
}

template <typename T>
void testSwap () {
	cout << endl << ">>>" << "testSwap()" << endl;

	Vector<T> myVector1;
	Vector<T> myVector2;
	vector<T> sysVector1;
	vector<T> sysVector2;

	fillVector(sysVector1, random(0, 20));
	fillVector(myVector1, sysVector1);
	fillVector(sysVector2, random(0, 20));
	fillVector(myVector2, sysVector2);
	
	myVector1.swap(myVector2);

	if (!areEqual (sysVector1, myVector2) || !areEqual (sysVector2, myVector1)) {
		cout << "error: bad swap()" << endl;
		cout << "sys. vector 1: " << sysVector1 << endl;
		cout << "sys. vector 2: " << sysVector2 << endl;
		cout << "swap. vector 1: " << myVector1 << endl;
		cout << "swap. vector 2: " << myVector2 << endl;
		failTest();
	}
}

template <typename T>
void testReserveAndShrinkToFit () {
	cout << endl << ">>>" << "testReserveAndShrinkToFit()" << endl;

	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(0, 20));
	fillVector(myVector, sysVector);

	myVector.reserve(random(myVector.size(), 10 * myVector.size()));

	if (!areEqual(sysVector, myVector)) {
		cout << "error: bad reserve()" << endl;
		cout << "sys. vector: " << sysVector << endl;
		cout << "src. vector: " << myVector << endl;
		failTest();
	}

	myVector.shrink_to_fit();

	//checking for consistency after shrink_to_fit
	if (!areEqual(sysVector, myVector)) {
		cout << "error: bad shrink_to_fit()" << endl;
		cout << "sys. vector: " << sysVector << endl;
		cout << "src. vector: " << myVector << endl;
		failTest();
	}
}

template <typename T>
void testPushBackLValue () {
	cout << endl << ">>>" << "testPushBackLValue()" << endl;

	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(0, 20));

	for (size_t i = 0, sz = sysVector.size(); i < sz; ++i) {
		myVector.push_back(sysVector[i]);
	}

	if (!areEqual(sysVector, myVector)) {
		cout << "error: bad push_back() lvalue" << endl;
		cout << "sys. vector: " << sysVector << endl;
		cout << "src. vector: " << myVector << endl;
		failTest();
	}
}

template <typename T>
void testPushBackRValue () {
	cout << endl << ">>>" << "testPushBackRValue()" << endl;

	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(0, 20));
	vector<T> sysCopy;
	fillVector (sysCopy, sysVector);

	for (size_t i = 0, sz = sysVector.size(); i < sz; ++i) {
		myVector.push_back(move(sysCopy[i]));
	}

	if (!areEqual(sysVector, myVector)) {
		cout << "error: bad push_back() rvalue" << endl;
		cout << "src. vector: " << sysVector << endl;
		cout << "my vector: " << myVector << endl;
		cout << "sys vector: " << sysCopy << endl;
		failTest();
	}
}

template <typename T>
void testPopBack () {
	cout << endl << ">>>" << "testPopBack()" << endl;

	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(0, 20));
	fillVector(myVector, sysVector);

	vector<T> sysCopy;
	fillVector (sysCopy, sysVector);

	while (!sysCopy.empty()) {
		sysCopy.pop_back();
		myVector.pop_back();

		if (!areEqual(sysCopy, myVector)) {
			cout << "error: bad pop_back()" << endl;
			cout << "src vector: " << sysVector << endl;
			cout << "my vector: " << myVector << endl;
			cout << "sys vector: " << sysCopy << endl;
			failTest();
		}
	}
}

template <typename T>
void testClear () {
	cout << endl << ">>>" << "testClear()" << endl;

	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(0, 20));
	fillVector(myVector, sysVector);

	myVector.clear();

	if (!myVector.empty()) {
		cout << "bad clear()" << endl;
		cout << "prev. data: " << sysVector;
		cout << "having: " << myVector;
		failTest();
	}
}

#pragma endregion

#pragma region test Iterators

template <typename T>
void testRangedFor () {
	cout << endl << ">>>" << "testRangedFor()" << endl;

	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(0, 20));
	fillVector(myVector, sysVector);

	typename vector<T>::iterator sysIt = sysVector.begin();
	typename vector<T>::iterator sysEnd = sysVector.end();

	for (T& element : myVector) {
		if (!(element == *sysIt)) {
			cout << "error: bad ranged for" << endl;
			cout << "my vector: " << myVector << endl;
			cout << "sys vector: " << sysVector << endl;
			failTest();
		}
		++sysIt;
	}


	Vector<typename Vector<T>::iterator> iterVector;
	for (size_t i = 0, sz = myVector.size(); i < sz; ++i) {
		iterVector.push_back(myVector.begin() + i);
	}

	for (size_t i = 0, sz = myVector.size(); i < sz; ++i) {
		if (!(*(iterVector[i]) == myVector[i])) {
			cout << "error: bad iterator created" << endl;
			cout << "my vector: " << myVector << endl;
			cout << "position: " << i << ", myValue = " << myVector[i] << ", iterValue = " << *(iterVector[i]) << endl;
			failTest();
		}
	}
}

template <typename T>
void testIteratorValidity () {
	cout << endl << ">>>" << "testIteratorValidity()" << endl;

	Vector<T> v;

	v.reserve(25);
	fillVector(v, random(5, 20));

    typename Vector<T>::iterator it = v.begin();

	auto checkValidity = [&](const string method_name) {
		try { *it; }
		catch (exception &ex) {
			cout << "Error! Iterator became invalid after calling method \"" << method_name << "\"" << endl;
			cout << "It was begin() iterator on vector: " << v << endl;
			cout << "Exception: " << ex.what() << endl;
			failTest();
		}
	};

	Vector<T> v2;
	fillVector(v2, random(5, 20));

	v.empty();
	checkValidity ("empty");
	v.size();
	checkValidity ("size");
	v.capacity();
	checkValidity ("capacity");
	v[0];
	checkValidity ("[] const");
	v.pop_back();
	checkValidity ("pop_back");
	v.push_back (v2[0]);
	checkValidity ("push_back");
}

template <typename T>
void testIteratorCasts () {
	cout << endl << ">>>" << "testIteratorCasts()" << endl;

	Vector<T> v;

	(typename Vector<T>::const_iterator)(v.begin());
	(typename Vector<T>::const_reverse_iterator)(v.rbegin());
}

template <typename T>
void testIteratorTraits () {
	cout << endl << ">>>" << "testIteratorTraits()" << endl;

	if (typeid(typename std::iterator_traits<typename Vector<T>::iterator>::value_type) != typeid(T)) {
		cout << "cannot determine iterator value type properly" << endl;
		cout << "iterator_traits::value_type = " << typeid(typename std::iterator_traits<typename Vector<T>::iterator>::value_type).name() << endl;
		cout << "T = " << typeid(T).name() << endl;
		failTest();
	}
	if (typeid(typename std::iterator_traits<typename Vector<T>::iterator>::iterator_category) != typeid(random_access_iterator_tag)) {
		cout << "iterator_traits::iterator_category = " << typeid(typename std::iterator_traits<typename Vector<T>::iterator>::iterator_category).name() << endl;
		failTest();
	}
	if (typeid(typename std::iterator_traits<typename Vector<T>::const_iterator>::value_type) != typeid(T)) {
		cout << "cannot determine const iterator value type properly" << endl;
		cout << "iterator_traits::value_type = " << typeid(typename std::iterator_traits<typename Vector<T>::const_iterator>::value_type).name() << endl;
		cout << "T = " << typeid(T).name() << endl;
		failTest();
	}
	if (typeid(typename std::iterator_traits<typename Vector<T>::const_iterator>::iterator_category) != typeid(random_access_iterator_tag)) {
		cout << "iterator_traits::iterator_category = " << typeid(typename std::iterator_traits<typename Vector<T>::const_iterator>::iterator_category).name() << endl;
		failTest();
	}
}

template <typename T>
void testReverseIterators () {
	cout << endl << ">>>" << "testReverseIterators()" << endl;

	Vector<T> v;

	fillVector (v, random(5, 20));

	typename Vector<T>::reverse_iterator revIter = v.rbegin(); //end
	typename Vector<T>::iterator iter = v.end() - 1;

	typename Vector<T>::iterator begin = v.begin();

	while (true) {
		if (!(*iter == *revIter)) {
			cout << "Error! Bad reverse_iterator!" << endl;
			cout << "*revIter = " << *revIter << ", *iter = " << *iter << endl;
			cout << "vector = " << v << endl;
			failTest();
		}
		if (iter == begin) {
			break;
		}

		iter--;
		revIter++;
	}
}

template <typename T>
void testIteratorUnaryIncrement () {
	cout << endl << ">>>" << "testIteratorUnaryIncrement()" << endl;

	Vector<T> v1;
	fillVector (v1, 3);

	Iterator<T> it = v1.begin();
	T value1 = *(++it);

	if (!(value1 == *(v1.begin() + 1)) || it != v1.begin() + 1) {
		cout << "Error! bad prefix unary increment operator!" << endl;
		cout << "value1 = " << value1 << ", *it = " << *it << ", vector = " << v1 << endl;
		failTest();
	}

	it = v1.begin();
	T value0 = *(it++);

	if (!(value0 == *v1.begin()) || it != v1.begin() + 1) {
		cout << "Error! bad postfix unary increment operator!" << endl;
		cout << "value0 = " << value0 << ", *it = " << *it << ", vector = " << v1 << endl;
		failTest();
	}
}

template <typename T>
void testIteratorOperations () {
	cout << endl << ">>>" << "testIteratorOperations()" << endl;

	Vector<T> v1;
	fillVector(v1, random(3, 20));
	Vector<T> v2;
	fillVector(v2, random(3, 20));

	testException<DifferentIteratorDomainException>([&](){ v1.begin() - v2.begin(); }, "v1.begin - v2.begin");
	testException<DifferentIteratorDomainException>([&](){ v1.begin() >= v2.begin(); }, "v1.begin >= v2.begin");

	testException<InvalidIteratorShiftException>([&](){ v1.begin() + 1000; }, "v1.begin + 1000");
	testException<InvalidIteratorShiftException>([&](){ v1.begin() - 1; }, "v1.begin - 1");
	testException<InvalidIteratorShiftException>([&](){ v1.end() + 1; }, "v1.end + 1");

	//won't compile
	//testException<ExceptionEmulator>([&](){ *v1.cbegin() = *v2.cbegin(); }, "*v1.cbegin = *v2.begin");

	testException<ExceptionEmulator>([&](){ *v1.begin() = *v2.begin(); throw ExceptionEmulator(); }, "*v1.begin = *v2.begin");
	testException<ExceptionEmulator>([&](){ *v1.begin() = *v2.cbegin(); throw ExceptionEmulator(); }, "*v1.begin = *v2.cbegin");
	testException<ExceptionEmulator>([&](){ v1.end() - 1; throw ExceptionEmulator(); }, "v1.end() - 1; throw 0");
	testException<ExceptionEmulator>([&](){ *(v1.begin() + 1); throw ExceptionEmulator(); }, "*(v1.begin + 1); throw 0");
	testException<ExceptionEmulator>([&](){ Iterator<T> it = v1.begin(); if (*it == *v1.begin()) throw ExceptionEmulator(); else throw "!"; }, "it = v1.begin; *it == *v1.begin?");

	testException<IteratorOutOfRangeException>([&](){ *v1.end();}, "*v1.end");

	typename Vector<T>::iterator it = v1.begin();
	v1.reserve(100);
	testException<InvalidIteratorException>([&](){ *it; }, "*it");

	testException<InvalidIteratorException>([&](){ it == v1.begin(); }, "it == v1.begin");

	testException<IndexOutOfRangeException>([&](){ v1[1000]; }, "v1[1000]");

	v1.clear();
	testException<InvalidOperationException>([&](){ v1.pop_back(); }, "v1.pop_back()");
}

#pragma endregion

template <typename T>
void test () {
	//search for leaks is performed after each test unit.

	testGetAndSet<T>();					watcher.checkTotalConsistency();
	testCopyConstructor<T>();			watcher.checkTotalConsistency();
	testMoveConstructor<T>();			watcher.checkTotalConsistency();
	testIteratorConstructor<T>();		watcher.checkTotalConsistency();
	testSetOperatorLValue<T>();			watcher.checkTotalConsistency();
	testSetOperatorRValue<T>();			watcher.checkTotalConsistency();
	testPopBack<T>();					watcher.checkTotalConsistency();
	testPushBackLValue<T>();			watcher.checkTotalConsistency();
	testPushBackRValue<T>();			watcher.checkTotalConsistency();
	testReserveAndShrinkToFit<T>();		watcher.checkTotalConsistency();
	testSwap<T>();						watcher.checkTotalConsistency();
	testClear<T>();						watcher.checkTotalConsistency();

	testIteratorValidity<T>();			watcher.checkTotalConsistency();
	testRangedFor<T>();					watcher.checkTotalConsistency();
	testIteratorCasts<T>();				watcher.checkTotalConsistency();
	testIteratorTraits<T>();			watcher.checkTotalConsistency();
	testIteratorOperations<T>();		watcher.checkTotalConsistency();
	testIteratorUnaryIncrement<T>();	watcher.checkTotalConsistency();
	testReverseIterators<T>();			watcher.checkTotalConsistency();
}

template <>
void test<TypeUncopiable> () {
	typedef TypeUncopiable T;

	testGetAndSet<T>();					watcher.checkTotalConsistency();
	testMoveConstructor<T>();			watcher.checkTotalConsistency();
	testSetOperatorRValue<T>();			watcher.checkTotalConsistency();
	testPopBack<T>();					watcher.checkTotalConsistency();
	testPushBackRValue<T>();			watcher.checkTotalConsistency();
	testSwap<T>();						watcher.checkTotalConsistency();
	testClear<T>();						watcher.checkTotalConsistency();

	testRangedFor<T>();					watcher.checkTotalConsistency();
	testIteratorCasts<T>();				watcher.checkTotalConsistency();
	testIteratorTraits<T>();			watcher.checkTotalConsistency();
	testReverseIterators<T>();			watcher.checkTotalConsistency();
}

int main() {

	for (size_t t = 0; t < 100; ++t) {
		cout << endl << endl << "\t\tTEST " << t << endl << endl;

		cout << endl << "Testing Vector<TypeWithoutDefCtor>" << endl;
		test<TypeWithoutDefCtor>();

		cout << endl << endl << "Testing Vector<int>" << endl;
		test<int>();

		cout << endl << "Testing Vector<Vector<int>>" << endl;
		test<Vector<int>>();

		cout << endl << "Testing Vector<TypeUncopiable>" << endl;
		test<TypeUncopiable>();

		cout << endl << "Testing over" << endl;
	}

	return 0;
}
