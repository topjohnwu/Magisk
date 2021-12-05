#pragma once

#define AVB2_INVALID     (1 << 1)
#define AVB2_UNSUPPORTED (1 << 2)

int avb2_verify_sign(const char *image, const char *privkey);
std::string mem_to_hexstring(const uint8_t* data, size_t len);
