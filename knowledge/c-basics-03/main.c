#include <assert.h>
#include <limits.h>
#include <stdio.h>

int main(void)
{
    int whole = 5 / 2;                    // 정수끼리 나눠서 2입니다.
    double late = (double)(5 / 2);        // 이미 버린 소수는 돌아오지 않습니다.
    double early = (double)5 / 2;         // 나누기 전에 double로 바꿉니다.
    unsigned int flags = 0u;
    const unsigned int mask = 1u << 2;    // 오른쪽에서 세 번째 비트: 4
    int left = INT_MAX;
    int right = 1;
    int rejected = 0;

    assert(whole == 2 && late == 2.0 && early == 2.5);
    assert(-5 / 2 == -2 && -5 % 2 == -1);
    assert((unsigned int)-1 == UINT_MAX);
    assert(UINT_MAX + 1u == 0u);          // unsigned 덧셈의 모듈러 규칙
    flags |= mask;                       // 켜기
    assert(flags == 4u);
    flags |= mask;                       // 다시 켜도 켜진 상태
    assert(flags == 4u);
    flags &= ~mask;                      // 끄기
    assert(flags == 0u);
    flags ^= mask;                       // 뒤집기
    assert(flags == 4u);
    assert((flags && 2u) == 1);          // 논리 AND: 두 값이 모두 0이 아님
    assert((flags & 2u) == 0u);          // 비트 AND: 겹치는 1비트가 없음

    // if와 ||는 04에서 자세히 배웁니다. 범위를 먼저 검사하는 예고입니다.
    // right의 부호별로 안전한 뺄셈만 평가합니다(단락 평가).
    if ((right > 0 && left > INT_MAX - right) ||
        (right < 0 && left < INT_MIN - right)) {
        rejected = 1;                    // 위험한 left + right를 실행하지 않습니다.
    } else {
        left += right;
    }
    assert(rejected == 1 && left == INT_MAX);
    printf("division=%d late=%.1f early=%.1f\n", whole, late, early);
    printf("flags=%u overflow_rejected=%d\n", flags, rejected);
    return 0;
}
