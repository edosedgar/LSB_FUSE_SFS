#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "jstegdev.h"

int main() {
/*
	char filename[] = "home.jpg";
	FILE* infile;
	jdev_entry* entry;

	if ((infile = fopen(filename, "r+b")) == NULL) {
      		fprintf(stderr, "can't open %s\n", filename);
      		return 0;
    	}

    	uint16_t jindex = 100;

   	entry = jentry_init(infile, &jindex);
    	printf("SIZE=%d\n", sizeof(filename));
   	memcpy(entry->data, filename, sizeof(filename));
    	entry->write_data(entry);
    	jentry_destroy(entry);


	if ((infile = fopen(filename, "r+b")) == NULL) {
      		fprintf(stderr, "can't open %s\n", filename);
      		return 0;
    	}    	
    	jindex = 255;
    	jdev_entry* entry2 = jentry_init(infile, &jindex);
    	entry2->read_data(entry2);
    	printf("%s\n", (char*) entry2->data);

    	printf("jindex2 == %d\n", entry2->jindex);

	FILE* pict = NULL; 
	pict = fopen("pictures/BB_FinalEps.jpg", "r+b");
	if (pict != NULL) fprintf(stderr, "open\n");

	filedev_data filedev;

	filedev.dirname = "pictures";
	(void) jstegdev_init(&filedev);

	for (int i = 0; i < filedev.jfile_num; i++) {
		memset(filedev.entries[i].data, 0XFF, filedev.entries[i].bytes);
		filedev.entries[i].write_data(&filedev.entries[i]);
		fprintf(stderr, "jindex=%d start=%d\n", filedev.entries[i].jindex, filedev.entries[i].start);
	}

	filedev_read();
*/
	#define FDEV ((filedev_data*) bdev->dev_data)

	size_t block_size = 255;

	filedev_data fdev;
	fdev.dirname = "pictures";
	blockdev* bdev = (blockdev*) jstegdev_create(bdev, &fdev, block_size);
	bdev->init(bdev);

	fprintf(stderr, "jfile_num %d\n", FDEV->jfile_num);
	fprintf(stderr, "start %d\n", FDEV->entries[0].start);
	fprintf(stderr, "bytes %d\n", FDEV->entries[0].bytes);
	fprintf(stderr, "start %d\n", FDEV->entries[1].start);
	fprintf(stderr, "bytes %d\n", FDEV->entries[1].bytes);

	//bdev = jstegdev_create(bdev, fdev, block_size);

	uint8_t in_buf[] = "Hello, world!";
	uint8_t out_buf[20];

	bdev->write(bdev, in_buf, 14, 20);
	printf("%s\n", (char*) FDEV->entries[0].data);


	filedev_dump(bdev, block_size);

	bdev->read(bdev, out_buf, 14, 20	);

	printf("READ %s\n", out_buf);


	return 0;
}