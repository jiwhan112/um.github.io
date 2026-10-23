#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct { int *data; size_t count; size_t capacity; } Inventory;
/* 결정적 실패 시험용입니다. 실제 메모리 고갈을 유발하지 않습니다. */
static int fail_next = 0;
static void *try_resize(void *ptr, size_t bytes)
{
    if (fail_next) { fail_next = 0; return NULL; }
    return realloc(ptr, bytes);
}

static int reserve(Inventory *bag, size_t wanted)
{
    if (bag == NULL) return 0;
    if (wanted <= bag->capacity) return 1;
    if (wanted > SIZE_MAX / sizeof *bag->data) return 0;
    int *next = try_resize(bag->data, wanted * sizeof *bag->data);
    if (next == NULL) return 0; /* 기존 소유 포인터와 내용은 유지됩니다. */
    bag->data = next; /* 성공 뒤 옛 포인터/별칭은 사용하지 않습니다. */
    bag->capacity = wanted;
    return 1;
}

static int push(Inventory *bag, int item)
{
    if (bag == NULL || bag->count == SIZE_MAX) return 0;
    if (bag->count == bag->capacity) {
        size_t wanted = bag->capacity == 0 ? 2 : bag->capacity;
        if (bag->capacity != 0) {
            if (wanted > SIZE_MAX / 2) return 0;
            wanted *= 2;
        }
        if (!reserve(bag, wanted)) return 0;
    }
    bag->data[bag->count] = item;
    ++bag->count;
    return 1;
}

static void destroy(Inventory *bag)
{
    if (bag == NULL) return;
    free(bag->data);
    *bag = (Inventory){ 0 }; /* 이 소유자만 비웁니다. 다른 별칭은 복구하지 못합니다. */
}

int main(void)
{
    Inventory bag = { 0 };
    int ok = push(&bag, 10);
    if (!ok) { fputs("allocation failed\n", stderr); destroy(&bag); return 1; }
    ok = push(&bag, 20);
    if (!ok) { fputs("allocation failed\n", stderr); destroy(&bag); return 1; }
    int *before = bag.data;
    fail_next = 1;
    ok = push(&bag, 30);
    assert(!ok && bag.data == before && bag.count == 2 && bag.capacity == 2);
    assert(bag.data[0] == 10 && bag.data[1] == 20);
    puts("injected failure: count=2 capacity=2 values=10,20");
    /* SIZE_MAX 자체는 항상 표현됩니다. 곱셈은 검증 전에 하지 않습니다. */
    if (sizeof *bag.data > 1) {
        ok = reserve(&bag, SIZE_MAX / sizeof *bag.data + 1);
        assert(!ok && bag.data == before && bag.count == 2);
    }
    /* 이후 성공한 realloc은 before를 무효화할 수 있으므로 다시 읽지 않습니다. */
    ok = push(&bag, 30);
    if (!ok) { fputs("allocation failed\n", stderr); destroy(&bag); return 1; }
    assert(bag.count == 3 && bag.capacity == 4 && bag.data[2] == 30);
    printf("retry: count=%zu capacity=%zu values=%d,%d,%d\n",
           bag.count, bag.capacity, bag.data[0], bag.data[1], bag.data[2]);
    destroy(&bag);
    destroy(&bag); /* 초기 상태로 되돌렸기 때문에 반복 정리가 가능합니다. */
    assert(bag.data == NULL && bag.count == 0 && bag.capacity == 0);
    puts("destroy: empty owner");
    return 0;
}
