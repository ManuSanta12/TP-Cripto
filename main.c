#include <stdio.h>
#include "include/stegobmp.h"
#include "include/parser.h"

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

    bmp_free(bmp);

    return 0;
}
