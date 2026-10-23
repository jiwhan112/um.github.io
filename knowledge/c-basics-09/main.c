#include <assert.h>
#include <stddef.h>
#include <stdio.h>

typedef struct { int hp; int x; int y; } Actor;
typedef enum { EVENT_DAMAGE, EVENT_MOVE } EventKind;
typedef struct {
    EventKind kind; /* 태그와 활성 멤버는 항상 함께 설정합니다. */
    union { int damage; struct { int x; int y; } move; } data;
} Event;

/* 실패하면 원래 상태를 유지합니다. 모든 멤버가 값인 구조체입니다. */
static int apply_event(Actor *actor, const Event *event)
{
    if (actor == NULL || event == NULL) return 0;
    Actor next = *actor;
    if (next.hp < 0 || next.hp > 100) return 0;
    switch (event->kind) {
    case EVENT_DAMAGE:
        if (event->data.damage < 0 || event->data.damage > 100) return 0;
        next.hp = event->data.damage >= next.hp ? 0 : next.hp - event->data.damage;
        break;
    case EVENT_MOVE:
        if (event->data.move.x < 0 || event->data.move.x > 9 ||
            event->data.move.y < 0 || event->data.move.y > 9) return 0;
        next.x = event->data.move.x;
        next.y = event->data.move.y;
        break;
    default: /* enum 선언만으로 외부 입력을 검증할 수는 없습니다. */
        return 0;
    }
    *actor = next; /* 검증 완료 뒤 한 번만 상태를 반영합니다. */
    return 1;
}

static int same_actor(Actor a, Actor b)
{
    /* 패딩 바이트가 아니라 의미 있는 멤버를 비교합니다. */
    return a.hp == b.hp && a.x == b.x && a.y == b.y;
}

int main(void)
{
    Actor hero = { .hp = 80, .x = 0, .y = 0 };
    Event hit = { .kind = EVENT_DAMAGE, .data.damage = 25 };
    Event move = { .kind = EVENT_MOVE, .data.move = { .x = 3, .y = 4 } };
    int ok = apply_event(&hero, &hit);
    assert(ok && hero.hp == 55);
    ok = apply_event(&hero, &move);
    assert(ok && hero.x == 3 && hero.y == 4);
    printf("hero: hp=%d position=(%d,%d)\n", hero.hp, hero.x, hero.y);

    Actor saved = hero;
    Event bad = { .kind = EVENT_MOVE, .data.move = { .x = 10, .y = 4 } };
    ok = apply_event(&hero, &bad);
    assert(!ok && same_actor(hero, saved));
    bad = (Event){ .kind = EVENT_DAMAGE, .data.damage = -1 };
    ok = apply_event(&hero, &bad);
    assert(!ok && same_actor(hero, saved));
    bad.kind = (EventKind)99;
    ok = apply_event(&hero, &bad);
    assert(!ok && same_actor(hero, saved));
    ok = apply_event(NULL, &hit);
    assert(!ok);
    hit.data.damage = 100;
    ok = apply_event(&hero, &hit);
    assert(ok && hero.hp == 0);
    /* 실제 오프셋을 관찰하되 특정 ABI의 바이트 수를 정답으로 삼지 않습니다. */
    assert(offsetof(Actor, hp) == 0);
    assert(offsetof(Actor, x) < offsetof(Actor, y));
    puts("invalid events: unchanged; lethal damage: hp=0");
    puts("member equality and layout invariants: ok");
    return 0;
}
