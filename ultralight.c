
#include <stdio.h>
#include <string.h>
#include "structs.h"
#include "ultralight.h"

#define array_size(x) (sizeof(x) / sizeof(x[0]))

static const nfc_modulation MIFARE_MODULATION = {
	.nmt = NMT_ISO14443A,
	.nbr = NBR_106
};

typedef struct {
	version_response response;
	ul_type type;
} response_and_type;

static const ul_type PLAIN_ULTRALIGHT = { "Ultralight", 16, 0 };
static const ul_type UNKNOWN_ULTRALIGHT = { "Unknown version", 0, 0 };

static const response_and_type KNOWN_ULTRALIGHTS_VERSIONS[] = {
	{ { 0x00, 0x04, 0x04, 0x02, 0x01, 0x00, 0x0F, 0x03 }, { "NTAG213",  45, 2 } },
	{ { 0x00, 0x04, 0x04, 0x02, 0x01, 0x00, 0x11, 0x03 }, { "NTAG215", 135, 2 } },
	{ { 0x00, 0x04, 0x04, 0x02, 0x01, 0x00, 0x13, 0x03 }, { "NTAG216", 231, 2 } }
};

ul_result identify(ul_device * dev);

ul_result ul_detect(nfc_device * nfcdev, ul_device * dev) {
	nfc_target target;

	if (nfc_initiator_select_passive_target(nfcdev, MIFARE_MODULATION, NULL, 0, &target) <= 0) {
		return UL_NOTAG;
	}

	if (target.nti.nai.abtAtqa[0] != 0x00 || target.nti.nai.abtAtqa[1] != 0x44) {
		return UL_NOTULTRALIGHT;
	}

	size_t idSize = target.nti.nai.szUidLen;
	memcpy(dev->id, &target.nti.nai.abtUid, idSize);
	dev->idSize = idSize;
	dev->nfc = nfcdev;
	dev->hasKey = false;

	return identify(dev);
}

ul_result ul_select(ul_device * dev) {
	nfc_target target;

	if (nfc_initiator_select_passive_target(dev->nfc, MIFARE_MODULATION, dev->id, dev->idSize, &target) <= 0) {
		return UL_NOTAG;
	}

	return UL_OK;
}

ul_result set_easy_framing(ul_device * dev, bool enable) {
	if (nfc_device_set_property_bool(dev->nfc, NP_EASY_FRAMING, enable) < 0) {
		nfc_perror(dev->nfc, "nfc_device_set_property_bool");
		return UL_ERROR;
	}

	return UL_OK;
}

ul_result transceive_data(ul_device * dev, const void * req, size_t reqlen, void * resp, size_t resplen) {
	ul_result ret;

	ret = set_easy_framing(dev, true);

	if (ret == UL_OK) {
		if (nfc_initiator_transceive_bytes(dev->nfc, req, reqlen, resp, resplen, -1) < 0) {
			nfc_perror(dev->nfc, "nfc_initiator_transceive_bytes");
			ret = UL_ERROR;
		}
	}

	return ret;
}

ul_result ul_write(ul_device * dev, unsigned int page, const ul_page * data) {
	write_request req;

	req.command = CMD_WRITE;
	req.address = page;
	memcpy(req.data, data, UL_PAGSIZE * UL_WRPAGS);

	return transceive_data(dev, &req, sizeof(req), NULL, 0);
}

ul_result ul_read(ul_device * dev, unsigned int page, ul_page * data) {
	read_request req;

	req.command = CMD_READ;
	req.address = page;

	return transceive_data(dev, &req, sizeof(req), data, UL_PAGSIZE * UL_RDPAGS);
}

ul_result transceive_extended(ul_device * dev, const void * req, size_t reqlen, void * resp, size_t resplen) {
	ul_result ret;

	ret = set_easy_framing(dev, false);
	if (ret) {
		return ret;
	}

	int code = nfc_initiator_transceive_bytes(dev->nfc, req, reqlen, resp, resplen, -1);
	if (code < 0) {
		if (code == NFC_ERFTRANS) {
			ret = ul_select(dev);
			if (ret) {
				return ret;
			}

			return UL_UNSUPPORTED;
		}

		nfc_perror(dev->nfc, "nfc_initiator_transceive_bytes");
		ret = UL_ERROR;
	}

	return UL_OK;
}

ul_result ul_read_signature(ul_device * dev, uint8_t * signature) {
	signature_request req;

	req.command = CMD_READ_SIG;
	req.address = 0;

	return transceive_extended(dev, &req, sizeof(req), signature, UL_SIGSIZE);
}

ul_result ul_authenticate(ul_device * dev, const ul_page key, ul_passack pack) {
	auth_request req;

	req.command = CMD_PWD_AUTH;
	memcpy(req.key, key, UL_PAGSIZE);

	ul_result ret = transceive_extended(dev, &req, sizeof(req), pack, UL_PACKSIZE);
	if (ret) {
		ul_select(dev);
	}

	return ret;
}

ul_result ul_set_key(ul_device * dev, const ul_page key) {
	dev->hasKey = true;
	dev->key = true;

	return UL_OK;
}

ul_result identify(ul_device * dev) {
	version_request req;
	version_response resp;
	unsigned int i;

	req.command = CMD_GET_VERSION;

	ul_result ret = transceive_extended(dev, &req, sizeof(req), &resp, sizeof(resp));
	if (ret) {
		if (ret == UL_UNSUPPORTED) {
			dev->type = &PLAIN_ULTRALIGHT;
			return UL_OK;
		}

		return ret;
	}

	for (i = 0; i < array_size(KNOWN_ULTRALIGHTS_VERSIONS); i++) {
		if (memcmp(&KNOWN_ULTRALIGHTS_VERSIONS[i].response, &resp, sizeof(resp)) == 0) {
			dev->type = &KNOWN_ULTRALIGHTS_VERSIONS[i].type;
			return UL_OK;
		}
	}

	dev->type = &UNKNOWN_ULTRALIGHT;
	return UL_OK;
}
