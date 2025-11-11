#include "include/stegobmp/stegobmp.h"
#include "include/parser/parser.h"
#include "include/analysis/stego_analysis.h"
#include "include/stegobmp/stegobmp_utils.h"

#include <stdio.h>
#include <stdlib.h>

int main(const int argc, char* argv[]) {

    ProgramArguments arguments = {0};
    if (parse_arguments(argc, argv, &arguments)) {
        return 1;
    }

    BMP *bmp = bmp_read(arguments.bmp_filename);
    if (!bmp) {
        printf("Error: Can not read BMP file: %s\n", arguments.bmp_filename);
        return 1;
    }

    if (arguments.embed) {
        const int embed_status = hide_file_in_bmp(
            arguments.input_filename,
            bmp,
            arguments.output_bmp_filename,
            arguments.steganography_method,
            arguments.encryption_method,
            arguments.encryption_mode,
            arguments.password
        );
        if (embed_status){
            printf("Error: Can not embed file %s\n", arguments.input_filename);
            bmp_free(bmp);
            return 1;
        }
        printf("File successfully embedded\n");

        const int write_status = bmp_write(bmp, arguments.output_bmp_filename);
        if (write_status) {
            printf("Error: Can not write BMP file: %s\n", arguments.output_bmp_filename);
            bmp_free(bmp);
            return 1;
        }
        printf("File successfully written to %s\n", arguments.output_bmp_filename);
    }

    if (arguments.extract) {
        const int extracted_file_in_bmp = extract_file_from_bmp(
            bmp,
            arguments.output_bmp_filename,
            arguments.steganography_method,
            arguments.encryption_method,
            arguments.encryption_mode,
            arguments.password
            );
        if (extracted_file_in_bmp) {
            printf("Error: Can not extract file %s\n", arguments.output_bmp_filename);
            bmp_free(bmp);
            return 1;
        }
        printf("File successfully extracted in %s\n", arguments.output_bmp_filename);
    }

    if (arguments.analyze) {
        StegoAnalysisResult analysis_result;
        stego_analysis_result_init(&analysis_result);

        const int analysis_status = stego_analysis_run(bmp, &analysis_result);
        if (analysis_status || !analysis_result.has_payload) {
            printf("No payload detected in BMP\n");
            if (analysis_result.has_method_guess && analysis_result.method_guess != STEGO_ANALYSIS_METHOD_UNKNOWN) {
                printf(
                    "Most likely method: %s (signature %s detected at offset %zu)\n",
                    stego_analysis_method_to_string(analysis_result.method_guess),
                    analysis_result.method_guess_signature ? analysis_result.method_guess_signature : "unknown",
                    analysis_result.method_guess_signature_offset
                );
            }
        } else {
            printf("Payload detected using method: %s\n", stego_analysis_method_to_string(analysis_result.method));
            printf("Declared payload size: %zu bytes\n", analysis_result.declared_payload_size);

            if (analysis_result.payload_format == STEGO_ANALYSIS_PAYLOAD_FORMAT_ENCRYPTED) {
                printf("Detected encrypted payload metadata.\n");
                if (arguments.encryption_method && arguments.encryption_mode && arguments.password) {
                    unsigned char *decrypted_payload = NULL;
                    size_t decrypted_payload_size = 0;
                    if (stego_analysis_decrypt_payload(&analysis_result, arguments.encryption_method, arguments.encryption_mode, arguments.password, &decrypted_payload, &decrypted_payload_size) == 0) {
                        if (save_extracted_file(decrypted_payload, decrypted_payload_size, arguments.output_bmp_filename) == 0) {
                            printf("Payload saved to %s\n", arguments.output_bmp_filename);
                        } else {
                            printf("Warning: Payload detected but could not be saved to %s\n", arguments.output_bmp_filename);
                        }
                        free(decrypted_payload);
                    } else {
                        printf("Warning: Detected encrypted payload but decryption failed with the provided parameters\n");
                    }
                } else {
                    printf("Warning: Payload appears encrypted. Provide -a <method> -m <mode> -pass <password> to attempt decryption.\n");
                }
            } else if (arguments.output_bmp_filename) {
                if (save_extracted_file(analysis_result.payload, analysis_result.extracted_payload_size, arguments.output_bmp_filename) == 0) {
                    printf("Payload saved to %s\n", arguments.output_bmp_filename);
                } else {
                    printf("Warning: Payload detected but could not be saved to %s\n", arguments.output_bmp_filename);
                }
            }
        }

        stego_analysis_result_free(&analysis_result);
    }

    bmp_free(bmp);

    return 0;
}
