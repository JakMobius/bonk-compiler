
#include <stdio.h>
#include <time.h>

long long print_number(long long number) {
    printf("%lld\n", number);
    return 1;
}

long long put_char(long long c) {
    printf("%c", (char)c);
    return 1;
}

long long get_clock() {
	return clock();
}

long long c_fibonacci(long long a) {
	if(a < 2) return 1;
	return c_fibonacci(a - 1) + c_fibonacci(a - 2);
}

long long print_clock_seconds(long long clocks) {
    printf("%g seconds\n", (double)clocks / (double)CLOCKS_PER_SEC);
}