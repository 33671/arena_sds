#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "sds.h"

int main(void)
{
    Arena a = {0};

    printf("=== Arena SDS tests ===\n");

    /* 1. Basic create / dup / free */
    {
        sds s1 = sdsnew(&a, "hello");
        assert(sdslen(s1) == 5);
        assert(strcmp(s1, "hello") == 0);
        printf("[PASS] sdsnew + sdslen\n");

        sds s2 = sdsdup(s1);
        assert(sdslen(s2) == 5);
        assert(strcmp(s2, "hello") == 0);
        printf("[PASS] sdsdup (same arena)\n");

        sdsfree(s1);
        sdsfree(s2);
        /* sdsfree is no-op in arena mode, just test it doesn't crash */
        printf("[PASS] sdsfree no-op in arena mode\n");
    }

    /* 2. sdsempty + sdscat */
    {
        sds s = sdsempty(&a);
        assert(sdslen(s) == 0);
        printf("[PASS] sdsempty\n");

        s = sdscat(s, "foo");
        s = sdscat(s, "bar");
        assert(sdslen(s) == 6);
        assert(strcmp(s, "foobar") == 0);
        printf("[PASS] sdscat\n");
    }

    /* 3. sdsnewlen (binary safe) */
    {
        sds s = sdsnewlen(&a, "ab\0cd", 5);
        assert(sdslen(s) == 5);
        assert(memcmp(s, "ab\0cd", 5) == 0);
        printf("[PASS] sdsnewlen binary-safe\n");
    }

    /* 4. sdscatsds */
    {
        sds a1 = sdsnew(&a, "Hello, ");
        sds a2 = sdsnew(&a, "World!");
        sds joined = sdscatsds(a1, a2);
        assert(sdslen(joined) == 13);
        assert(strcmp(joined, "Hello, World!") == 0);
        printf("[PASS] sdscatsds\n");
    }

    /* 5. sdscpy / sdscpylen */
    {
        sds s = sdsnewlen(&a, "xxxxxxxxxx", 10);
        s = sdscpy(s, "abc");
        assert(sdslen(s) == 3);
        assert(strcmp(s, "abc") == 0);
        printf("[PASS] sdscpy (shorter)\n");

        s = sdscpy(s, "a very long string that needs realloc");
        assert(sdslen(s) == 37);
        assert(strcmp(s, "a very long string that needs realloc") == 0);
        printf("[PASS] sdscpy (longer, cross-type realloc)\n");
    }

    /* 6. Multi-arena: two arenas simultaneously */
    {
        Arena arena1 = {0}, arena2 = {0};

        sds s1 = sdsnew(&arena1, "from arena 1");
        sds s2 = sdsnew(&arena2, "from arena 2");

        assert(strcmp(s1, "from arena 1") == 0);
        assert(strcmp(s2, "from arena 2") == 0);

        /* Modify s2 - it should use arena2 */
        s2 = sdscat(s2, " modified");
        assert(strcmp(s2, "from arena 2 modified") == 0);

        arena_free(&arena1);
        arena_free(&arena2);
        printf("[PASS] Multi-arena\n");
    }

    /* 7. sdsMakeRoomFor + sdsIncrLen (low-level API) */
    {
        sds s = sdsnewlen(&a, "abc", 3);
        size_t oldlen = sdslen(s);
        s = sdsMakeRoomFor(s, 10);
        assert(sdslen(s) == oldlen);  /* length unchanged */
        assert(sdsavail(s) >= 10);    /* but space available */

        /* Manually write and commit */
        memcpy(s + oldlen, "defghij", 7);
        sdsIncrLen(s, 7);
        assert(sdslen(s) == 10);
        s[10] = '\0';
        assert(strcmp(s, "abcdefghij") == 0);
        printf("[PASS] sdsMakeRoomFor + sdsIncrLen\n");
    }

    /* 8. sdstrim */
    {
        sds s = sdsnew(&a, "   hello world   ");
        s = sdstrim(s, " ");
        assert(sdslen(s) == 11);
        assert(strcmp(s, "hello world") == 0);
        printf("[PASS] sdstrim\n");
    }

    /* 9. sdsrange */
    {
        sds s = sdsnew(&a, "Hello World");
        sdsrange(s, 0, 4);
        assert(strcmp(s, "Hello") == 0);
        printf("[PASS] sdsrange\n");
    }

    /* 10. sdscmp */
    {
        sds s1 = sdsnew(&a, "abc");
        sds s2 = sdsnew(&a, "abd");
        assert(sdscmp(s1, s2) < 0);
        assert(sdscmp(s2, s1) > 0);

        sds s3 = sdsnew(&a, "abc");
        assert(sdscmp(s1, s3) == 0);
        printf("[PASS] sdscmp\n");
    }

    /* 11. sdssplitlen */
    {
        int count;
        sds *tokens = sdssplitlen(&a, "a,b,c,d", 7, ",", 1, &count);
        assert(count == 4);
        assert(strcmp(tokens[0], "a") == 0);
        assert(strcmp(tokens[1], "b") == 0);
        assert(strcmp(tokens[2], "c") == 0);
        assert(strcmp(tokens[3], "d") == 0);
        sdsfreesplitres(tokens, count);
        printf("[PASS] sdssplitlen\n");
    }

    /* 12. sdsjoin */
    {
        char *words[] = {"this", "is", "a", "test"};
        sds s = sdsjoin(&a, words, 4, " ");
        assert(strcmp(s, "this is a test") == 0);
        printf("[PASS] sdsjoin\n");
    }

    /* 13. sdsfromlonglong */
    {
        sds s = sdsfromlonglong(&a, -12345);
        assert(strcmp(s, "-12345") == 0);
        printf("[PASS] sdsfromlonglong\n");
    }

    /* 14. sdscatfmt */
    {
        sds s = sdsnew(&a, ">> ");
        s = sdscatfmt(s, "int=%i, str=%s", 42, "hello");
        assert(strcmp(s, ">> int=42, str=hello") == 0);
        printf("[PASS] sdscatfmt\n");
    }

    /* 15. sdscatprintf */
    {
        sds s = sdsempty(&a);
        s = sdscatprintf(s, "%d + %d = %d", 1, 2, 3);
        assert(strcmp(s, "1 + 2 = 3") == 0);
        printf("[PASS] sdscatprintf\n");
    }

    /* 16. sdsgrowzero */
    {
        sds s = sdsnew(&a, "hi");
        s = sdsgrowzero(s, 10);
        assert(sdslen(s) == 10);
        // bytes 2-9 should be zero
        for (size_t i = 2; i < 10; i++)
            assert(s[i] == '\0');
        printf("[PASS] sdsgrowzero\n");
    }

    /* 17. arena_reset reuses memory */
    {
        arena_reset(&a);

        /* Same arena, fresh allocation */
        sds s2 = sdsnew(&a, "after reset");
        assert(strcmp(s2, "after reset") == 0);
        /* The old pointer is invalid after reset, but we don't use it */
        printf("[PASS] arena_reset + reuse\n");
    }

    /* 18. sdstolower / sdstoupper */
    {
        sds s = sdsnew(&a, "Hello World");
        sdstolower(s);
        assert(strcmp(s, "hello world") == 0);
        sdstoupper(s);
        assert(strcmp(s, "HELLO WORLD") == 0);
        printf("[PASS] sdstolower / sdstoupper\n");
    }

    /* Clean up */
    arena_free(&a);

    printf("\n=== All arena tests passed! ===\n");
    return 0;
}
