#include "inventory.h"
#include "inventory.h" /* 가드 덕분에 같은 번역 단위의 중복 포함은 안전하다. */
#include <stdlib.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* NDEBUG 설정과 무관하게 식을 한 번 평가하는 학습용 검사다. */
static void check(int passed, int line)
{
    if (!passed) { fprintf(stderr, "check failed at line %d\n", line); abort(); }
}
#define CHECK(expression) check((expression), __LINE__)

int main(void)
{
    Inventory bag = {0u};
    CHECK(inventory_add(&bag, 0u) == INV_OK);
    CHECK(inventory_add(&bag, 40u) == INV_OK);
    CHECK(inventory_add(&bag, 59u) == INV_OK);
    CHECK(bag.quantity == INVENTORY_LIMIT);
    CHECK(inventory_add(&bag, 1u) == INV_RANGE && bag.quantity == 99u);
    CHECK(inventory_add(&bag, UINT_MAX) == INV_RANGE && bag.quantity == 99u);
    CHECK(inventory_add(NULL, 1u) == INV_ARGUMENT);
    bag.quantity = 100u; /* 깨진 외부 상태도 거부한다. */
    CHECK(inventory_add(&bag, 0u) == INV_RANGE && bag.quantity == 100u);
    CHECK(strcmp(inventory_result_name(INV_OK), "ok") == 0);
    CHECK(strcmp(inventory_result_name(INV_RANGE), "range") == 0);
    CHECK(strcmp(inventory_result_name(INV_ARGUMENT), "argument") == 0);
    CHECK(strcmp(inventory_result_name((InventoryResult)99), "unknown") == 0);
    puts("build: declarations, linkage, inventory contracts OK");
    return 0;
}
