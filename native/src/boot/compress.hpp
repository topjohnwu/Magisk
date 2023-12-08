#pragma once

#include <cxx.h>
#include <stream.hpp>

#include "format.hpp"

out_strm_ptr get_encoder(format_t type, out_strm_ptr &&base);
out_strm_ptr get_decoder(format_t type, out_strm_ptr &&base);
void compress(const char *method, const char *infile, const char *outfile);
void decompress(char *infile, const char *outfile);
bool decompress(rust::Slice<const uint8_t> buf, int fd);
bool xz(rust::Slice<const uint8_t> buf, rust::Vec<uint8_t> &out);
bool unxz(rust::Slice<const uint8_t> buf, rust::Vec<uint8_t> &out);
