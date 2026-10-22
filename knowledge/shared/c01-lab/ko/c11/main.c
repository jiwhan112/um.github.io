// scanf의 대입 성공 개수를 확인하고, 덧셈 전에 각 피연산자를 확장합니다.
// C11 코드이며 이 실습에서는 C++11·C++20에서도 동일한 소스를 컴파일합니다.
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    int a = 0, b = 0;
    // 성공적으로 변환하여 대입한 항목이 정확히 두 개인지 확인합니다.
    if (scanf("%d %d", &a, &b) != 2) {
        fputs("Expected two integers.\n", stderr);
        return 1;
    }
    printf("You entered: %d and %d\n", a, b);
    // int 덧셈 전에 각각 int64_t로 바꾸어 32비트 합계 범위를 확보합니다.
    printf("Sum: %" PRId64 "\n", (int64_t)a + (int64_t)b);
    return 0;
}
