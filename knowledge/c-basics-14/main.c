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

enum { ITEM_COUNT = 3, STACK_LIMIT = 99 };
typedef struct { unsigned int quantity[ITEM_COUNT]; } Inventory;
typedef enum { COMMAND_ADD, COMMAND_USE } Operation;
typedef struct { Operation operation; unsigned int item; unsigned int amount; } Command;
typedef enum { RESULT_OK, RESULT_SYNTAX, RESULT_RANGE, RESULT_STOCK } Result;

/* 문법을 좁힌다: 십진 숫자 1개 이상. 공백과 부호는 허용하지 않는다. */
static int parse_number(const char **cursor, unsigned int *out)
{
    const char *p = *cursor;
    unsigned int value = 0;
    if (*p < '0' || *p > '9') return 0;
    while (*p >= '0' && *p <= '9') {
        unsigned int digit = (unsigned int)(*p - '0');
        if (value > (UINT_MAX - digit) / 10u) return 0;
        value = value * 10u + digit;
        ++p;
    }
    *cursor = p;
    *out = value;
    return 1;
}

/* 입력은 NUL 종료 문자열. 실패 시 *out은 그대로다. */
static Result parse_command(const char *text, Command *out)
{
    Command candidate;
    const char *p;
    if (strncmp(text, "add ", 4) == 0) candidate.operation = COMMAND_ADD;
    else if (strncmp(text, "use ", 4) == 0) candidate.operation = COMMAND_USE;
    else return RESULT_SYNTAX;
    p = text + 4;
    if (!parse_number(&p, &candidate.item) || *p != ' ') return RESULT_SYNTAX;
    ++p;
    if (!parse_number(&p, &candidate.amount) || *p != '\0') return RESULT_SYNTAX;
    if (candidate.item >= ITEM_COUNT || candidate.amount == 0 ||
        candidate.amount > STACK_LIMIT) return RESULT_RANGE;
    *out = candidate;
    return RESULT_OK;
}

static Result apply_command(Inventory *bag, const Command *command)
{
    unsigned int *quantity;
    size_t i;
    for (i = 0; i < ITEM_COUNT; ++i)
        if (bag->quantity[i] > STACK_LIMIT) return RESULT_RANGE;
    if (command->item >= ITEM_COUNT || command->amount == 0 ||
        command->amount > STACK_LIMIT) return RESULT_RANGE;
    quantity = &bag->quantity[command->item];
    if (command->operation == COMMAND_ADD) {
        if (command->amount > STACK_LIMIT - *quantity) return RESULT_RANGE;
        *quantity += command->amount;
    } else if (command->operation == COMMAND_USE) {
        if (command->amount > *quantity) return RESULT_STOCK;
        *quantity -= command->amount;
    } else return RESULT_RANGE;
    return RESULT_OK;
}

/* 배치 전체가 성공해야 원본에 반영된다. 실패 위치는 0부터 센다. */
static Result run_batch(Inventory *bag, const char *const lines[],
                        size_t count, size_t *failed_at)
{
    Inventory candidate = *bag;
    size_t i;
    for (i = 0; i < count; ++i) {
        Command command;
        Result result = parse_command(lines[i], &command);
        if (result == RESULT_OK) result = apply_command(&candidate, &command);
        if (result != RESULT_OK) { *failed_at = i; return result; }
    }
    *bag = candidate;
    *failed_at = count;
    return RESULT_OK;
}

static int same_inventory(const Inventory *a, const Inventory *b)
{
    size_t i;
    for (i = 0; i < ITEM_COUNT; ++i)
        if (a->quantity[i] != b->quantity[i]) return 0;
    return 1; /* 패딩까지 비교하는 memcmp 대신 의미 있는 필드를 비교한다. */
}

int main(void)
{
    Inventory bag = {{0u, 0u, 0u}}, before;
    Command command = {COMMAND_USE, 2u, 1u};
    size_t failed_at = 0;
    const char *good[] = {"add 0 8", "use 0 3", "add 2 99"};
    const char *bad[] = {"add 1 4", "use 0 6"};
    const char *bad_text[] = {"add 0 1", "add 3 1"};
    const char *invalid[] = {"", "add", "add -1 2", "add 0 +2", "add  0 2",
        "add 0 2 ", "add 0 2x", "use 0", "ADD 0 1"};
    char overflow[sizeof(unsigned int) * CHAR_BIT + 16];
    int written;
    size_t i;
    written = snprintf(overflow, sizeof overflow, "add 0 %u0", UINT_MAX);
    if (written < 0 || (size_t)written >= sizeof overflow) return 1;
    CHECK(parse_command(overflow, &command) == RESULT_SYNTAX);
    CHECK(run_batch(&bag, good, 3, &failed_at) == RESULT_OK && failed_at == 3);
    CHECK(bag.quantity[0] == 5u && bag.quantity[1] == 0u && bag.quantity[2] == 99u);
    before = bag;
    CHECK(run_batch(&bag, bad, 2, &failed_at) == RESULT_STOCK && failed_at == 1);
    CHECK(same_inventory(&bag, &before));
    CHECK(run_batch(&bag, bad_text, 2, &failed_at) == RESULT_RANGE && failed_at == 1);
    CHECK(same_inventory(&bag, &before));
    for (i = 0; i < sizeof invalid / sizeof invalid[0]; ++i) {
        CHECK(parse_command(invalid[i], &command) == RESULT_SYNTAX);
        CHECK(command.operation == COMMAND_USE && command.item == 2u && command.amount == 1u);
    }
    CHECK(parse_command("add 0 0", &command) == RESULT_RANGE);
    CHECK(parse_command("add 0 100", &command) == RESULT_RANGE);
    command = (Command){COMMAND_ADD, 2u, 1u};
    CHECK(apply_command(&bag, &command) == RESULT_RANGE && same_inventory(&bag, &before));
    command = (Command){(Operation)99, 0u, 1u};
    CHECK(apply_command(&bag, &command) == RESULT_RANGE && same_inventory(&bag, &before));
    CHECK(run_batch(&bag, NULL, 0, &failed_at) == RESULT_OK && failed_at == 0);
    puts("inventory: parse, overflow guards, atomic batches, rollback OK");
    return 0;
}
