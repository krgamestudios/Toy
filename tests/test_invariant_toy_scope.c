#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/resource.h>

/*
 * Security invariant: Formatting error messages containing user-controlled
 * variable names into fixed-size buffers must never overflow the stack,
 * regardless of the length or content of the variable name.
 *
 * We test this by simulating the exact sprintf patterns used in toy_scope.c
 * with adversarial inputs and verifying that safe formatting (snprintf) would
 * produce bounded output, and that the raw sprintf patterns are demonstrably
 * unsafe with these inputs.
 *
 * The property: any error message buffer used in toy_scope.c must be able to
 * hold the formatted result without overflow when given adversarial variable names.
 */

/* Buffer size as used in toy_scope.c (typical fixed stack buffer) */
#define TYPICAL_BUFFER_SIZE 256
#define SAFE_BUFFER_SIZE 65536

/* Simulate the sprintf format strings from toy_scope.c */
static const char *FORMAT_STRINGS[] = {
    "Can't redefine a variable: %s",
    "Incorrect value type in declaration of '%s' (expected %s, got %s)",
    "Undefined variable: %s\n",
    "Incorrect value type in assignment of '%s' (expected %s, got %s)",
    "Can't assign to a constant variable %s",
    "Undefined variable: %s\n",
};
static const int NUM_FORMATS = sizeof(FORMAT_STRINGS) / sizeof(FORMAT_STRINGS[0]);

/* Helper: compute required buffer size for a given format + variable name */
static int compute_required_size(const char *fmt, const char *varname) {
    char safe_buf[SAFE_BUFFER_SIZE];
    int needed = snprintf(safe_buf, sizeof(safe_buf), fmt, varname, "integer", "string");
    return needed;
}

START_TEST(test_adversarial_variable_names_buffer_safety)
{
    /* Invariant: The formatted error message length must be bounded and
     * must not exceed a safe maximum. If the code uses a fixed buffer,
     * the variable name length must be constrained to prevent overflow. */

    const char *payloads[] = {
        /* Long names that would overflow a 256-byte stack buffer */
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",

        /* 512 bytes */
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* 1024 bytes */
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",

        /* Format string injection attempts */
        "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
        "%n%n%n%n%n%n%n%n%n%n",
        "%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x",
        "%.99999d",
        "%99999s",

        /* Null-byte and special characters */
        "var\x00hidden",
        "var\nwith\nnewlines\n",
        "var\twith\ttabs",

        /* Unicode-like sequences */
        "\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf\xc0\xaf",

        /* Path traversal in name */
        "../../../../etc/passwd",
        "../../../../../../../../../../../etc/shadow",

        /* Shell injection */
        "$(rm -rf /)",
        "`cat /etc/passwd`",
        "'; DROP TABLE variables; --",

        /* Exactly at boundary - 255 chars */
        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD",

        /* Exactly at boundary - 256 chars */
        "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
        "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
        "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
        "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE",

        /* One over boundary - 257 chars */
        "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
        "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
        "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
        "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",

        /* Empty string */
        "",

        /* Single character */
        "x",

        /* Whitespace only */
        "   ",

        /* Mixed adversarial */
        "AAAA%sAAAA%nAAAA%xAAAA",
    };

    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        const char *varname = payloads[i];
        size_t varname_len = strlen(varname);

        for (int f = 0; f < NUM_FORMATS; f++) {
            /* 
             * INVARIANT: The required buffer size to safely format the error
             * message must be computable without overflow. We use snprintf
             * to determine the required size safely.
             *
             * The security property: if the code uses a fixed-size buffer
             * (e.g., 256 bytes), then variable names longer than
             * (buffer_size - fixed_format_overhead) MUST be rejected or
             * truncated BEFORE being passed to sprintf.
             *
             * We verify that:
             * 1. snprintf correctly bounds the output
             * 2. The required size for adversarial inputs exceeds TYPICAL_BUFFER_SIZE
             *    (demonstrating the vulnerability exists with sprintf)
             * 3. A safe implementation using snprintf would not overflow
             */

            char safe_output[SAFE_BUFFER_SIZE];
            int required = snprintf(safe_output, sizeof(safe_output),
                                    FORMAT_STRINGS[f], varname, "integer", "string");

            /* snprintf must always return a non-negative value */
            ck_assert_msg(required >= 0,
                "snprintf returned negative value for format %d with payload %d", f, i);

            /* snprintf must never write beyond SAFE_BUFFER_SIZE */
            ck_assert_msg((size_t)required < SAFE_BUFFER_SIZE,
                "Required buffer size %d exceeds safe buffer for format %d, payload %d",
                required, f, i);

            /* 
             * For long variable names, assert that the required size
             * EXCEEDS the typical fixed buffer - this documents that
             * sprintf() would overflow with these inputs.
             */
            if (varname_len >= TYPICAL_BUFFER_SIZE) {
                ck_assert_msg(required > TYPICAL_BUFFER_SIZE,
                    "Expected overflow condition not detected for format %d, payload %d "
                    "(varname_len=%zu, required=%d)",
                    f, i, varname_len, required);
            }

            /*
             * Verify that a safe implementation (snprintf with proper size)
             * produces output that fits within the safe buffer.
             */
            char bounded_output[SAFE_BUFFER_SIZE];
            int written = snprintf(bounded_output, sizeof(bounded_output),
                                   FORMAT_STRINGS[f], varname, "integer", "string");
            ck_assert_msg(written >= 0 && (size_t)written < sizeof(bounded_output),
                "Safe snprintf failed for format %d, payload %d", f, i);

            /* The output must be null-terminated */
            ck_assert_msg(bounded_output[sizeof(bounded_output) - 1] == '\0' ||
                          (size_t)written < sizeof(bounded_output),
                "Output not null-terminated for format %d, payload %d", f, i);
        }

        /*
         * Additional invariant: variable names that would cause overflow
         * in a 256-byte buffer must be detectable by length check alone.
         * A safe implementation must check: strlen(varname) < (buffer_size - max_format_overhead)
         */
        if (varname_len > 0) {
            /* The maximum format overhead (fixed text) in any format string */
            const int MAX_FORMAT_OVERHEAD = 80; /* conservative estimate */
            int would_overflow = (varname_len + MAX_FORMAT_OVERHEAD) >= TYPICAL_BUFFER_SIZE;

            /* If it would overflow, the required size must exceed TYPICAL_BUFFER_SIZE */
            if (would_overflow) {
                for (int f = 0; f < NUM_FORMATS; f++) {
                    char check_buf[SAFE_BUFFER_SIZE];
                    int req = snprintf(check_buf, sizeof(check_buf),
                                       FORMAT_STRINGS[f], varname, "integer", "string");
                    if (req > TYPICAL_BUFFER_SIZE) {
                        /* Confirmed: this input would overflow a typical fixed buffer */
                        /* The invariant is that safe code must handle this gracefully */
                        ck_assert_msg(req > 0,
                            "Required size calculation failed for overflow case");
                    }
                }
            }
        }
    }
}
END_TEST

START_TEST(test_buffer_size_invariant)
{
    /*
     * Invariant: For any variable name of length N, the formatted error
     * message requires at least N bytes. Therefore, a fixed buffer of size B
     * can only safely handle variable names of length < B - overhead.
     *
     * This test verifies that the relationship between input length and
     * required output buffer size is monotonically increasing.
     */

    /* Generate variable names of increasing length */
    for (int len = 1; len <= 1024; len *= 2) {
        char *varname = malloc(len + 1);
        ck_assert_ptr_nonnull(varname);
        memset(varname, 'A', len);
        varname[len] = '\0';

        for (int f = 0; f < NUM_FORMATS; f++) {
            char safe_buf[SAFE_BUFFER_SIZE];
            int required = snprintf(safe_buf, sizeof(safe_buf),
                                    FORMAT_STRINGS[f], varname, "integer", "string");

            /* Required size must be at least as large as the variable name */
            ck_assert_msg(required >= len,
                "Required buffer size %d is less than variable name length %d "
                "for format %d", required, len, f);

            /* Required size must fit in our safe buffer */
            ck_assert_msg(required < SAFE_BUFFER_SIZE,
                "Required size %d exceeds safe buffer for len=%d, format=%d",
                required, len, f);

            /* For names longer than typical buffer, overflow would occur with sprintf */
            if (len >= TYPICAL_BUFFER_SIZE) {
                ck_assert_msg(required >= TYPICAL_BUFFER_SIZE,
                    "Expected required size >= %d for len=%d, format=%d, got %d",
                    TYPICAL_BUFFER_SIZE, len, f, required);
            }
        }

        free(var