
#include <stdio.h>
#include "common.h"

bool open_default_device(nfc_context ** retCtx, nfc_device ** retDev) {
	nfc_context * ctx;
	nfc_device * dev;

	nfc_init(&ctx);
	if (ctx == NULL) {
		perror("nfc_init");
		goto fail0;
	}

	dev = nfc_open(ctx, NULL);
	if (dev == NULL) {
		perror("nfc_open");
		goto fail1;
	}

	if (nfc_initiator_init(dev) < 0) {
		nfc_perror(dev, "nfc_initiator_init");
		goto fail2;
	}

	if (nfc_device_set_property_bool(dev, NP_INFINITE_SELECT, false) < 0) {
		nfc_perror(dev, "nfc_device_set_property_bool");
		goto fail2;
	}

	*retCtx = ctx;
	*retDev = dev;
	return true;

fail2:
	nfc_close(dev);
fail1:
	nfc_exit(ctx);
fail0:
	return false;
}

ul_result initialize(nfc_context ** ctx, nfc_device ** nfcdev, ul_device * uldev) {
	if (!open_default_device(ctx, nfcdev)) {
		return UL_ERROR;
	}

	ul_result ret;
	unsigned int i;

	ret = ul_detect(*nfcdev, uldev);
	if (ret) {
		fprintf(stderr, "* Unable to detect Ultralight tag *\n");
		return ret;
	}

	fprintf(stderr, "Detected Ultralight\n");
	fprintf(stderr, " - UID:");
	for (i = 0; i < uldev->id_size; i++) {
		fprintf(stderr, " %02X", uldev->id[i]);
	}
	fprintf(stderr, "\n");

	fprintf(stderr, " - Model: %s\n - %d pages (%d bytes)\n - %d write-only password pages (%d bytes)\n", uldev->type->name, uldev->type->pages, 4 * uldev->type->pages, uldev->type->password_pages, 4 * uldev->type->password_pages);
	if (uldev->type->pages == 0) {
		fprintf(stderr, "* Unsupported model *\n");
		return UL_UNSUPPORTED;
	}

	return UL_OK;
}

void finalize(nfc_context * ctx, nfc_device * nfcdev) {
	nfc_close(nfcdev);
	nfc_exit(ctx);
}

