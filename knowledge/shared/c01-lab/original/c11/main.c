// Strictly demonstrate checking scanf and widening before signed addition.
// C11-compatible, also valid C++11 and C++20.
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    int a = 0, b = 0;
    if (scanf("%d %d", &a, &b) != 2) {
        fputs("Expected two integers.\n", stderr);
        return 1;
    }
    printf("You entered: %d and %d\n", a, b);
    printf("Sum: %" PRId64 "\n", (int64_t)a + (int64_t)b);
    return 0;
}
