#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// 호출자가 전달한 배열의 실제 원소 수는 count로 따로 받습니다.
// 예제 계약: 최대 4개, 각 점수는 0~100이므로 합계는 400을 넘지 않습니다.
static int sum_scores(const int scores[], size_t count, int *out_total)
{
    if (out_total == NULL || count > 4 || (count != 0 && scores == NULL)) {
        return 0;  // 잘못된 호출에서는 출력 객체를 변경하지 않습니다.
    }
    int total = 0;
    for (size_t i = 0; i < count; ++i) {
        if (scores[i] < 0 || scores[i] > 100) {
            return 0;  // 계산 범위와 의미를 함께 검증합니다.
        }
        total += scores[i];
    }
    *out_total = total;
    return 1;
}

int main(void)
{
    int scores[4] = {78, 93, 85, 100};
    const size_t count = sizeof scores / sizeof scores[0];
    int total = -1;
    if (!sum_scores(scores, count, &total)) return 1;
    assert(total == 356);
    if (sum_scores(scores, count + 1, &total)) return 2;
    assert(total == 356); // 실패해도 기존 결과는 유지됩니다.

    const int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int matrix_total = 0;
    for (size_t row = 0; row < 2; ++row) {
        for (size_t col = 0; col < 3; ++col) {
            matrix_total += matrix[row][col];
        }
    }
    assert(matrix_total == 21);

    char tag[] = "wolf"; // 다섯 번째 원소는 문자열 끝을 알리는 '\0'입니다.
    const size_t bytes = sizeof tag;
    const size_t length = strlen(tag);
    assert(bytes == 5 && length == 4);

    char formatted[16];
    const int written = snprintf(formatted, sizeof formatted, "%s-%d", tag, total);
    // snprintf의 반환값은 널 종료를 제외한 필요한 문자 수입니다.
    assert(written >= 0 && (size_t)written < sizeof formatted);
    assert(strcmp(formatted, "wolf-356") == 0);

    char shifted[] = "ABCDE";
    // 영역이 겹치는 이동에는 memcpy 대신 memmove를 사용합니다.
    memmove(shifted + 1, shifted, 4);
    assert(strcmp(shifted, "AABCD") == 0);

    printf("count=%zu total=%d matrix=%d tag=%s bytes=%zu length=%zu moved=%s formatted=%s\n",
           count, total, matrix_total, tag, bytes, length, shifted, formatted);
    return 0;
}
