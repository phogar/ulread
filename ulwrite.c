
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "common.h"
#include "ultralight.h"

ul_result execute(ul_device * dev, unsigned int page, unsigned int count, bool lenient, ul_page key) {
	ul_page buffer[UL_WRPAGS];
	ul_result ret;

	page = min(dev->type->pages, page);
	count = min(dev->type->pages - page, count);
	fprintf(stderr, "Writing %u pages starting at page 0x%02X (lenient: %s)\n", count, page, lenient ? "true" : "false");

	bool auth = false;
	while (count > 0) {
		if (key != NULL && !auth) {
			ul_passack pack;
			ret = ul_authenticate(dev, key, pack);
			fprintf(stderr, "Pack: %02X %02X", pack[0], pack[1]);
			if (ret) {
				fprintf(stderr, "* Error authenticating with given key: %02X %02X %02X %02X (err %i) *\n", key[0], key[1], key[2], key[3], ret);
				return ret;
			}
			auth = true;
		}

		if (!fread(buffer, sizeof(buffer), 1, stdin)) {
			fprintf(stderr, "* End of input stream *\n");
			return UL_ERROR;
		}

		ret = ul_write(dev, page, buffer);
		if (ret) {
			fprintf(stderr, "* Error writing page 0x%02X *\n", page);
			if (!lenient) {
				return ret;
			}
			auth = false;
		}

		page++;
		count--;
	}

	return UL_OK;
}

int main(int argc, char ** argv) {
	nfc_context * ctx;
	nfc_device * nfcdev;
	ul_device uldev;
	ul_result ret;

	ul_page key;
	bool hasKey = false;
	unsigned int startPage = 0;
	unsigned int pageCount = ~0;
	bool lenient = false;

	while (true) {
		int c = getopt(argc, argv, "s:c:lk:");
		if (c == -1) {
			break;
		}

		switch (c) {
			case 's':
				startPage = strtoul(optarg, NULL, 0);
				break;

			case 'c':
				pageCount = strtoul(optarg, NULL, 0);
				break;

			case 'l':
				lenient = true;
				break;

			case 'k':
				hex2bin(optarg, key, UL_PAGSIZE);
				hasKey = true;
				break;
		}
	}

	ret = initialize(&ctx, &nfcdev, &uldev);
	if (ret) {
		return ret;
	}

	ret = execute(&uldev, startPage, pageCount, lenient, hasKey ? key : NULL);
	finalize(ctx, nfcdev);

	return ret;
}
