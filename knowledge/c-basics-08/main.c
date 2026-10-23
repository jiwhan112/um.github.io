#include <assert.h>
#include <stddef.h>
#include <stdio.h>

// 동일한 함수 시그니처를 갖는 비교 함수의 타입을 별칭으로 정의합니다.
typedef int (*OrderFn)(int left, int right);

static int ascending(int left, int right)
{
    // left - right는 signed 오버플로 위험이 있으므로 비교식으로 반환합니다.
    return (left > right) - (left < right);
}

static int descending(int left, int right)
{
    return ascending(right, left);
}

// data는 count개의 유효한 int 원소를 가리킨다는 호출자 계약이 필요합니다.
static int sort_small(int data[], size_t count, OrderFn order)
{
    if (order == NULL || count > 8 || (count != 0 && data == NULL)) {
        return 0;
    }
    for (size_t i = 0; i < count; ++i) {
        size_t best = i;
        for (size_t j = i + 1; j < count; ++j) {
            if (order(data[j], data[best]) < 0) {
                best = j;
            }
        }
        if (best != i) {
            const int saved = data[i];
            data[i] = data[best];
            data[best] = saved;
        }
    }
    return 1;
}

int main(void)
{
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int (*row)[3] = matrix; // 포인터의 한 번 이동 단위가 int 3개짜리 한 행입니다.
    assert(row[0][2] == 3 && row[1][2] == 6);
    assert((row + 1) == &matrix[1]);

    int numbers[] = {5, -3, 9, 0, -1};
    const size_t count = sizeof numbers / sizeof numbers[0];
    if (!sort_small(numbers, count, ascending)) return 1;
    const int low_to_high[] = {-3, -1, 0, 5, 9};
    for (size_t i = 0; i < count; ++i) {
        assert(numbers[i] == low_to_high[i]);
    }

    // 함수 이름은 호출 문맥에서 호환되는 함수 포인터로 변환됩니다.
    OrderFn selected = descending;
    if (!sort_small(numbers, count, selected)) return 2;
    const int high_to_low[] = {9, 5, 0, -1, -3};
    for (size_t i = 0; i < count; ++i) {
        assert(numbers[i] == high_to_low[i]);
    }
    if (sort_small(numbers, count, NULL)) return 3;
    if (sort_small(NULL, 1, ascending)) return 4;

    printf("matrix=%d ascending_first=%d descending=%d,%d,%d,%d,%d\n",
           row[1][2], low_to_high[0], numbers[0], numbers[1], numbers[2],
           numbers[3], numbers[4]);
    return 0;
}
