#ifndef STEGOBMP_PARSER_H
#define STEGOBMP_PARSER_H

typedef struct {
    int embed;
    int extract;
    const char *input_filename;
    const char *bmp_filename;
    const char *output_bmp_filename;
    const char *steganography_method;
    const char *encryption_method;
    const char *encryption_mode;
    const char *password;
} ProgramArguments;

int parse_arguments(int argc, char *argv[], ProgramArguments *arguments);

#endif //STEGOBMP_PARSER_H