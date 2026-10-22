#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const int score = 72;
    const char *grade = "invalid";

    // 유효 범위를 먼저 확인한 다음 점수 구간을 분류합니다.
    if (score >= 0 && score <= 100) {
        if (score >= 90) {
            grade = "excellent";
        } else if (score >= 60) {
            grade = "pass";
        } else {
            grade = "retry";
        }
    }
    assert(strcmp(grade, "pass") == 0);

    int total = 0;
    // 종료 조건과 증감식을 명시하고, 1부터 5까지만 더합니다.
    for (int value = 1; value <= 5; ++value) {
        total += value;
    }
    assert(total == 15);

    int visits = 0;
    // 조건이 거짓이 되는 시점에 반복을 끝냅니다.
    while (visits < 3) {
        ++visits;
    }
    assert(visits == 3);

    int attempts = 0;
    // do-while은 조건을 확인하기 전에 본문을 한 번 실행합니다.
    do {
        ++attempts;
    } while (attempts < 1);
    assert(attempts == 1);

    int selected = -1;
    for (int candidate = 1; candidate <= 10; ++candidate) {
        if (candidate % 2 != 0) {
            continue; // 홀수는 처리하지 않고 다음 후보로 넘어갑니다.
        }
        selected = candidate;
        if (candidate == 6) {
            break; // 가장 가까운 반복문만 종료합니다.
        }
    }
    assert(selected == 6);

    const char *status = "unknown";
    // case 사이에 자동 break는 없으므로 각 경로를 명시합니다.
    switch (selected) {
        case 6:
            status = "found";
            break;
        default:
            status = "missing";
            break;
    }
    assert(strcmp(status, "found") == 0);

    printf("grade=%s total=%d visits=%d attempts=%d selected=%d status=%s\n",
           grade, total, visits, attempts, selected, status);
    return 0;
}
