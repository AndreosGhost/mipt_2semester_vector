#pragma once

#include <cstddef>
#include <exception>

#if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 180000000
#define noexcept throw()
#endif

struct MemoryLeakException : public std::exception {
	const char * message;

	explicit MemoryLeakException (const char * const &msg) : message (msg) { }

	const char * what () const noexcept override {
		return message;
	}
};

//This class is tracking memory leaks
class MemoryWatcher {
private:
	size_t vectorsDefCreated;  //created with default constructor
	size_t vectorsCopyCreated; //created with 'copy' constructor
	size_t vectorsMoveCreated; //created with 'move' constructor
	size_t vectorsIterCreated; //created with 'iterator' constructor
	size_t vectorsDestroyed;   //destroyed

	ptrdiff_t memoryAllocated;	//allocated memory. Measured with std::distance
	ptrdiff_t memoryDeallocated; //deallocated memory. Measured the same way.

	size_t baseIteratorsPtrCreated; //created with private 'pointer' constructor
	size_t baseIteratorsDefCreated;
	size_t baseIteratorsCopyCreated;
	size_t baseIteratorsMoveCreated;
	size_t baseIteratorsDestroyed;

	size_t containersHostCreated;	//host vector is given as a parameter to this constructor
	size_t containersDestroyed;

public:
	size_t getVectorsDefCreated () const noexcept { return vectorsDefCreated; }
	size_t getVectorsCopyCreated () const noexcept { return vectorsCopyCreated; }
	size_t getVectorsMoveCreated () const noexcept { return vectorsMoveCreated; }
	size_t getVectorsIterCreated () const noexcept { return vectorsIterCreated; }
	size_t getVectorsDestroyed () const noexcept { return vectorsDestroyed; }

	size_t getMemoryAllocated () const noexcept { return memoryAllocated; }
	size_t getMemoryDeallocated () const noexcept { return memoryDeallocated; }

	size_t getBaseIteratorsDefCreated () const noexcept { return baseIteratorsDefCreated; }
	size_t getBaseIteratorsCopyCreated () const noexcept { return baseIteratorsCopyCreated; }
	size_t getBaseIteratorsMoveCreated () const noexcept { return baseIteratorsMoveCreated; }
	size_t getBaseIteratorsPtrCreated () const noexcept { return baseIteratorsPtrCreated; }
	size_t getBaseIteratorsDestroyed () const noexcept { return baseIteratorsDestroyed; }

	size_t getContainersHostCreated () const noexcept { return containersHostCreated; }
	size_t getContainersDestroyed () const noexcept { return containersDestroyed; }

	int vectorsAlive () const noexcept { return (int)vectorsDefCreated + vectorsCopyCreated + vectorsMoveCreated + vectorsIterCreated - vectorsDestroyed; }
	int memoryAlive () const noexcept { return (int)memoryAllocated - memoryDeallocated; }
	int iteratorsAlive () const noexcept { return (int)baseIteratorsDefCreated + baseIteratorsCopyCreated + baseIteratorsMoveCreated + baseIteratorsPtrCreated - baseIteratorsDestroyed; }
	int containersAlive () const noexcept { return (int)containersHostCreated - containersDestroyed; }

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