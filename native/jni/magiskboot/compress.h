#pragma once

#include <stream.h>

#include "format.h"

stream_ptr get_encoder(format_t type, sFILE &&fp);

stream_ptr get_decoder(format_t type, sFILE &&fp);

void compress(const char *method, const char *infile, const char *outfile);

void decompress(char *infile, const char *outfile);
