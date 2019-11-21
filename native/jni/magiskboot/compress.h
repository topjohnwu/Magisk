#pragma once

#include <stream.h>

#include "format.h"

filter_stream *get_encoder(format_t type, FILE *fp = nullptr);
filter_stream *get_decoder(format_t type, FILE *fp = nullptr);
void compress(const char *method, const char *infile, const char *outfile);
void decompress(char *infile, const char *outfile);
