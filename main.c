#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stegobmp.h"

int main(const int argc, char * argv[]) {

  if (argc < 9) {
    printf("Usage: %s -embed -in <input> -p <bmp> -out <bmp_out> -steg <LSB1|LSB4|LSBI> [-a <aes128|aes192|aes256|3des>] [-m <ecb|cfb|ofb|cbc>] [-pass <password>]\n", argv[0]);
    printf("Usage: %s -extract -p <bmp> -out <bmp_out> -steg <LSB1|LSB4|LSBI> [-a <aes128|aes192|aes256|3des>] [-m <ecb|cfb|ofb|cbc>] [-pass <password>]\n", argv[0]);
    return 1;
  }

  int embed = 0;
  int extract = 0;
  char *input_filename = NULL;
  const char *bmp_filename = NULL;
  char *output_bmp_filename = NULL;
  const char *steganography_method = NULL;
  const char *encryption_method = NULL;
  const char *encryption_mode = NULL;
  const char *password = NULL;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-embed") == 0) {
      embed = 1;
    } else if (strcmp(argv[i], "-extract") == 0) {
      extract = 1;
    } else if (strcmp(argv[i], "-in") == 0) {
      input_filename = argv[i + 1];
    } else if (strcmp(argv[i], "-p") == 0) {
      bmp_filename = argv[i + 1];
    } else if (strcmp(argv[i], "-out") == 0) {
      output_bmp_filename = argv[i + 1];
    } else if (strcmp(argv[i], "-steg") == 0) {
      steganography_method = argv[i + 1];
    } else if (strcmp(argv[i], "-a") == 0) {
      encryption_method = argv[i + 1];
    } else if (strcmp(argv[i], "-m") == 0) {
      encryption_mode = argv[i + 1];
    } else if (strcmp(argv[i], "-pass") == 0) {
      password = argv[i + 1];
    }
  }

  if (embed) {
    const int hidden_file_in_bmp = hide_file_in_bmp(
        input_filename,
        bmp_filename,
        output_bmp_filename,
        steganography_method,
        encryption_method,
        encryption_mode,
        password
        );
    if (hidden_file_in_bmp) {
      printf("Can not hide file %s\n", input_filename);
    } else {
      printf("File successfully hidden in %s\n", output_bmp_filename);
    }
  }

  if (extract)
  {
    const int extracted_file_in_bmp = extract_file_in_bmp(
      bmp_filename,
      output_bmp_filename,
      steganography_method,
      encryption_method,
      encryption_mode,
      password);
    if (extracted_file_in_bmp) {
      printf("Can not extract file %s\n", output_bmp_filename);
    } else {
      printf("File successfully extracted in %s\n", output_bmp_filename);
    }
  }

  return 0;
}