#ifndef ATLAS_C13_INVENTORY_H
#define ATLAS_C13_INVENTORY_H

/* enum 상수는 저장 공간을 만들지 않는다. 헤더는 계약을 공개한다. */
enum { INVENTORY_LIMIT = 99 };
typedef struct { unsigned int quantity; } Inventory;
typedef enum { INV_OK, INV_RANGE, INV_ARGUMENT } InventoryResult;

/* 유효한 객체의 수량은 0..INVENTORY_LIMIT. 실패 시 변경하지 않는다. */
InventoryResult inventory_add(Inventory *inventory, unsigned int amount);
const char *inventory_result_name(InventoryResult result);

#endif
