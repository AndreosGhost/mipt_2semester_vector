#include "Vector.h"
#include <string>
#include <iostream>
#include <vector>
#include <functional>

using namespace std;

#pragma region classes

class ForbiddenCallException : public exception {
public:
	ForbiddenCallException () : exception ("forbidden call") { }
	ForbiddenCallException (const char* msg) : exception(msg) { }
};

class IntIncapsulator {
protected:
	int value;
public:
	explicit IntIncapsulator (int val) : value(val) {
		/*if (DEBUG_MODE) {
			cerr << "initializing with val = " << value << endl;
		}*/
	}
	~IntIncapsulator () {
		/*if (DEBUG_MODE) {
			cerr << "destructing with val = " << value << endl;
		}*/
	}

	inline bool operator== (const IntIncapsulator &obj) const { return value == obj.value; }
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
	TypeWithoutDefCtor (TypeWithoutDefCtor &&another) : IntIncapsulator(move(another.value)) { }
	TypeWithoutDefCtor& operator= (TypeWithoutDefCtor &&another) {
		value = move(another.value);
	}
};

class TypeUncopiable : public TypeWithoutDefCtor {
private:
	explicit TypeUncopiable (const TypeUncopiable &another) : TypeWithoutDefCtor(another.value) {
		cout << "inside TypeUncopiable(const &)" << endl;
		throw ForbiddenCallException();
	}
	TypeUncopiable& operator= (const TypeUncopiable &another) {
		cout << "inside TypeUncopiable::operator= (const &)" << endl;
		throw ForbiddenCallException();
	}
	
public:
	TypeUncopiable () : TypeWithoutDefCtor(0) { }
	TypeUncopiable (int val) : TypeWithoutDefCtor(val) { }	

	TypeUncopiable (TypeUncopiable &&another) : TypeWithoutDefCtor(another) { }
	TypeUncopiable& operator= (const TypeUncopiable &&another) {
		TypeWithoutDefCtor::operator=(another);
	}
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

//Calls 'action'. If exception of type 'Exception' is thrown, returns. If no - writes a message.
template <typename Exception>
void testException (function<void (void)> action, const char* actionName) {
	bool caughtSomething = false;

	try {
		action();
	}
	catch (const Exception &ex) {
		caughtSomething = true;
	}
	catch (const exception &ex) {
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

template <typename T>
bool operator== (const Vector<T> &v1, const Vector<T> &v2) {
	return areEqual(v1, v2);
}

template <typename T>
bool operator== (const vector<T> &sysVector, const Vector<T> &myVector) {
	return areEqual(sysVector, myVector);
}

template <typename T>
bool operator== (const Vector<T> &myVector, const vector<T> &sysVector) {
	return areEqual(sysVector, myVector);
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

	vector<T> sysVector;

	fillVector(sysVector, random(0, 20));

	Vector<T> v_copy(sysVector.begin(), sysVector.end());

	if (!areEqual(sysVector, v_copy)) {
		cout << "error: bad iterator constructor" << endl;
		cout << "sys. vector: " << sysVector << endl;
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

	Vector<T>::iterator it1 = myVector1.begin();
	Vector<T>::iterator it2 = myVector2.begin();

	myVector1.swap(myVector2);
	
	if (!areEqual (sysVector1, myVector2) || !areEqual (sysVector2, myVector1) ||
		!(myVector1.size() == 0 || *it2 == myVector1[0]) ||
		!(sysVector2.size() == 0 || *it2 == sysVector2[0]) ||
		!(myVector2.size() == 0 || *it1 == myVector2[0]) ||
		!(sysVector1.size() == 0 || *it1 == sysVector1[0])) {
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

	vector<T>::iterator sysIt = sysVector.begin();
	vector<T>::iterator sysEnd = sysVector.end();

	for (T& element : myVector) {
		if (!(element == *sysIt)) {
			cout << "error: bad ranged for" << endl;
			cout << "my vector: " << myVector << endl;
			cout << "sys vector: " << sysVector << endl;
			failTest();
		}
		++sysIt;
	}


	Vector<Vector<T>::iterator> iterVector;
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

	Vector<T>::iterator it = v.begin();

	auto checkValidity = [&](char* method_name) {
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
	v.swap(v2);
	checkValidity ("swap");
}

template <typename T>
void testIteratorCasts () {
	cout << endl << ">>>" << "testIteratorCasts()" << endl;

	Vector<T> v;

	(Vector<T>::const_iterator)(v.begin());
	(Vector<T>::const_reverse_iterator)(v.rbegin());
}

template <typename T>
void testIteratorTraits () {
	cout << endl << ">>>" << "testIteratorTraits()" << endl;

	if (typeid(iterator_traits<Vector<T>::iterator>::value_type) != typeid(T)) {
		cout << "cannot determine iterator value type properly" << endl;
		cout << "iterator_traits::value_type = " << typeid(iterator_traits<Vector<T>::iterator>::value_type).name() << endl;
		cout << "T = " << typeid(T).name() << endl;
		failTest();
	}
	if (typeid(iterator_traits<Vector<T>::iterator>::iterator_category) != typeid(random_access_iterator_tag)) {
		cout << "iterator_traits::iterator_category = " << typeid(iterator_traits<Vector<T>::iterator>::iterator_category).name() << endl;
		failTest();
	}
	if (typeid(iterator_traits<Vector<T>::const_iterator>::value_type) != typeid(T)) {
		cout << "cannot determine const iterator value type properly" << endl;
		cout << "iterator_traits::value_type = " << typeid(iterator_traits<Vector<T>::const_iterator>::value_type).name() << endl;
		cout << "T = " << typeid(T).name() << endl;
		failTest();
	}
	if (typeid(iterator_traits<Vector<T>::const_iterator>::iterator_category) != typeid(random_access_iterator_tag)) {
		cout << "iterator_traits::iterator_category = " << typeid(iterator_traits<Vector<T>::const_iterator>::iterator_category).name() << endl;
		failTest();
	}
}

template <typename T>
void testInvalidOperations () {
	cout << endl << ">>>" << "testInvalidOperations()" << endl;

	Vector<T> v1;
	fillVector(v1, random(3, 20));
	Vector<T> v2;
	fillVector(v2, random(3, 20));

	testException<DifferentIteratorDomainException>([&](){ v1.begin() - v2.begin(); }, "v1.begin - v2.begin");
	testException<DifferentIteratorDomainException>([&](){ v1.begin() >= v2.begin(); }, "v1.begin >= v2.begin");

	testException<InvalidIteratorShiftException>([&](){ v1.begin() - 1000; }, "v1.begin + 1000");
	testException<InvalidIteratorShiftException>([&](){ v1.begin() - 1; }, "v1.begin - 1");

	testException<ExceptionEmulator>([&](){ v1.end() - 1; throw ExceptionEmulator(); }, "v1.end() - 1; throw 0");
	testException<ExceptionEmulator>([&](){ *(v1.begin() + 1); throw ExceptionEmulator(); }, "*(v1.begin + 1); throw 0");
	testException<ExceptionEmulator>([&](){ Iterator<T> it = v1.begin(); if (*it == *v1.begin()) throw ExceptionEmulator(); else throw "!"; }, "it = v1.begin; *it == *v1.begin?");

	testException<IteratorOutOfRangeException>([&](){ *v1.end();}, "*v1.end");

	Vector<T>::iterator it = v1.begin();
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
	testGetAndSet<T>();
	testCopyConstructor<T>();
	testMoveConstructor<T>();
	testIteratorConstructor<T>();
	testSetOperatorLValue<T>();
	testSetOperatorRValue<T>();
	testPopBack<T>();
	testPushBackLValue<T>();
	testPushBackRValue<T>();
	testReserveAndShrinkToFit<T>();
	testSwap<T>();
	testClear<T>();

	testIteratorValidity<T>();
	testRangedFor<T>();
	testIteratorCasts<T>();
	testIteratorTraits<T>();
	testInvalidOperations<T>();
}

template <>
void test<TypeUncopiable> () {
	typedef TypeUncopiable T;

	testGetAndSet<T>();
	testMoveConstructor<T>();
	testSetOperatorRValue<T>();
	testPopBack<T>();
	testPushBackRValue<T>();
	testSwap<T>();
	testClear<T>();

	testRangedFor<T>();
	testIteratorCasts<T>();
	testIteratorTraits<T>();
	testInvalidOperations<T>();
}

int main() {

	for (size_t t = 0; t < 50; ++t) {
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
