#include <assert.h>
#include <stdio.h>

// 선언은 호출 전에 함수의 매개변수와 반환 타입을 알려 줍니다.
static int clamp_score(int value);
static int sum_first(int count);
static int next_ticket(void);

static int clamp_score(int value)
{
    // value는 호출자가 넘긴 int 값의 지역 사본입니다.
    if (value < 0) {
        return 0;
    }
    if (value > 100) {
        return 100;
    }
    return value;
}

static int sum_first(int count)
{
    // 예제 계약: 0~100만 허용하며, 잘못된 입력은 -1을 반환합니다.
    if (count < 0 || count > 100) {
        return -1;
    }
    int sum = 0;
    for (int value = 1; value <= count; ++value) {
        sum += value;
    }
    return sum; // 허용된 범위의 최대 합은 5050입니다.
}

static int next_ticket(void)
{
    // 정적 저장 기간 객체는 호출이 끝나도 값이 유지됩니다.
    static int issued = 0;
    ++issued;
    return issued;
}

int main(void)
{
    int original = 120;
    int clamped = clamp_score(original);
    assert(original == 120); // 값 전달이므로 원본 변수는 바뀌지 않습니다.
    assert(clamped == 100);
    assert(clamp_score(-3) == 0 && clamp_score(55) == 55);
    assert(sum_first(0) == 0 && sum_first(5) == 15);
    assert(sum_first(100) == 5050);
    assert(sum_first(-1) == -1 && sum_first(101) == -1);

    int outer = 5;
    {
        int inside = outer + 1; // inside는 이 블록 안에서만 이름이 보입니다.
        assert(inside == 6);
    }
    assert(outer == 5);

    int first = next_ticket();
    int second = next_ticket();
    assert(first == 1 && second == 2);

    printf("original=%d clamped=%d sum5=%d tickets=%d,%d\n",
           original, clamped, sum_first(5), first, second);
    return 0;
}
