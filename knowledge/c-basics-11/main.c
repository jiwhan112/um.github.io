#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { LINE_OK, LINE_END, LINE_LONG, LINE_NUL, LINE_IO, LINE_ARG } LineStatus;
typedef enum { NUMBER_OK, NUMBER_EMPTY, NUMBER_RANGE, NUMBER_JUNK, NUMBER_ARG } NumberStatus;

/* 긴 줄도 끝까지 소비하여 다음 호출이 같은 줄의 꼬리를 읽지 않게 합니다. */
static LineStatus read_line(FILE *stream, char *out, size_t capacity)
{
    if (stream == NULL || out == NULL || capacity < 2) return LINE_ARG;
    size_t used = 0;
    int ch, seen = 0, too_long = 0, has_nul = 0;
    while ((ch = fgetc(stream)) != EOF) {
        seen = 1;
        if (ch == '\n') break;
        if (ch == '\0') has_nul = 1;
        if (used < capacity - 1) out[used++] = (char)ch;
        else too_long = 1;
    }
    out[used] = '\0';
    if (ferror(stream)) return LINE_IO;
    if (!seen && ch == EOF) return LINE_END;
    if (has_nul) return LINE_NUL;
    if (too_long) return LINE_LONG;
    return LINE_OK; /* 마지막 줄의 개행이 없어도 읽은 내용이 있으면 성공입니다. */
}

/* 게임 인원 수 1..8만 허용합니다. 실패하면 *out을 변경하지 않습니다. */
static NumberStatus parse_players(const char *text, int *out)
{
    if (text == NULL || out == NULL) return NUMBER_ARG;
    char *end;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (end == text) return NUMBER_EMPTY;
    if (errno == ERANGE || value < 1 || value > 8) return NUMBER_RANGE;
    while (isspace((unsigned char)*end)) ++end;
    if (*end != '\0') return NUMBER_JUNK;
    *out = (int)value; /* 범위 검증 후에만 좁은 타입으로 변환합니다. */
    return NUMBER_OK;
}

int main(void)
{
    int players = 7;
    NumberStatus number = parse_players(" 4 \t", &players);
    assert(number == NUMBER_OK && players == 4);
    number = parse_players("", &players);
    assert(number == NUMBER_EMPTY && players == 4);
    number = parse_players("  ", &players);
    assert(number == NUMBER_EMPTY);
    number = parse_players("3x", &players);
    assert(number == NUMBER_JUNK && players == 4);
    number = parse_players("0", &players);
    assert(number == NUMBER_RANGE);
    number = parse_players("-1", &players);
    assert(number == NUMBER_RANGE);
    number = parse_players("9", &players);
    assert(number == NUMBER_RANGE);
    number = parse_players("9999999999999999999999999999999999999999", &players);
    assert(number == NUMBER_RANGE && players == 4);
    number = parse_players("1", &players);
    assert(number == NUMBER_OK && players == 1);
    number = parse_players("8", &players);
    assert(number == NUMBER_OK && players == 8);
    puts("parse: boundaries 1,8 accepted; empty/junk/range rejected");

    FILE *stream = tmpfile();
    if (stream == NULL) { fputs("temporary stream unavailable\n", stderr); return 1; }
    const char input[] = "abcd\n2\na\0b\n\n123\n8";
    if (fwrite(input, 1, sizeof input - 1, stream) != sizeof input - 1 ||
        fflush(stream) != 0 || fseek(stream, 0, SEEK_SET) != 0) {
        fputs("stream setup failed\n", stderr); fclose(stream); return 1;
    }
    char line[4];
    LineStatus status = read_line(stream, line, sizeof line);
    assert(status == LINE_LONG);
    status = read_line(stream, line, sizeof line);
    assert(status == LINE_OK && strcmp(line, "2") == 0);
    status = read_line(stream, line, sizeof line);
    assert(status == LINE_NUL);
    status = read_line(stream, line, sizeof line);
    assert(status == LINE_OK && line[0] == '\0');
    status = read_line(stream, line, sizeof line);
    assert(status == LINE_OK && strcmp(line, "123") == 0);
    status = read_line(stream, line, sizeof line);
    assert(status == LINE_OK && strcmp(line, "8") == 0);
    status = read_line(stream, line, sizeof line);
    assert(status == LINE_END);
    status = read_line(stream, line, 1);
    assert(status == LINE_ARG);
    if (fclose(stream) != 0) { fputs("stream close failed\n", stderr); return 1; }
    puts("lines: long drained; NUL rejected; empty/exact/final accepted; EOF separate");
    return 0;
}
