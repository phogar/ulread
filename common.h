
#ifndef HAVE_COMMON_H
#define HAVE_COMMON_H

#include <nfc/nfc.h>
#include <stdbool.h>
#include "ultralight.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

ul_result initialize(nfc_context ** ctx, nfc_device ** nfcdev, ul_device * uldev);
void finalize(nfc_context * ctx, nfc_device * nfcdev);
size_t hex2bin(const char * string, uint8_t * bytes, size_t size);

#endif
