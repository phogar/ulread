
#include <stdio.h>
#include "common.h"
#include "ultralight.h"

ul_result execute(ul_device * dev) {
	ul_page buffer[UL_RDPAGS];
	unsigned int page, i, j;
	ul_result ret;

	for (page = 0; page < dev->type->pages; page += UL_RDPAGS) {
		ret = ul_read(dev, page, buffer);
		if (ret) {
			fprintf(stderr, "* Unexpected error while reading tag *\n");
			return ret;
		}

		for (i = 0; i < UL_RDPAGS && page + i < dev->type->pages - dev->type->password_pages; i++) {
			for (j = 0; j < UL_PAGSIZE; j++) {
				if (fputc(buffer[i][j], stdout) == EOF) {
					fprintf(stderr, "* Unexpected end of output stream *\n");
					return UL_ERROR;
				}
			}
		}
	}

	return UL_OK;
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
