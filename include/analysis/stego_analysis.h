#ifndef STEGO_ANALYSIS_H
#define STEGO_ANALYSIS_H

#include "../bmp/bmp.h"

#include <stddef.h>

typedef enum {
    STEGO_ANALYSIS_METHOD_UNKNOWN = 0,
    STEGO_ANALYSIS_METHOD_LSB1,
    STEGO_ANALYSIS_METHOD_LSB4,
    STEGO_ANALYSIS_METHOD_LSBI
} StegoAnalysisMethod;

typedef struct {
    int has_payload;
    StegoAnalysisMethod method;
    size_t declared_payload_size;
    size_t extracted_payload_size;
    unsigned char *payload;
} StegoAnalysisResult;

void stego_analysis_result_init(StegoAnalysisResult *result);
void stego_analysis_result_free(StegoAnalysisResult *result);
int stego_analysis_run(const BMP *bmp, StegoAnalysisResult *result);
const char *stego_analysis_method_to_string(StegoAnalysisMethod method);

#endif
