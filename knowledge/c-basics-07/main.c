#include <assert.h>
#include <stddef.h>
#include <stdio.h>

// 성공 시에만 호출자의 객체를 변경하는 출력 매개변수 예제입니다.
static int set_level(int *target, int next)
{
    if (target == NULL || next < 0 || next > 100) {
        return 0;
    }
    *target = next;
    return 1;
}

// 읽기 포인터와 쓰기 포인터를 서로 다른 타입 계약으로 표현합니다.
static int half_if_even(const int *input, int *out_half)
{
    if (input == NULL || out_half == NULL || *input % 2 != 0) {
        return 0;
    }
    const int result = *input / 2;
    *out_half = result;
    return 1;
}

int main(void)
{
    int level = 10;
    int *mutable_pointer = &level; // 변수의 주소를 얻어 같은 객체를 가리킵니다.
    if (!set_level(mutable_pointer, 21)) return 1;
    assert(level == 21);
    if (set_level(mutable_pointer, 101)) return 2;
    assert(level == 21);           // 실패는 기존 값을 덮어쓰지 않습니다.
    if (set_level(NULL, 42)) return 3; // 널 포인터는 역참조하지 않습니다.

    int samples[3] = {4, 8, 12};
    const int *read_only = samples;  // 이 포인터를 통해 쓰지 못합니다.
    int *const fixed_address = samples; // 포인터 변수 자체를 재지정하지 못합니다.
    assert(*read_only == 4 && *fixed_address == 4);

    // end는 배열 끝 바로 다음을 가리키지만 절대 *end를 실행하지 않습니다.
    const int *begin = samples;
    const int *end = samples + 3;
    const ptrdiff_t count = end - begin;
    int total = 0;
    for (const int *it = begin; it != end; ++it) {
        total += *it;
    }
    assert(count == 3 && total == 24);
    assert(*(end - 1) == 12); // 마지막 실제 원소만 역참조합니다.

    int half = -1;
    if (!half_if_even(&total, &half)) return 4;
    assert(half == 12);
    if (half_if_even(&level, &half)) return 5; // 21은 홀수이므로 거부합니다.
    assert(half == 12);

    printf("level=%d count=%td total=%d half=%d last=%d\n",
           level, count, total, half, *(end - 1));
    return 0;
}
