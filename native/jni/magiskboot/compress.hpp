#pragma once

#include <stream.hpp>

#include "format.hpp"

stream_ptr get_encoder(format_t type, stream_ptr &&base);

stream_ptr get_decoder(format_t type, stream_ptr &&base);

void compress(const char *method, const char *infile, const char *outfile);

void decompress(char *infile, const char *outfile);
