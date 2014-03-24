#include "Vector.h"
#include <string>
#include <iostream>
#include <vector>

using namespace std;

class TypeWithoutDefCtor {
private:
	int value;
	TypeWithoutDefCtor () { cout << "Do not call this constructor"; }
public:
	TypeWithoutDefCtor (int value) {
		this->value = value;
	}
	bool operator== (const TypeWithoutDefCtor &obj) const {
		return value == obj.value;
	}
	
	int getValue () const {
		return value;
	}
};

#pragma region utility

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

ostream &operator << (ostream &s, const TypeWithoutDefCtor &obj) {
	s << obj.getValue();
	return s;
}

template <typename T>
ostream &operator<< (ostream &s, const vector<T> &vector) {
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
ostream &operator<< (ostream &s, const Vector<T> &vector) {
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

void fillVector (vector<int> &v, size_t length) {
	v.clear();

	for (size_t i = 0; i < length; ++i) {
		v.push_back(random(1, 100000));
	}
}

void fillVector (Vector<int> &v, size_t length) {
	v.clear();

	for (size_t i = 0; i < length; ++i) {
		v.push_back(random(1, 100000));
	}
}

void fillVector (Vector<int> &myVector, const vector<int> &sysVector) {
	myVector.clear();

	for (size_t i = 0, sz = sysVector.size(); i < sz; ++i) {
		myVector.push_back(sysVector[i]);
	}
}

void fillVector (vector<TypeWithoutDefCtor> &v, size_t length) {
	v.clear();

	for (size_t i = 0; i < length; ++i) {
		v.push_back(TypeWithoutDefCtor(random(1, 100000)));
	}
}

void fillVector (Vector<TypeWithoutDefCtor> &v, size_t length) {
	v.clear();

	for (size_t i = 0; i < length; ++i) {
		v.push_back(TypeWithoutDefCtor(random(1, 100000)));
	}
}

void fillVector (Vector<TypeWithoutDefCtor> &myVector, const vector<TypeWithoutDefCtor> &sysVector) {
	myVector.clear();

	for (size_t i = 0, sz = sysVector.size(); i < sz; ++i) {
		myVector.push_back(sysVector[i]);
	}
}

void fillVector (vector<Vector<int>> &v, size_t length) {
	v.clear();

	for (size_t i = 0; i < length; ++i) {
		Vector<int> temp;
		fillVector(temp, random(static_cast<size_t>(1), length));
		v.push_back(std::move(temp));
	}
}

void fillVector (Vector<Vector<int>> &myVector, const vector<Vector<int>> &sysVector) {
	myVector.clear();

	for (size_t i = 0, sz = sysVector.size(); i < sz; ++i) {
		myVector.push_back(sysVector[i]);
	}
}

#pragma endregion

#pragma region test Vector

template <typename T>
void testCopyConstructor () {
	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(1, 20));
	fillVector(myVector, sysVector);

	Vector<T> v_copy(myVector);

	if (!areEqual(sysVector, v_copy)) {
		cout << "error: bad copy constructor" << endl;
		cout << "sys. vector: " << sysVector << endl;
		cout << "src. vector: " << myVector << endl;
		cout << "copy vector: " << v_copy << endl;
	}
}

template <typename T>
void testMoveConstructor () {
	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(1, 20));
	fillVector(myVector, sysVector);

	Vector<T> v_move(move(myVector));

	if (!areEqual(sysVector, v_move) || !myVector.empty()) {
		cout << "error: bad move constructor" << endl;
		cout << "sys. vector: " << sysVector << endl;
		cout << "src. vector: " << myVector << endl;
		cout << "move vector: " << v_move << endl;
	}
}

template <typename T>
void testSetOperatorLValue () {
	Vector<T> myVector1;
	Vector<T> myVector2;
	vector<T> sysVector1;
	vector<T> sysVector2;

	fillVector(sysVector1, random(1, 20));
	fillVector(myVector1, sysVector1);
	fillVector(sysVector2, random(1, 20));
	fillVector(myVector2, sysVector2);

	myVector1 = myVector2;
	if (!areEqual(sysVector2, myVector1) || !areEqual(sysVector2, myVector2)) {
		cout << "error: bad operator= lvalue" << endl;
		cout << "sys. src vector: " << sysVector2 << endl;
		cout << "sys. dst vector: " << sysVector1 << endl;
		cout << "my src. vector (must be empty): " << myVector2 << endl;
		cout << "my dst. vector (must be = sys src.): " << myVector1 << endl;
	}
}

template <typename T>
void testSetOperatorRValue () {
	Vector<T> myVector1;
	Vector<T> myVector2;
	vector<T> sysVector1;
	vector<T> sysVector2;

	fillVector(sysVector1, random(1, 20));
	fillVector(myVector1, sysVector1);
	fillVector(sysVector2, random(1, 20));
	fillVector(myVector2, sysVector2);

	myVector1 = std::move(myVector2);
	if (!areEqual(sysVector2, myVector1) || !myVector2.empty()) {
		cout << "error: bad operator= rvalue" << endl;
		cout << "sys. src vector: " << sysVector2 << endl;
		cout << "sys. dst vector: " << sysVector1 << endl;
		cout << "my src. vector (must be empty): " << myVector2 << endl;
		cout << "my dst. vector (must be = sys src.): " << myVector1 << endl;
	}
}

template <typename T>
void testGetAndSet () {
	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(1, 20));
	fillVector(myVector, sysVector);

	if (!areEqual(sysVector, myVector)) {
		cout << "error: bad get&set [] operators" << endl;
		cout << "sys. vector: " << sysVector << endl;
		cout << "src. vector: " << myVector << endl;
	}
}

template <typename T>
void testSwap () {
	Vector<T> myVector1;
	Vector<T> myVector2;
	vector<T> sysVector1;
	vector<T> sysVector2;

	fillVector(sysVector1, random(1, 20));
	fillVector(myVector1, sysVector1);
	fillVector(sysVector2, random(1, 20));
	fillVector(myVector2, sysVector2);

	myVector1.swap(myVector2);

	if (!areEqual (sysVector1, myVector2) || !areEqual (sysVector2, myVector1)) {
		cout << "error: bad swap()" << endl;
		cout << "sys. vector 1: " << sysVector1 << endl;
		cout << "sys. vector 2: " << sysVector2 << endl;
		cout << "swap. vector 1: " << myVector1 << endl;
		cout << "swap. vector 2: " << myVector2 << endl;
	}
}

template <typename T>
void testReserveAndShrinkToFit () {
	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(1, 20));
	fillVector(myVector, sysVector);

	myVector.reserve(random(myVector.size(), 10 * myVector.size()));

	if (!areEqual(sysVector, myVector)) {
		cout << "error: bad reserve()" << endl;
		cout << "sys. vector: " << sysVector << endl;
		cout << "src. vector: " << myVector << endl;
	}

	myVector.shrink_to_fit();

	if (!areEqual(sysVector, myVector)) {
		cout << "error: bad shrink_to_fit()" << endl;
		cout << "sys. vector: " << sysVector << endl;
		cout << "src. vector: " << myVector << endl;
	}
}

template <typename T>
void testPushBackLValue () {
	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(1, 20));

	for (size_t i = 0, sz = sysVector.size(); i < sz; ++i) {
		myVector.push_back(sysVector[i]);
	}

	if (!areEqual(sysVector, myVector)) {
		cout << "error: bad push_back() lvalue" << endl;
		cout << "sys. vector: " << sysVector << endl;
		cout << "src. vector: " << myVector << endl;
	}
}

template <typename T>
void testPushBackRValue () {
	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(1, 20));
	vector<T> sysCopy(sysVector);

	for (size_t i = 0, sz = sysVector.size(); i < sz; ++i) {
		myVector.push_back(move(sysCopy[i]));
	}

	if (!areEqual(sysVector, myVector)) {
		cout << "error: bad push_back() rvalue" << endl;
		cout << "src. vector: " << sysVector << endl;
		cout << "my vector: " << myVector << endl;
		cout << "sys vector: " << sysCopy << endl;
	}
}

template <typename T>
void testPopBack () {
	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(1, 20));
	fillVector(myVector, sysVector);
	
	vector<T> sysCopy(sysVector);
	while (!sysCopy.empty()) {
		sysCopy.pop_back();
		myVector.pop_back();

		if (!areEqual(sysCopy, myVector)) {
			cout << "error: bad pop_back()" << endl;
			cout << "src vector: " << sysVector << endl;
			cout << "my vector: " << myVector << endl;
			cout << "sys vector: " << sysCopy << endl;
		}
	}
}

template <typename T>
void testClear () {
	Vector<T> myVector;
	vector<T> sysVector;

	fillVector(sysVector, random(1, 20));
	fillVector(myVector, sysVector);

	myVector.clear();

	if (!myVector.empty()) {
		cout << "bad clear()" << endl;
		cout << "prev. data: " << sysVector;
		cout << "having: " << myVector;
	}
}

template <typename T>
void testVector () {
	//call all methods starting with 'test'
	
	testGetAndSet<T>();
	testCopyConstructor<T>();
	testMoveConstructor<T>();
	testSetOperatorLValue<T>();
	testSetOperatorRValue<T>();
	testPopBack<T>();
	testPushBackLValue<T>();
	testPushBackRValue<T>();
	testReserveAndShrinkToFit<T>();
	testSwap<T>();
	testClear<T>();
}

#pragma endregion

int main() {
	cout << "Testing Vector<int>" << endl;
	testVector<int>();

	cout << "Testing Vector<Vector<int>>" << endl;
	testVector<Vector<int>>();

	cout << "Testing Vector<TypeWithoutDefCtor>" << endl;
	testVector<TypeWithoutDefCtor>();

	cout << "Testing over" << endl;

	//add tests on objects with [no] operator= &/&&, ctor(&/&&), ctor()

	return 0;
}
