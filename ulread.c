
#include <stdio.h>
#include "common.h"
#include "ultralight.h"

int execute(ul_device * dev) {
	uint32_t buffer[UL_RDPAGS];
	unsigned int page, i;
	ul_result ret;

	for (page = 0; page < dev->type->pages; page += UL_RDPAGS) {
		ret = ul_read(dev, page, buffer);
		if (ret) {
			fprintf(stderr, "* Unexpected error while reading tag *\n");
			return ret;
		}

		fprintf(stdout, "[%02X]", page);
		for (i = 0; i < UL_RDPAGS && page + i < dev->type->pages; i++) {
			if (i + page >= dev->type->pages - dev->type->password_pages) {
				fprintf(stdout, " ????????");
			} else {
				fprintf(stdout, " %08X", buffer[i]);
			}
		}
		fprintf(stdout, "\n");
	}

	return 0;
}

int main() {
	nfc_context * ctx;
	nfc_device * nfcdev;
	ul_device uldev;
	ul_result ret;

	ret = initialize(&ctx, &nfcdev, &uldev);
	if (ret) {
		return ret;
	}

	ret = execute(&uldev);
	finalize(ctx, nfcdev);

	return ret;
}
