#include <iostream>
#include <windows.h>
#include <emmintrin.h>
#include <intrin.h>
#define PAGE_SIZE 4096

typedef unsigned char byte;

byte array[256 * PAGE_SIZE];
uint8_t target_data = 0x37;

int main(){    
    uint8_t junk;
    uint8_t* p = &target_data;
    uintptr_t ptr123;
    uint64_t time[256];

    // Clear cache
    for (int i = 0; i < 256; i++){
        _mm_clflush(&array[i * PAGE_SIZE]);
    }

    // Using Exception Handler
    __try {
        *(int*)0 = 0; // Access Violation
        // Instruction below this line is executed out-of-order
        junk = array[*p * PAGE_SIZE];
    }
    __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
		// An access violation is expected.
	}

    // Now the address of &array[target_data] is cached
    uint64_t minTime = 0xFFFF;
    int leak;
    for (int i = 0; i < 256; i++) {
        uint32_t aux;
		uint64_t t1 = __rdtscp(&aux);
        junk = array[i * PAGE_SIZE];
        //rand(); // A function to ensure that CPU won't run __rdtsc below out-of-order.
		uint64_t t2 = __rdtscp(&aux) - t1;
		if (t2 < minTime) {
			minTime = t2;
			leak = i;
		}
        time[i] = t2;
	}

    std::cout << std::endl;
    std::cout << "The leaked target_data is " << leak << std::endl;

    return 0;
}