#ifndef _SSP_H_
#define _SSP_H_

#define SD_DEV LPC_SSP1
#define SD_CS_PIN 11

#define SDCMD_GO_IDLE_STATE		0 // Software reset.
#define SDCMD_SEND_OP_COND		1 // Initiate initialization process.
#define SDCMD_APP_SEND_OP_COND	41 // For only SDC. Initiate initialization process.
#define SDCMD_SEND_IF_COND		8 // For only SDC V2. Check voltage range.
#define SDCMD_SEND_CSD			9 // Read CSD register.
#define SDCMD_SEND_CID 			10 // Read CID register.
#define SDCMD_STOP_TRANSMISSION	12 // Stop to read data.
#define SDCMD_SET_BLOCKLEN		16 // Change R/W block size.
#define SDCMD_READ_SINGLE_BLOCK	17
#define SDCMD_READ_MULTIPLE_BLOCK 18
#define SDCMD_SET_BLOCK_COUNT 	23 // For only MMC. Define number of blocks to transfer with next multi-block read/write command.
#define SDCMD_APP_SET_WR_BLOCK_ERASE_COUNT 23 // For only SDC. Define number of blocks to pre-erase with next multi-block write command.
#define SDCMD_WRITE_BLOCK		24
#define SDCMD_WRITE_MULTIPLE_BLOCK 25
#define SDCMD_APP_CMD			55 // Leading command of APP_* commands.
#define SDCMD_READ_OCR			58

void ssp_init(void);
void sd_init(void);

#endif
