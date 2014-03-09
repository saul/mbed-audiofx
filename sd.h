#ifndef _SD_H_
#define _SD_H_

#include <stdint.h>
#include <stdbool.h>
#include "fatfs/ff.h"


// Externs
// ----------------------------------------------------------------------------
extern FATFS g_fs;
extern uint8_t g_fSDStatus;


// Constants
// ----------------------------------------------------------------------------
#define SD_BLOCK_SIZE 512

#define SD_STATUS_READY		(1<<0)
#define SD_STATUS_SDV2		(1<<1)
#define SD_STATUS_BLOCKADDR	(1<<2)

#define SD_TIMEOUT_INDEFINITE	0


// Commands
// ----------------------------------------------------------------------------
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


// Responses
// ----------------------------------------------------------------------------
typedef enum
{
	SD_RESP_NONE = 0,
	SD_RESP_R1,
	SD_RESP_R3,
	SD_RESP_R7,
} SDResponseType_e;

// R1
// ==============================================
typedef enum
{
	SD_R1_IN_IDLE_STATE = (1<<0),
	SD_R1_ERASE_RESET = (1<<1),
	SD_R1_ILLEGAL_COMMAND = (1<<2),
	SD_R1_CRC_ERROR = (1<<3),
	SD_R1_ERASE_SEQ_ERROR = (1<<4),
	SD_R1_ADDRESS_ERROR = (1<<5),
	SD_R1_PARAM_ERROR = (1<<6),
} SDR1ResponseFlag_e;

// R3/R7
// ==============================================
typedef struct
{
	uint8_t result;
	uint32_t extra;
} SDR3Response_t;
typedef SDR3Response_t SDR7Response_t; // same format


// Function declarations
// ----------------------------------------------------------------------------
void sd_init(void);
void sd_cs(bool high);
void sd_send_command(uint8_t index, uint32_t argument);
bool sd_command(uint8_t index, uint32_t argument, SDResponseType_e respType, void *pRespData, uint32_t ulTimeoutMsec);
void fs_init(void);

#endif
