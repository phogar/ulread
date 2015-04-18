
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "common.h"
#include "ultralight.h"

ul_result execute(ul_device * dev, unsigned int page, unsigned int count, bool lenient) {
	ul_page buffer[UL_RDPAGS];
	ul_result ret;

	page = min(dev->type->pages - dev->type->write_only_pages, page);
	count = min(dev->type->pages - dev->type->write_only_pages - page, count);
	fprintf(stderr, "Reading %u pages starting at page %02X\n", count, page);

	while (count > 0) {
		unsigned int printpags = min(count, UL_RDPAGS);

		ret = ul_read(dev, page, buffer);
		if (ret) {
			fprintf(stderr, "* Unexpected error while reading tag (pages 0x%02X ~ 0x%02X) *\n", page, page + UL_RDPAGS - 1);
			if (!lenient) {
				return ret;
			}

			buffer[0][0] = 0xDE;
			buffer[0][1] = 0xAD;
			buffer[0][2] = 0xBE;
			buffer[0][3] = 0xEF;
			printpags = 1;
		}

		if (fwrite(buffer, UL_PAGSIZE, printpags, stdout) != printpags) {
			fprintf(stderr, "* Unexpected end of output stream *\n");
			return UL_ERROR;
		}

		page += printpags;
		count -= printpags;
	}

	return UL_OK;
}

int main(int argc, char ** argv) {
	nfc_context * ctx;
	nfc_device * nfcdev;
	ul_device uldev;
	ul_result ret;

	unsigned int startPage = 0;
	unsigned int pageCount = ~0;
	bool lenient = false;
	while (true) {
		int c = getopt(argc, argv, "s:c:l");
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
		}
	}

	ret = initialize(&ctx, &nfcdev, &uldev);
	if (ret) {
		return ret;
	}

	ret = execute(&uldev, startPage, pageCount, lenient);
	finalize(ctx, nfcdev);

	return ret;
}
