#include "../../include/analysis/stego_analysis.h"
#include "../../include/stegobmp/stegobmp_lsb.h"
#include "../../include/stegobmp/stegobmp_utils.h"
#include "../../include/bmp/bmp_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

typedef unsigned char *(*stego_analysis_retrieve_fn)(const BMP *, size_t *);

typedef struct {
    StegoAnalysisMethod method;
    stego_analysis_retrieve_fn retrieve_fn;
} StegoAnalysisCandidate;

static unsigned char *call_retrieve_quiet(stego_analysis_retrieve_fn retrieve_fn, const BMP *bmp, size_t *extracted_size) {
    if (!retrieve_fn) {
        return NULL;
    }

    fflush(stdout);

    const int stdout_fd = dup(STDOUT_FILENO);
    if (stdout_fd < 0) {
        return retrieve_fn(bmp, extracted_size);
    }

    const int devnull_fd = open("/dev/null", O_WRONLY);
    if (devnull_fd < 0) {
        close(stdout_fd);
        return retrieve_fn(bmp, extracted_size);
    }

    if (dup2(devnull_fd, STDOUT_FILENO) < 0) {
        close(devnull_fd);
        close(stdout_fd);
        return retrieve_fn(bmp, extracted_size);
    }

    close(devnull_fd);

    unsigned char *payload_buffer = retrieve_fn(bmp, extracted_size);

    fflush(stdout);
    dup2(stdout_fd, STDOUT_FILENO);
    close(stdout_fd);

    return payload_buffer;
}

static int validate_payload_buffer(const unsigned char *payload_buffer, size_t payload_size, size_t *declared_payload_size) {
    if (!payload_buffer || payload_size < BMP_INT_SIZE_BYTES + STEGOBMP_NULL_CHARACTER_SIZE) {
        return 0;
    }

    const uint32_t declared_size = read_uint32_big_endian(payload_buffer);
    size_t extension_offset = 0;
    size_t extension_length = 0;

    if (!stego_payload_locate_extension(payload_buffer, payload_size, declared_size, &extension_offset, &extension_length)) {
        return 0;
    }

    const size_t required_size = BMP_INT_SIZE_BYTES + (size_t) declared_size + extension_length + STEGOBMP_NULL_CHARACTER_SIZE;
    if (required_size > payload_size) {
        return 0;
    }

    if (declared_payload_size) {
        *declared_payload_size = declared_size;
    }
    return 1;
}

void stego_analysis_result_init(StegoAnalysisResult *result) {
    if (!result) {
        return;
    }
    result->has_payload = 0;
    result->method = STEGO_ANALYSIS_METHOD_UNKNOWN;
    result->declared_payload_size = 0;
    result->extracted_payload_size = 0;
    result->payload = NULL;
}

void stego_analysis_result_free(StegoAnalysisResult *result) {
    if (!result) {
        return;
    }
    free(result->payload);
    stego_analysis_result_init(result);
}

const char *stego_analysis_method_to_string(const StegoAnalysisMethod method) {
    switch (method) {
        case STEGO_ANALYSIS_METHOD_LSB1:
            return STEGOBMP_LSB1_METHOD;
        case STEGO_ANALYSIS_METHOD_LSB4:
            return STEGOBMP_LSB4_METHOD;
        case STEGO_ANALYSIS_METHOD_LSBI:
            return STEGOBMP_LSBI_METHOD;
        default:
            return "UNKNOWN";
    }
}

int stego_analysis_run(const BMP *bmp, StegoAnalysisResult *result) {
    if (!bmp || !result) {
        return 1;
    }

    stego_analysis_result_init(result);

    const StegoAnalysisCandidate candidates[] = {
        { STEGO_ANALYSIS_METHOD_LSB1, lsb_1_retrieve },
        { STEGO_ANALYSIS_METHOD_LSB4, lsb_4_retrieve },
        { STEGO_ANALYSIS_METHOD_LSBI, lsb_i_retrieve }
    };

    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
        size_t extracted_size = 0;
        unsigned char *payload_buffer = call_retrieve_quiet(candidates[i].retrieve_fn, bmp, &extracted_size);
        if (!payload_buffer) {
            continue;
        }

        size_t declared_size = 0;
        const int payload_valid = validate_payload_buffer(payload_buffer, extracted_size, &declared_size);
        if (!payload_valid) {
            free(payload_buffer);
            continue;
        }

        result->has_payload = 1;
        result->method = candidates[i].method;
        result->declared_payload_size = declared_size;
        result->extracted_payload_size = extracted_size;
        result->payload = payload_buffer;
        return 0;
    }

    return 1;
}
