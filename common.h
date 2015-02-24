
#ifndef HAVE_COMMON_H
#define HAVE_COMMON_H

#include <nfc/nfc.h>
#include <stdbool.h>
#include "ultralight.h"

ul_result initialize(nfc_context ** ctx, nfc_device ** nfcdev, ul_device * uldev);
void finalize(nfc_context * ctx, nfc_device * nfcdev);

#endif
