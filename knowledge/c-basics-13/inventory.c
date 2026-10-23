#include "inventory.h"
#include <stddef.h>

/* 내부 연결: 다른 번역 단위에서 직접 호출하지 못한다. */
static int fits(unsigned int current, unsigned int amount)
{
    return current <= INVENTORY_LIMIT && amount <= INVENTORY_LIMIT - current;
}

InventoryResult inventory_add(Inventory *inventory, unsigned int amount)
{
    if (inventory == NULL) return INV_ARGUMENT;
    if (!fits(inventory->quantity, amount)) return INV_RANGE;
    inventory->quantity += amount;
    return INV_OK;
}

const char *inventory_result_name(InventoryResult result)
{
    switch (result) {
    case INV_OK: return "ok";
    case INV_RANGE: return "range";
    case INV_ARGUMENT: return "argument";
    default: return "unknown";
    }
}
