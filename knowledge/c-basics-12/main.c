#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

/* NDEBUG 설정과 무관하게 식을 한 번 평가하는 학습용 검사다. */
static void check(int passed, int line)
{
    if (!passed) { fprintf(stderr, "check failed at line %d\n", line); abort(); }
}
#define CHECK(expression) check((expression), __LINE__)

_Static_assert(CHAR_BIT == 8, "This file format requires 8-bit bytes");
typedef struct { unsigned int coins; unsigned int potions; } Save;
typedef enum { SAVE_OK, SAVE_IO, SAVE_FORMAT } SaveResult;

/* fp는 쓰기 가능한 유효한 스트림, save는 유효한 객체여야 한다.
   객체 표현이 아니라 명시적인 8바이트 형식을 쓴다. */
static SaveResult write_save(FILE *fp, const Save *save)
{
    unsigned char bytes[8] = {0x47, 0x41, 0x4d, 0x45, 1, 0, 0, 0};
    if (save->coins > 65535u || save->potions > 99u) return SAVE_FORMAT;
    bytes[5] = (unsigned char)(save->coins / 256u);
    bytes[6] = (unsigned char)(save->coins % 256u);
    bytes[7] = (unsigned char)save->potions;
    if (fwrite(bytes, 1, sizeof bytes, fp) != sizeof bytes) return SAVE_IO;
    return fflush(fp) == 0 ? SAVE_OK : SAVE_IO;
}

/* fp는 읽기 가능한 유효한 스트림, out은 유효한 출력 객체여야 한다.
   실패하면 *out은 그대로다. */
static SaveResult read_save(FILE *fp, Save *out)
{
    unsigned char bytes[8];
    Save candidate;
    int extra;
    if (fread(bytes, 1, sizeof bytes, fp) != sizeof bytes)
        return ferror(fp) ? SAVE_IO : SAVE_FORMAT;
    extra = fgetc(fp);
    if (extra != EOF) return SAVE_FORMAT;
    if (ferror(fp)) return SAVE_IO;
    if (bytes[0] != 0x47 || bytes[1] != 0x41 || bytes[2] != 0x4d ||
        bytes[3] != 0x45 || bytes[4] != 1 || bytes[7] > 99) return SAVE_FORMAT;
    candidate.coins = (unsigned int)bytes[5] * 256u + bytes[6];
    candidate.potions = bytes[7];
    *out = candidate; /* 끝까지 확인한 뒤 한 번만 커밋한다. */
    return SAVE_OK;
}

/* tmpfile은 이진 갱신 스트림이다. fseek로 쓰기→읽기 전환한다. */
static int check_bytes(const unsigned char *data, size_t length,
                       SaveResult expected)
{
    FILE *fp = tmpfile();
    Save out = {77u, 3u};
    SaveResult result;
    int close_result;
    if (fp == NULL) return 0;
    if (fwrite(data, 1, length, fp) != length ||
        fseek(fp, 0L, SEEK_SET) != 0) { (void)fclose(fp); return 0; }
    result = read_save(fp, &out);
    close_result = fclose(fp);
    if (result == SAVE_IO) return 0; /* 운영 I/O 오류는 형식 검사 실패와 구별한다. */
    CHECK(result == expected);
    if (expected != SAVE_OK) CHECK(out.coins == 77u && out.potions == 3u);
    if (expected == SAVE_OK) CHECK(out.coins == 0u && out.potions == 0u);
    return close_result == 0;
}

int main(void)
{
    FILE *fp = tmpfile();
    Save original = {65535u, 99u}, loaded = {0u, 0u};
    unsigned char valid[8] = {0x47, 0x41, 0x4d, 0x45, 1, 0, 0, 0};
    unsigned char trailing[9] = {0x47, 0x41, 0x4d, 0x45, 1, 0, 1, 2, 0};
    if (fp == NULL) return 1;
    if (write_save(fp, &original) != SAVE_OK || fseek(fp, 0L, SEEK_SET) != 0 ||
        read_save(fp, &loaded) != SAVE_OK) { (void)fclose(fp); return 1; }
    if (fclose(fp) != 0) return 1;
    CHECK(loaded.coins == 65535u && loaded.potions == 99u);
    if (!check_bytes(valid, sizeof valid, SAVE_OK) ||
        !check_bytes(valid, 0, SAVE_FORMAT) ||
        !check_bytes(valid, 7, SAVE_FORMAT) ||
        !check_bytes(trailing, sizeof trailing, SAVE_FORMAT)) return 1;
    valid[4] = 2; /* 미지원 버전도 조용히 해석하지 않는다. */
    if (!check_bytes(valid, sizeof valid, SAVE_FORMAT)) return 1;
    valid[4] = 1; valid[7] = 100;
    if (!check_bytes(valid, sizeof valid, SAVE_FORMAT)) return 1;
    valid[7] = 0; valid[0] = 0;
    if (!check_bytes(valid, sizeof valid, SAVE_FORMAT)) return 1;
    fp = tmpfile();
    if (fp == NULL) return 1;
    original.potions = 100;
    CHECK(write_save(fp, &original) == SAVE_FORMAT);
    if (fclose(fp) != 0) return 1;
    puts("save: round-trip, boundaries, rejection, rollback OK");
    return 0;
}
