#include <assert.h>
#include <limits.h>
#include <stdio.h>

// 정적 저장 기간의 객체는 초기값을 생략하면 0으로 초기화됩니다.
static int total;

int main(void)
{
    int score = 80;               // 선언과 함께 초기값을 정합니다.
    const int bonus = 5;          // 이 이름을 통해 값을 바꾸지 않습니다.
    double average = 5.0 / 2.0;   // 소수 부분을 갖는 계산입니다.
    unsigned char byte = 0;       // sizeof(byte)는 C의 1바이트입니다.
    score = score + bonus;        // 이미 존재하는 객체에 대입합니다.

    assert(score == 85);
    assert(total == 0);
    assert(average == 2.5);
    assert(sizeof byte == 1);
    assert(CHAR_BIT >= 8);
    assert(INT_MAX >= 32767);
    printf("score=%d total=%d average=%.1f\n", score, total, average);
    // sizeof의 결과는 size_t이므로 %zu로 출력합니다.
    printf("sizeof(int)=%zu CHAR_BIT=%d\n", sizeof(int), CHAR_BIT);
    printf("int=[%d, %d] UCHAR_MAX=%u\n", INT_MIN, INT_MAX,
           (unsigned int)UCHAR_MAX);
    return 0;
}
