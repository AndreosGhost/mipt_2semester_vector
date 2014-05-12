#pragma once

#include <cstddef>
#include <exception>

#if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 180000000
#define noexcept throw()
#endif

struct MemoryLeakException : public std::exception {
	const char* message;

	explicit MemoryLeakException (const char* msg) : message (msg) { }

	const char* what () const noexcept override {
		return message;
	}
};

//This class is tracking memory leaks
class MemoryWatcher {
private:
	int vectorsDefCreated;  //created with default constructor
	int vectorsCopyCreated; //created with 'copy' constructor
	int vectorsMoveCreated; //created with 'move' constructor
	int vectorsIterCreated; //created with 'iterator' constructor
	int vectorsDestroyed;   //destroyed

	ptrdiff_t memoryAllocated;	//allocated memory. Measured with std::distance
	ptrdiff_t memoryDeallocated; //deallocated memory. Measured the same way.

	int baseIteratorsPtrCreated; //created with private 'pointer' constructor
	int baseIteratorsDefCreated;
	int baseIteratorsCopyCreated;
	int baseIteratorsMoveCreated;
	int baseIteratorsDestroyed;

	int containersHostCreated;	//host vector is given as a parameter to this constructor
	int containersDestroyed;

public:
	int getVectorsDefCreated () const noexcept { return vectorsDefCreated; }
	int getVectorsCopyCreated () const noexcept { return vectorsCopyCreated; }
	int getVectorsMoveCreated () const noexcept { return vectorsMoveCreated; }
	int getVectorsIterCreated () const noexcept { return vectorsIterCreated; }
	int getVectorsDestroyed () const noexcept { return vectorsDestroyed; }

	int getMemoryAllocated () const noexcept { return memoryAllocated; }
	int getMemoryDeallocated () const noexcept { return memoryDeallocated; }

	int getBaseIteratorsDefCreated () const noexcept { return baseIteratorsDefCreated; }
	int getBaseIteratorsCopyCreated () const noexcept { return baseIteratorsCopyCreated; }
	int getBaseIteratorsMoveCreated () const noexcept { return baseIteratorsMoveCreated; }
	int getBaseIteratorsPtrCreated () const noexcept { return baseIteratorsPtrCreated; }
	int getBaseIteratorsDestroyed () const noexcept { return baseIteratorsDestroyed; }

	int getContainersHostCreated () const noexcept { return containersHostCreated; }
	int getContainersDestroyed () const noexcept { return containersDestroyed; }

	int vectorsAlive () const noexcept { return vectorsDefCreated + vectorsCopyCreated + vectorsMoveCreated + vectorsIterCreated - vectorsDestroyed; }
	int memoryAlive () const noexcept { return memoryAllocated - memoryDeallocated; }
	int iteratorsAlive () const noexcept { return baseIteratorsDefCreated + baseIteratorsCopyCreated + baseIteratorsMoveCreated + baseIteratorsPtrCreated - baseIteratorsDestroyed; }
	int containersAlive () const noexcept { return containersHostCreated - containersDestroyed; }

	void checkVectorCreationConsistency () const {
		if (vectorsAlive() != 0) {
			throw MemoryLeakException ("Memory leak! Some vectors may be still alive!");
		}
	}

	void checkMemoryConsistency () const {
		if (memoryAlive() != 0) {
			throw MemoryLeakException ("Memory leak! Memory inside vectors is not consistent!");
		}
	}

	void checkIteratorConsistency () const {
		if (iteratorsAlive() != 0) {
			throw MemoryLeakException ("Memory leak! Some iterators may be still alive!");
		}
	}

	void checkContainerConsistency () const {
		if (containersAlive() != 0) {
			throw MemoryLeakException ("Memory leak! Some containers may be still alive!");
		}
	}

	void checkTotalConsistency () const {
		checkVectorCreationConsistency ();
		checkMemoryConsistency ();
		checkIteratorConsistency ();
		checkContainerConsistency ();
	}

	void onVectorDefCreated () noexcept { ++vectorsDefCreated; }
	void onVectorCopyCreated () noexcept { ++vectorsCopyCreated; }
	void onVectorMoveCreated () noexcept { ++vectorsMoveCreated; }
	void onVectorIterCreated () noexcept { ++vectorsIterCreated; }
	void onVectorDestroyed () noexcept { ++vectorsDestroyed; }

	void onMemoryAllocated (ptrdiff_t memory) noexcept { memoryAllocated += memory; }
	void onMemoryDeallocated (ptrdiff_t memory) noexcept { memoryDeallocated += memory; }

	void onBaseIteratorDefCreated () noexcept { ++baseIteratorsDefCreated; }
	void onBaseIteratorCopyCreated () noexcept { ++baseIteratorsCopyCreated; }
	void onBaseIteratorMoveCreated () noexcept { ++baseIteratorsMoveCreated; }
	void onBaseIteratorPtrCreated () noexcept { ++baseIteratorsPtrCreated; }
	void onBaseIteratorDestroyed () noexcept { ++baseIteratorsDestroyed; }

	void onContainerHostCreated () noexcept { ++containersHostCreated; }
	void onContainerDestroyed () noexcept { ++containersDestroyed; }
};

//use this object to trace memory flows
static MemoryWatcher watcher;