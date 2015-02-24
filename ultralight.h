
#ifndef HAVE_TYPE_H
#define HAVE_TYPE_H

#include <stdbool.h>
#include <nfc/nfc.h>

typedef enum {
	UL_OK = 0,
	UL_ERROR = 1,
	UL_UNSUPPORTED = 2,
	UL_NOTAG = 3,
	UL_NOTULTRALIGHT = 4,
	UL_NEEDSPASS = 5,
	UL_WRONGPASS = 6
} ul_result;

#define UL_WRPAGS 1
#define UL_RDPAGS 4

#define UL_SIGSIZE 32

typedef struct {
	const char * name;
	unsigned int pages;
	unsigned int password_pages;
} ul_type;

typedef struct {
	nfc_device * nfc;
	ul_type const * type;
	size_t id_size;
	uint8_t id[16];
	bool has_key;
	uint32_t key;
} ul_device;

typedef struct {
	ul_type * product;
	uint32_t * contents;
	uint32_t password;
} ul_dump;

ul_result ul_detect(nfc_device * nfcdev, ul_device * dev);
ul_result ul_select(ul_device * dev);
ul_result ul_write(ul_device * dev, unsigned int page, uint32_t * data);
ul_result ul_read(ul_device * dev, unsigned int page, uint32_t * data);
ul_result ul_read_signature(ul_device * dev, uint8_t * data);

#endif
