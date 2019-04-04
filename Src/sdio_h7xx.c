/*
    Copyright (c) 2019 Chris Hockuba (https://github.com/conkerkh)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

/* Include(s) -------------------------------------------------------------------------------------------------------*/

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define USE_SDCARD_SDIO
#ifdef USE_SDCARD_SDIO

#include "sdmmc_sdio.h"
#include "io.h"


/* Define(s) --------------------------------------------------------------------------------------------------------*/
#define BLOCK_SIZE                      ((uint32_t)(512))

#define SDMMC_ICR_STATIC_FLAGS          ((uint32_t)(SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_CTIMEOUT |\
                                                         SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_TXUNDERR | SDMMC_FLAG_RXOVERR  |\
                                                         SDMMC_FLAG_CMDREND  | SDMMC_FLAG_CMDSENT  | SDMMC_FLAG_DATAEND  |\
                                                         SDMMC_FLAG_DHOLD      | SDMMC_FLAG_DBCKEND  | SDMMC_FLAG_DABORT   |\
                                                         SDMMC_FLAG_BUSYD0END  | SDMMC_FLAG_SDIOIT   | SDMMC_FLAG_ACKFAIL  |\
                                                         SDMMC_FLAG_ACKTIMEOUT | SDMMC_FLAG_VSWEND   | SDMMC_FLAG_CKSTOP   |\
                                                         SDMMC_FLAG_IDMATE     | SDMMC_FLAG_IDMABTC))

#define SD_SOFTWARE_COMMAND_TIMEOUT     ((uint32_t)0xF0000000)

#define SD_OCR_ADDR_OUT_OF_RANGE        ((uint32_t)0x80000000)
#define SD_OCR_ADDR_MISALIGNED          ((uint32_t)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR            ((uint32_t)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR            ((uint32_t)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM          ((uint32_t)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION     ((uint32_t)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED       ((uint32_t)0x01000000)
#define SD_OCR_COM_CRC_FAILED           ((uint32_t)0x00800000)
#define SD_OCR_ILLEGAL_CMD              ((uint32_t)0x00400000)
#define SD_OCR_CARD_ECC_FAILED          ((uint32_t)0x00200000)
#define SD_OCR_CC_ERROR                 ((uint32_t)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR    ((uint32_t)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN     ((uint32_t)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN     ((uint32_t)0x00020000)
#define SD_OCR_CID_CSD_OVERWRITE        ((uint32_t)0x00010000)
#define SD_OCR_WP_ERASE_SKIP            ((uint32_t)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED        ((uint32_t)0x00004000)
#define SD_OCR_ERASE_RESET              ((uint32_t)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR            ((uint32_t)0x00000008)
#define SD_OCR_ERRORBITS                ((uint32_t)0xFDFFE008)

#define SD_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((uint32_t)0x00008000)

#define SD_VOLTAGE_WINDOW_SD            ((uint32_t)0x80100000)
#define SD_RESP_HIGH_CAPACITY           ((uint32_t)0x40000000)
#define SD_RESP_STD_CAPACITY            ((uint32_t)0x00000000)
#define SD_CHECK_PATTERN                ((uint32_t)0x000001AA)

#define SD_MAX_VOLT_TRIAL               ((uint32_t)0x0000FFFF)
#define SD_ALLZERO                      ((uint32_t)0x00000000)

#define SD_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000)
#define SD_CARD_LOCKED                  ((uint32_t)0x02000000)

#define SD_0TO7BITS                     ((uint32_t)0x000000FF)
#define SD_8TO15BITS                    ((uint32_t)0x0000FF00)
#define SD_16TO23BITS                   ((uint32_t)0x00FF0000)
#define SD_24TO31BITS                   ((uint32_t)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((uint32_t)0x01FFFFFF)

#define SD_CCCC_ERASE                   ((uint32_t)0x00000020)

#define SD_SDMMC_SEND_IF_COND           ((uint32_t)SD_CMD_HS_SEND_EXT_CSD)

#define SD_BUS_WIDE_1B                  ((uint32_t)0x00000000)
#define SD_BUS_WIDE_4B                  SDMMC_CLKCR_WIDBUS_0
#define SD_BUS_WIDE_8B                  SDMMC_CLKCR_WIDBUS_1

#define SD_CMD_RESPONSE_SHORT           SDMMC_CMD_WAITRESP_0
#define SD_CMD_RESPONSE_LONG            SDMMC_CMD_WAITRESP

#define SD_DATABLOCK_SIZE_8B            (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_1)
#define SD_DATABLOCK_SIZE_64B           (SDMMC_DCTRL_DBLOCKSIZE_1|SDMMC_DCTRL_DBLOCKSIZE_2)
#define SD_DATABLOCK_SIZE_512B          (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_3)

#define CLKCR_CLEAR_MASK                ((uint32_t)(SDMMC_CLKCR_CLKDIV  | SDMMC_CLKCR_PWRSAV |\
                                                    SDMMC_CLKCR_BUSSPEED | SDMMC_CLKCR_WIDBUS |\
                                                    SDMMC_CLKCR_NEGEDGE | SDMMC_CLKCR_HWFC_EN))

#define DCTRL_CLEAR_MASK                ((uint32_t)(SDMMC_DCTRL_DTEN    | SDMMC_DCTRL_DTDIR |\
                                                    SDMMC_DCTRL_DTMODE  | SDMMC_DCTRL_DBLOCKSIZE))

#define CMD_CLEAR_MASK                  ((uint32_t)(SDMMC_CMD_CMDINDEX | SDMMC_CMD_WAITRESP |\
                                                    SDMMC_CMD_WAITINT  | SDMMC_CMD_WAITPEND |\
                                                    SDMMC_CMD_CPSMEN   | SDMMC_CMD_sdmmc_instanceSUSPEND))

#define SDMMC_INIT_CLK_DIV              ((uint8_t)0xFA)
#define SDMMC_CLK_DIV                   ((uint8_t)0x04)


#define SD_CMD_GO_IDLE_STATE            ((uint8_t)0)   // Resets the SD memory card.
#define SD_CMD_SEND_OP_COND             ((uint8_t)1)   // Sends host capacity support information and activates the card's initialization process.
#define SD_CMD_ALL_SEND_CID             ((uint8_t)2)   // Asks any card connected to the host to send the CID numbers on the CMD line.
#define SD_CMD_SET_REL_ADDR             ((uint8_t)3)   // Asks the card to publish a new relative address (RCA).
#define SD_CMD_HS_SWITCH                ((uint8_t)6)   // Checks switchable function (mode 0) and switch card function (mode 1).
#define SD_CMD_SEL_DESEL_CARD           ((uint8_t)7)   // Selects the card by its own relative address and gets deselected by any other address
#define SD_CMD_HS_SEND_EXT_CSD          ((uint8_t)8)   // Sends SD Memory Card interface condition, which includes host supply voltage information
                                                       // and asks the card whether card supports voltage.
#define SD_CMD_SEND_CSD                 ((uint8_t)9)   // Addressed card sends its card specific data (CSD) on the CMD line.
#define SD_CMD_SEND_CID                 ((uint8_t)10)  // Addressed card sends its card identification (CID) on the CMD line.
#define SD_CMD_STOP_TRANSMISSION        ((uint8_t)12)  // Forces the card to stop transmission.
#define SD_CMD_SEND_STATUS              ((uint8_t)13)  // Addressed card sends its status register.
#define SD_CMD_SET_BLOCKLEN             ((uint8_t)16)  // Sets the block length (in bytes for SDSC) for all following block commands
                                                       // (read, write, lock). Default block length is fixed to 512 Bytes. Not effective
                                                       // for SDHS and SDXC.
#define SD_CMD_READ_SINGLE_BLOCK        ((uint8_t)17)  // Reads single block of size selected by SET_BLOCKLEN in case of SDSC, and a block of
                                                       // fixed 512 bytes in case of SDHC and SDXC.
#define SD_CMD_READ_MULT_BLOCK          ((uint8_t)18)  // Continuously transfers data blocks from card to host until interrupted by
                                                       // STOP_TRANSMISSION command.
#define SD_CMD_WRITE_SINGLE_BLOCK       ((uint8_t)24)  // Writes single block of size selected by SET_BLOCKLEN in case of SDSC, and a block of
                                                       // fixed 512 bytes in case of SDHC and SDXC.
#define SD_CMD_WRITE_MULT_BLOCK         ((uint8_t)25)  // Continuously writes blocks of data until a STOP_TRANSMISSION follows.
#define SD_CMD_SD_ERASE_GRP_START       ((uint8_t)32)  // Sets the address of the first write block to be erased. (For SD card only).
#define SD_CMD_SD_ERASE_GRP_END         ((uint8_t)33)  // Sets the address of the last write block of the continuous range to be erased.
                                                       // system set by switch function command (CMD6).
#define SD_CMD_ERASE                    ((uint8_t)38)  // Reserved for SD security applications.
#define SD_CMD_FAST_IO                  ((uint8_t)39)  // SD card doesn't support it (Reserved).
#define SD_CMD_APP_CMD                  ((uint8_t)55)  // Indicates to the card that the next command is an application specific command rather
                                                       // than a standard command.

/* Following commands are SD Card Specific commands.
   SDMMC_APP_CMD should be sent before sending these commands. */
#define SD_CMD_APP_SD_SET_BUSWIDTH      ((uint8_t)6)   // (ACMD6) Defines the data bus width to be used for data transfer. The allowed data bus
                                                       // widths are given in SCR register.
#define SD_CMD_SD_APP_STATUS            ((uint8_t)13)  // (ACMD13) Sends the SD status.
#define SD_CMD_SD_APP_OP_COND           ((uint8_t)41)  // (ACMD41) Sends host capacity support information (HCS) and asks the accessed card to
                                                       // send its operating condition register (OCR) content in the response on the CMD line.
#define SD_CMD_SD_APP_SEND_SCR          ((uint8_t)51)  // Reads the SD Configuration Register (SCR).

#define SDMMC_DIR_TX 1
#define SDMMC_DIR_RX 0


/* Typedef(s) -------------------------------------------------------------------------------------------------------*/

typedef enum
{
    SD_SINGLE_BLOCK    = 0,             // Single block operation
    SD_MULTIPLE_BLOCK  = 1,             // Multiple blocks operation
} SD_Operation_t;


typedef struct
{
    uint32_t          CSD[4];           // SD card specific data table
    uint32_t          CID[4];           // SD card identification number table
    volatile uint32_t TransferComplete; // SD transfer complete flag in non blocking mode
    volatile uint32_t TransferError;    // SD transfer error flag in non blocking mode
    volatile uint32_t RXCplt;		   // SD RX Complete is equal 0 when no transfer
    volatile uint32_t TXCplt;		   // SD TX Complete is equal 0 when no transfer
    volatile uint32_t Operation;        // SD transfer operation (read/write)
} SD_Handle_t;

typedef enum
{
    SD_CARD_READY                  = ((uint32_t)0x00000001),  // Card state is ready
    SD_CARD_IDENTIFICATION         = ((uint32_t)0x00000002),  // Card is in identification state
    SD_CARD_STANDBY                = ((uint32_t)0x00000003),  // Card is in standby state
    SD_CARD_TRANSFER               = ((uint32_t)0x00000004),  // Card is in transfer state
    SD_CARD_SENDING                = ((uint32_t)0x00000005),  // Card is sending an operation
    SD_CARD_RECEIVING              = ((uint32_t)0x00000006),  // Card is receiving operation information
    SD_CARD_PROGRAMMING            = ((uint32_t)0x00000007),  // Card is in programming state
    SD_CARD_DISCONNECTED           = ((uint32_t)0x00000008),  // Card is disconnected
    SD_CARD_ERROR                  = ((uint32_t)0x000000FF)   // Card is in error state
} SD_CardState_t;

/* Variable(s) ------------------------------------------------------------------------------------------------------*/

static SD_Handle_t                 SD_Handle;
SD_CardInfo_t                      SD_CardInfo;
static uint32_t                    SD_Status;
static uint32_t                    SD_CardRCA;
SD_CardType_t                      SD_CardType;
SDMMC_TypeDef                      *sdmmc_instance;


/* Private function(s) ----------------------------------------------------------------------------------------------*/

static void             SD_DataTransferInit         (uint32_t Size, uint32_t DataBlockSize, bool IsItReadFromCard, bool enableDPSM);
static SD_Error_t       SD_TransmitCommand          (uint32_t Command, uint32_t Argument, int8_t ResponseType);
static SD_Error_t       SD_CmdResponse              (uint8_t SD_CMD, int8_t ResponseType);
static void             SD_GetResponse              (uint32_t* pResponse);
static SD_Error_t       CheckOCR_Response           (uint32_t Response_R1);
static SD_Error_t       SD_InitializeCard           (void);
static SD_Error_t       SD_Abort                    (void);
static SD_Error_t       SD_PowerON                  (void);
static SD_Error_t       SD_WideBusOperationConfig   (uint32_t WideMode);
static SD_Error_t       SD_FindSCR                  (uint32_t *pSCR);
static void             SD_EnableIDMA               (uint32_t *pBuffer);
static void             SD_StartBlockTransfer       (uint32_t BlockSize, uint32_t NumberOfBlocks, uint8_t dir);

//static void             SD_PowerOFF                 (void);

/** -----------------------------------------------------------------------------------------------------------------*/
/**		SD_IsDetected
  *
  * @brief  Test if card is present
  * @param  bool   true or false
  */
bool SD_IsDetected(void)
{
      __IO uint8_t status = SD_PRESENT;
      /*!< Check GPIO to detect SD */
    #ifdef SDCARD_DETECT_PIN
      const IO_t sd_det = IOGetByTag(IO_TAG(SDCARD_DETECT_PIN));
      if (IORead(sd_det) != 0)
      {
        status = SD_NOT_PRESENT;
      }
    #endif
      return status;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**		SD_TransmitCommand
  *
  * @brief  Send the commande to sdmmc_instance
  * @param  uint32_t Command
  * @param  uint32_t Argument              Must provide the response size
  * @param  uint8_t ResponseType
  * @retval SD Card error state
  */
static SD_Error_t SD_TransmitCommand(uint32_t Command, uint32_t Argument, int8_t ResponseType)
{
    SD_Error_t ErrorState;

    uint32_t cmd_clear_mask = ((uint32_t)(SDMMC_CMD_CMDINDEX | SDMMC_CMD_WAITRESP |\
            SDMMC_CMD_WAITINT  | SDMMC_CMD_WAITPEND |\
            SDMMC_CMD_CPSMEN   | SDMMC_CMD_CMDSUSPEND));

    WRITE_REG(sdmmc_instance->ICR, SDMMC_ICR_STATIC_FLAGS);                               // Clear the Command Flags
    WRITE_REG(sdmmc_instance->ARG, (uint32_t) Argument);                                  // Set the sdmmc_instance Argument value
    MODIFY_REG(sdmmc_instance->CMD, cmd_clear_mask,(uint32_t) (Command | SDMMC_CMD_CPSMEN));              // Set sdmmc_instance command parameters
    if((Argument == 0) && (ResponseType == 0)) ResponseType = -1;                         // Go idle command
    ErrorState  = SD_CmdResponse(Command & SDMMC_CMD_CMDINDEX, ResponseType);
    WRITE_REG(sdmmc_instance->ICR, SDMMC_ICR_STATIC_FLAGS);                               // Clear the Command Flags
    return ErrorState;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Checks for error conditions for any response.
  *                                     - R2 (CID or CSD) response.
  *                                     - R3 (OCR) response.
  *
  * @param  SD_CMD: The sent command Index
  * @retval SD Card error state
  */
static SD_Error_t SD_CmdResponse(uint8_t SD_CMD, int8_t ResponseType)
{
    uint32_t Response_R1;
    uint32_t TimeOut;
    uint32_t Flag;

    if(ResponseType == -1) Flag = SDMMC_STA_CMDSENT;
    else                   Flag = SDMMC_STA_CCRCFAIL | SDMMC_STA_CMDREND | SDMMC_STA_CTIMEOUT | SDMMC_STA_BUSYD0END;

    TimeOut = SD_SOFTWARE_COMMAND_TIMEOUT;
    do
    {
        SD_Status = sdmmc_instance->STA;
        TimeOut--;
    }
    while(((SD_Status & Flag) == 0) && (TimeOut > 0));

    if(ResponseType <= 0)
    {
        if(TimeOut == 0)                            return SD_CMD_RSP_TIMEOUT;
        else                                        return SD_OK;
    }

    if((sdmmc_instance->STA & SDMMC_STA_CTIMEOUT) != 0)     return SD_CMD_RSP_TIMEOUT;
    if(ResponseType == 3)
    {
        if(TimeOut == 0)                            return SD_CMD_RSP_TIMEOUT;  // Card is not V2.0 compliant or card does not support the set voltage range
        else                                        return SD_OK;               // Card is SD V2.0 compliant
    }

    if((sdmmc_instance->STA & SDMMC_STA_CCRCFAIL) != 0)     return SD_CMD_CRC_FAIL;
    if(ResponseType == 2)                           return SD_OK;
    if((uint8_t)sdmmc_instance->RESPCMD != SD_CMD)          return SD_ILLEGAL_CMD;      // Check if response is of desired command

    Response_R1 = sdmmc_instance->RESP1;                    // We have received response, retrieve it for analysis

    if(ResponseType == 1)
    {
        return CheckOCR_Response(Response_R1);
    }
    else if(ResponseType == 6)
    {
        if((Response_R1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)) == SD_ALLZERO)
        {
            SD_CardRCA = Response_R1;
        }
        if((Response_R1 & SD_R6_GENERAL_UNKNOWN_ERROR) == SD_R6_GENERAL_UNKNOWN_ERROR)      return SD_GENERAL_UNKNOWN_ERROR;
        if((Response_R1 & SD_R6_ILLEGAL_CMD)           == SD_R6_ILLEGAL_CMD)                return SD_ILLEGAL_CMD;
        if((Response_R1 & SD_R6_COM_CRC_FAILED)        == SD_R6_COM_CRC_FAILED)             return SD_COM_CRC_FAILED;
    }

    return SD_OK;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Analyze the OCR response and return the appropriate error code
  * @param  Response_R1: OCR Response code
  * @retval SD Card error state
  */
static SD_Error_t CheckOCR_Response(uint32_t Response_R1)
{
    if((Response_R1 & SD_OCR_ERRORBITS)             == SD_ALLZERO)                  return SD_OK;
    if((Response_R1 & SD_OCR_ADDR_OUT_OF_RANGE)     == SD_OCR_ADDR_OUT_OF_RANGE)    return SD_ADDR_OUT_OF_RANGE;
    if((Response_R1 & SD_OCR_ADDR_MISALIGNED)       == SD_OCR_ADDR_MISALIGNED)      return SD_ADDR_MISALIGNED;
    if((Response_R1 & SD_OCR_BLOCK_LEN_ERR)         == SD_OCR_BLOCK_LEN_ERR)        return SD_BLOCK_LEN_ERR;
    if((Response_R1 & SD_OCR_ERASE_SEQ_ERR)         == SD_OCR_ERASE_SEQ_ERR)        return SD_ERASE_SEQ_ERR;
    if((Response_R1 & SD_OCR_BAD_ERASE_PARAM)       == SD_OCR_BAD_ERASE_PARAM)      return SD_BAD_ERASE_PARAM;
    if((Response_R1 & SD_OCR_WRITE_PROT_VIOLATION)  == SD_OCR_WRITE_PROT_VIOLATION) return SD_WRITE_PROT_VIOLATION;
    if((Response_R1 & SD_OCR_LOCK_UNLOCK_FAILED)    == SD_OCR_LOCK_UNLOCK_FAILED)   return SD_LOCK_UNLOCK_FAILED;
    if((Response_R1 & SD_OCR_COM_CRC_FAILED)        == SD_OCR_COM_CRC_FAILED)       return SD_COM_CRC_FAILED;
    if((Response_R1 & SD_OCR_ILLEGAL_CMD)           == SD_OCR_ILLEGAL_CMD)          return SD_ILLEGAL_CMD;
    if((Response_R1 & SD_OCR_CARD_ECC_FAILED)       == SD_OCR_CARD_ECC_FAILED)      return SD_CARD_ECC_FAILED;
    if((Response_R1 & SD_OCR_CC_ERROR)              == SD_OCR_CC_ERROR)             return SD_CC_ERROR;
    if((Response_R1 & SD_OCR_GENERAL_UNKNOWN_ERROR) == SD_OCR_GENERAL_UNKNOWN_ERROR)return SD_GENERAL_UNKNOWN_ERROR;
    if((Response_R1 & SD_OCR_STREAM_READ_UNDERRUN)  == SD_OCR_STREAM_READ_UNDERRUN) return SD_STREAM_READ_UNDERRUN;
    if((Response_R1 & SD_OCR_STREAM_WRITE_OVERRUN)  == SD_OCR_STREAM_WRITE_OVERRUN) return SD_STREAM_WRITE_OVERRUN;
    if((Response_R1 & SD_OCR_CID_CSD_OVERWRITE)     == SD_OCR_CID_CSD_OVERWRITE)    return SD_CID_CSD_OVERWRITE;
    if((Response_R1 & SD_OCR_WP_ERASE_SKIP)         == SD_OCR_WP_ERASE_SKIP)        return SD_WP_ERASE_SKIP;
    if((Response_R1 & SD_OCR_CARD_ECC_DISABLED)     == SD_OCR_CARD_ECC_DISABLED)    return SD_CARD_ECC_DISABLED;
    if((Response_R1 & SD_OCR_ERASE_RESET)           == SD_OCR_ERASE_RESET)          return SD_ERASE_RESET;
    if((Response_R1 & SD_OCR_AKE_SEQ_ERROR)         == SD_OCR_AKE_SEQ_ERROR)        return SD_AKE_SEQ_ERROR;

    return SD_OK;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**		GetResponse
  *
  * @brief  Get response from SD device
  * @param  uint32_t*       pResponse
  */
static void SD_GetResponse(uint32_t* pResponse)
{
    pResponse[0] = sdmmc_instance->RESP1;
    pResponse[1] = sdmmc_instance->RESP2;
    pResponse[2] = sdmmc_instance->RESP3;
    pResponse[3] = sdmmc_instance->RESP4;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Initializes all cards or single card as the case may be Card(s) come
  *         into standby state.
  * @retval SD Card error state
  */
static SD_Error_t SD_InitializeCard(void)
{
    SD_Error_t ErrorState = SD_OK;

    if((sdmmc_instance->POWER & SDMMC_POWER_PWRCTRL) != 0) // Power off
    {
        if(SD_CardType != SD_SECURE_DIGITAL_IO)
        {
            // Send CMD2 ALL_SEND_CID
            if((ErrorState = SD_TransmitCommand((SD_CMD_ALL_SEND_CID | SD_CMD_RESPONSE_LONG), 0, 2)) != SD_OK)
            {
                return ErrorState;
            }

            // Get Card identification number data
            SD_GetResponse(SD_Handle.CID);
        }

        if((SD_CardType == SD_STD_CAPACITY_V1_1)    || (SD_CardType == SD_STD_CAPACITY_V2_0) ||
           (SD_CardType == SD_SECURE_DIGITAL_IO_COMBO) || (SD_CardType == SD_HIGH_CAPACITY))
        {
            // Send CMD3 SET_REL_ADDR with argument 0
            // SD Card publishes its RCA.
            if((ErrorState = SD_TransmitCommand((SD_CMD_SET_REL_ADDR | SD_CMD_RESPONSE_SHORT), 0, 6)) != SD_OK)
            {
                return ErrorState;
            }
        }

        if(SD_CardType != SD_SECURE_DIGITAL_IO)
        {
            // Send CMD9 SEND_CSD with argument as card's RCA
            if((ErrorState = SD_TransmitCommand((SD_CMD_SEND_CSD | SD_CMD_RESPONSE_LONG), SD_CardRCA, 2)) == SD_OK)
            {
                // Get Card Specific Data
                SD_GetResponse(SD_Handle.CSD);
            }
        }
    }
    else
    {
        ErrorState = SD_REQUEST_NOT_APPLICABLE;
    }

    return ErrorState;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Prepre the DMA transfer
  * @param  pDMA:         DMA Stream to use for the DMA operation
  * @param  pBuffer:      Pointer to the buffer that will contain the data to transmit
  * @param  BlockSize:    The SD card Data block size
  * @note   BlockSize must be 512 bytes.
  * @param  NumberOfBlocks: Number of blocks to write
  * @retval SD Card error state
  */
static void SD_StartBlockTransfer(uint32_t BlockSize, uint32_t NumberOfBlocks, uint8_t dir)
{
    sdmmc_instance->DCTRL      = 0;                                                                 // Initialize data control register
    SD_Handle.TransferComplete = 0;                                                                 // Initialize handle flags
    SD_Handle.TransferError    = SD_OK;
    SD_Handle.Operation        = (NumberOfBlocks > 1) ? SD_MULTIPLE_BLOCK : SD_SINGLE_BLOCK;        // Initialize SD Read operation
    SD_Handle.Operation       |= dir << 1;
    sdmmc_instance->MASK       = 0;
    sdmmc_instance->DTIMER = SD_DATATIMEOUT;                                                        // Set the sdmmc_instance Data TimeOut value
    sdmmc_instance->DCTRL |= SD_DATABLOCK_SIZE_512B;
    sdmmc_instance->DLEN       = NumberOfBlocks * BlockSize;                                        // Set the sdmmc_instance DataLength value
    sdmmc_instance->ICR       |= SDMMC_ICR_IDMABTCC | SDMMC_ICR_IDMATEC;

    if (dir == SDMMC_DIR_RX) {
        sdmmc_instance->DCTRL |= SDMMC_DCTRL_DTDIR;
        sdmmc_instance->MASK            |= (SDMMC_MASK_DCRCFAILIE | SDMMC_MASK_DTIMEOUTIE |         // Enable transfer interrupts
                                      SDMMC_MASK_DATAENDIE  | SDMMC_MASK_RXOVERRIE);
    } else {
        sdmmc_instance->DCTRL &= ~(SDMMC_DCTRL_DTDIR);
        sdmmc_instance->MASK            |= (SDMMC_MASK_DCRCFAILIE | SDMMC_MASK_DTIMEOUTIE |         // Enable transfer interrupts
                                      SDMMC_MASK_TXUNDERRIE | SDMMC_MASK_DATAENDIE);
    }
}

static void SD_EnableIDMA(uint32_t *pBuffer)
{
    sdmmc_instance->IDMACTRL  |= SDMMC_IDMA_IDMAEN;                                                 // Enable sdmmc_instance DMA transfer
    sdmmc_instance->IDMABASE0  = (uint32_t) pBuffer;                                                // Configure DMA Stream memory address
}

/** -----------------------------------------------------------------------------------------------------------------*/
/**     DataTransferInit
  *
  * @brief  Prepare the state machine for transfer
  * @param  SD_TransferType_e   TransfertDir
  * @param  SD_CARD_BlockSize_e Size
  */
static void SD_DataTransferInit(uint32_t Size, uint32_t DataBlockSize, bool IsItReadFromCard, bool enableDPSM)
{
    uint32_t Direction;

    sdmmc_instance->DTIMER = SD_DATATIMEOUT;        // Set the sdmmc_instance Data TimeOut value
    sdmmc_instance->DLEN   = Size;                  // Set the sdmmc_instance DataLength value
    Direction      = (IsItReadFromCard == true) ? SDMMC_DCTRL_DTDIR : 0;
    sdmmc_instance->DCTRL |= DataBlockSize;
    sdmmc_instance->DCTRL |=  (uint32_t)(Direction | enableDPSM);
    return;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Reads block(s) from a specified address in a card. The Data transfer
  *         is managed by DMA mode.
  * @note   This API should be followed by the function SD_CheckOperation()
  *         to check the completion of the read process
  * @param  pReadBuffer: Pointer to the buffer that will contain the received data
  * @param  ReadAddr: Address from where data is to be read
  * @param  BlockSize: SD card Data block size
  * @note   BlockSize must be 512 bytes.
  * @param  NumberOfBlocks: Number of blocks to read.
  * @retval SD Card error state
  */
SD_Error_t SD_ReadBlocks_DMA(uint64_t ReadAddress, uint32_t *buffer, uint32_t BlockSize, uint32_t NumberOfBlocks)
{
    SD_Error_t ErrorState;
    uint32_t   CmdIndex;
    SD_Handle.RXCplt = 1;

    assert_param((buffer >= 0x24000000) && (buffer <= (0x24080000)));

    //printf("Reading at %ld into %p %ld blocks\n", (uint32_t)ReadAddress, (void*)buffer, NumberOfBlocks);

    if(SD_CardType != SD_HIGH_CAPACITY)
    {
        ReadAddress *= 512;
    }

    // Set Block Size for Card
    ErrorState = SD_TransmitCommand((SD_CMD_SET_BLOCKLEN | SD_CMD_RESPONSE_SHORT), BlockSize, 1);

    // Configure the SD DPSM (Data Path State Machine)
    SD_StartBlockTransfer(BlockSize, NumberOfBlocks, SDMMC_DIR_RX);

    // Enable transfer
    sdmmc_instance->CMD |= SDMMC_CMD_CMDTRANS;

    // Enable IDMA
    SD_EnableIDMA(buffer);

    // Send CMD18 READ_MULT_BLOCK with argument data address
    // or send CMD17 READ_SINGLE_BLOCK depending on number of block
    uint8_t retries = 10;
    CmdIndex   = (NumberOfBlocks > 1) ? SD_CMD_READ_MULT_BLOCK : SD_CMD_READ_SINGLE_BLOCK;
    do {
            ErrorState = SD_TransmitCommand((CmdIndex | SD_CMD_RESPONSE_SHORT), (uint32_t)ReadAddress, 1);
            if (ErrorState != SD_OK && retries--) {
                ErrorState = SD_TransmitCommand((SD_CMD_APP_CMD | SD_CMD_RESPONSE_SHORT), 0, 1);
            }
    } while (ErrorState != SD_OK && retries);

    if (ErrorState != SD_OK) {
            SD_Handle.RXCplt = 0;
    }

    // Update the SD transfer error in SD handle
    SD_Handle.TransferError = ErrorState;

    return ErrorState;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Writes block(s) to a specified address in a card. The Data transfer
  *         is managed by DMA mode.
  * @note   This API should be followed by the function SD_CheckOperation()
  *         to check the completion of the write process (by SD current status polling).
  * @param  pWriteBuffer: pointer to the buffer that will contain the data to transmit
  * @param  WriteAddress: Address from where data is to be read
  * @param  BlockSize: the SD card Data block size
  * @note   BlockSize must be 512 bytes.
  * @param  NumberOfBlocks: Number of blocks to write
  * @retval SD Card error state
  */
SD_Error_t SD_WriteBlocks_DMA(uint64_t WriteAddress, uint32_t *buffer, uint32_t BlockSize, uint32_t NumberOfBlocks)
{
    SD_Error_t ErrorState;
    uint32_t   CmdIndex;
    SD_Handle.TXCplt = 1;

    assert_param((buffer >= 0x24000000) && (buffer <= (0x24080000)));

    if(SD_CardType != SD_HIGH_CAPACITY)
    {
        WriteAddress *= 512;
    }

    // Set Block Size for Card
    ErrorState = SD_TransmitCommand((SD_CMD_SET_BLOCKLEN | SD_CMD_RESPONSE_SHORT), BlockSize, 1);

    // Set transfer
    SD_StartBlockTransfer(BlockSize, NumberOfBlocks, SDMMC_DIR_TX);

    // Check number of blocks command
    // Send CMD24 WRITE_SINGLE_BLOCK
    // Send CMD25 WRITE_MULT_BLOCK with argument data address
    CmdIndex = (NumberOfBlocks > 1) ? SD_CMD_WRITE_MULT_BLOCK : SD_CMD_WRITE_SINGLE_BLOCK;

    // Enable transfer
    sdmmc_instance->CMD |= SDMMC_CMD_CMDTRANS;

    // Enable IDMA
    SD_EnableIDMA(buffer);

#ifdef SDMMC_CACHE_MAINTANANCE
    // Invalidate cache
    if (SCB->CCR & SCB_CCR_DC_Msk) {
        uint32_t alignedAddr = (uint32_t)buffer & ~0x1F;
        SCB_CleanDCache_by_Addr((uint32_t*)alignedAddr, NumberOfBlocks * BLOCKSIZE + ((uint32_t)buffer - alignedAddr));
    }
#endif

    // Set Block Size for Card
    uint8_t retries = 10;
    do {
            ErrorState = SD_TransmitCommand((CmdIndex | SD_CMD_RESPONSE_SHORT), (uint32_t)WriteAddress, 1);
            if (ErrorState != SD_OK && retries--) {
                ErrorState = SD_TransmitCommand((SD_CMD_APP_CMD | SD_CMD_RESPONSE_SHORT), 0, 1);
            }
    } while(ErrorState != SD_OK && retries);

    if (ErrorState != SD_OK) {
            SD_Handle.TXCplt = 0;
            return ErrorState;
    }

    SD_Handle.TransferError = ErrorState;

    return ErrorState;
}

SD_Error_t SD_CheckWrite(void) {
    SD_Error_t error = SD_OK;
    if (SD_Handle.TXCplt != 0) {
        error = SD_BUSY;
    }
    if (SD_Handle.TransferError) {
        printf("SD Card TXError %lu will abort...\n", SD_Handle.TransferError);
        if (SD_Abort() == SD_OK) {
            SD_Handle.TXCplt = 0;
        }
    }
    return error;
}

SD_Error_t SD_CheckRead(void) {
    SD_Error_t error = SD_OK;
    if (SD_Handle.RXCplt != 0) {
        error = SD_BUSY;
    }
    if (SD_Handle.TransferError) {
        printf("SD Card RXError %lu will abort...\n", SD_Handle.TransferError);
        if (SD_Abort() == SD_OK) {
            SD_Handle.RXCplt = 0;
        }
    }
    return error;
}

/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Erases the specified memory area of the given SD card.
  * @param  StartAddress: Start byte address
  * @param  EndAddress: End byte address
  * @retval SD Card error state
  */
/*
SD_Error_t SD_Erase(uint64_t StartAddress, uint64_t EndAddress)
{
    SD_Error_t ErrorState;
    uint32_t   Delay;
    uint32_t   MaxDelay;
    uint8_t    CardState;

    // Check if the card command class supports erase command
    if(((SD_Handle.CSD[1] >> 20) & SD_CCCC_ERASE) == 0)
    {
        return SD_REQUEST_NOT_APPLICABLE;
    }

    // Get max delay value
    MaxDelay = 120000 / (((sdmmc_instance->CLKCR) & 0xFF) + 2);

    if((sdmmc_instance->RESP1 & SD_CARD_LOCKED) == SD_CARD_LOCKED)
    {
        return SD_LOCK_UNLOCK_FAILED;
    }

    // Get start and end block for high capacity cards
    if(SD_CardType == SD_HIGH_CAPACITY)
    {
        StartAddress /= 512;
        EndAddress   /= 512;
    }

    // According to sd-card spec 1.0 ERASE_GROUP_START (CMD32) and erase_group_end(CMD33)
    if ((SD_CardType == SD_STD_CAPACITY_V1_1) || (SD_CardType == SD_STD_CAPACITY_V2_0) ||
        (SD_CardType == SD_HIGH_CAPACITY))
    {
        // Send CMD32 SD_ERASE_GRP_START with argument as addr
        if((ErrorState = SD_TransmitCommand((SD_CMD_SD_ERASE_GRP_START | SDMMC_CMD_RESPONSE_SHORT), (uint32_t)StartAddress, 1)) != SD_OK)
        {
            return ErrorState;
        }

        // Send CMD33 SD_ERASE_GRP_END with argument as addr
        if((ErrorState = SD_TransmitCommand((SD_CMD_SD_ERASE_GRP_END | SDMMC_CMD_RESPONSE_SHORT), (uint32_t)EndAddress, 1)) != SD_OK)
        {
            return ErrorState;
        }
    }

    // Send CMD38 ERASE
    if((ErrorState = SD_TransmitCommand((SD_CMD_ERASE | SDMMC_CMD_RESPONSE_SHORT), 0, 1)) != SD_OK)
    {
        return ErrorState;
    }

    for(Delay = 0; Delay < MaxDelay; Delay++);

    // Wait until the card is in programming state
    ErrorState = SD_IsCardProgramming(&CardState);

    Delay = SD_DATATIMEOUT;
    while((Delay > 0) && (ErrorState == SD_OK) && ((CardState == SD_CARD_PROGRAMMING) || (CardState == SD_CARD_RECEIVING)))
    {
        ErrorState = SD_IsCardProgramming( &CardState);
        Delay--;
    }

    return ErrorState;
}
*/


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Returns information about specific card.
  *         contains all SD cardinformation
  * @retval SD Card error state
  */
SD_Error_t SD_GetCardInfo(void)
{
    SD_Error_t ErrorState = SD_OK;
    uint32_t Temp = 0;

    // Byte 0
    Temp = (SD_Handle.CSD[0] & 0xFF000000) >> 24;
    SD_CardInfo.SD_csd.CSDStruct      = (uint8_t)((Temp & 0xC0) >> 6);
    SD_CardInfo.SD_csd.SysSpecVersion = (uint8_t)((Temp & 0x3C) >> 2);
    SD_CardInfo.SD_csd.Reserved1      = Temp & 0x03;

    // Byte 1
    Temp = (SD_Handle.CSD[0] & 0x00FF0000) >> 16;
    SD_CardInfo.SD_csd.TAAC = (uint8_t)Temp;

    // Byte 2
    Temp = (SD_Handle.CSD[0] & 0x0000FF00) >> 8;
    SD_CardInfo.SD_csd.NSAC = (uint8_t)Temp;

    // Byte 3
    Temp = SD_Handle.CSD[0] & 0x000000FF;
    SD_CardInfo.SD_csd.MaxBusClkFrec = (uint8_t)Temp;

    // Byte 4
    Temp = (SD_Handle.CSD[1] & 0xFF000000) >> 24;
    SD_CardInfo.SD_csd.CardComdClasses = (uint16_t)(Temp << 4);

    // Byte 5
    Temp = (SD_Handle.CSD[1] & 0x00FF0000) >> 16;
    SD_CardInfo.SD_csd.CardComdClasses |= (uint16_t)((Temp & 0xF0) >> 4);
    SD_CardInfo.SD_csd.RdBlockLen       = (uint8_t)(Temp & 0x0F);

    // Byte 6
    Temp = (SD_Handle.CSD[1] & 0x0000FF00) >> 8;
    SD_CardInfo.SD_csd.PartBlockRead   = (uint8_t)((Temp & 0x80) >> 7);
    SD_CardInfo.SD_csd.WrBlockMisalign = (uint8_t)((Temp & 0x40) >> 6);
    SD_CardInfo.SD_csd.RdBlockMisalign = (uint8_t)((Temp & 0x20) >> 5);
    SD_CardInfo.SD_csd.DSRImpl         = (uint8_t)((Temp & 0x10) >> 4);
    SD_CardInfo.SD_csd.Reserved2       = 0; /*!< Reserved */

    if((SD_CardType == SD_STD_CAPACITY_V1_1) || (SD_CardType == SD_STD_CAPACITY_V2_0))
    {
        SD_CardInfo.SD_csd.DeviceSize = (Temp & 0x03) << 10;

        // Byte 7
        Temp = (uint8_t)(SD_Handle.CSD[1] & 0x000000FF);
        SD_CardInfo.SD_csd.DeviceSize |= (Temp) << 2;

        // Byte 8
        Temp = (uint8_t)((SD_Handle.CSD[2] & 0xFF000000) >> 24);
        SD_CardInfo.SD_csd.DeviceSize |= (Temp & 0xC0) >> 6;

        SD_CardInfo.SD_csd.MaxRdCurrentVDDMin = (Temp & 0x38) >> 3;
        SD_CardInfo.SD_csd.MaxRdCurrentVDDMax = (Temp & 0x07);

        // Byte 9
        Temp = (uint8_t)((SD_Handle.CSD[2] & 0x00FF0000) >> 16);
        SD_CardInfo.SD_csd.MaxWrCurrentVDDMin = (Temp & 0xE0) >> 5;
        SD_CardInfo.SD_csd.MaxWrCurrentVDDMax = (Temp & 0x1C) >> 2;
        SD_CardInfo.SD_csd.DeviceSizeMul      = (Temp & 0x03) << 1;

        // Byte 10
        Temp = (uint8_t)((SD_Handle.CSD[2] & 0x0000FF00) >> 8);
        SD_CardInfo.SD_csd.DeviceSizeMul |= (Temp & 0x80) >> 7;

        SD_CardInfo.CardCapacity  = (SD_CardInfo.SD_csd.DeviceSize + 1) ;
        SD_CardInfo.CardCapacity *= (1 << (SD_CardInfo.SD_csd.DeviceSizeMul + 2));
        SD_CardInfo.CardBlockSize = 1 << (SD_CardInfo.SD_csd.RdBlockLen);
        SD_CardInfo.CardCapacity *= SD_CardInfo.CardBlockSize;
    }
    else if(SD_CardType == SD_HIGH_CAPACITY)
    {
        // Byte 7
        Temp = (uint8_t)(SD_Handle.CSD[1] & 0x000000FF);
        SD_CardInfo.SD_csd.DeviceSize = (Temp & 0x3F) << 16;

        // Byte 8
        Temp = (uint8_t)((SD_Handle.CSD[2] & 0xFF000000) >> 24);

        SD_CardInfo.SD_csd.DeviceSize |= (Temp << 8);

        // Byte 9
        Temp = (uint8_t)((SD_Handle.CSD[2] & 0x00FF0000) >> 16);

        SD_CardInfo.SD_csd.DeviceSize |= (Temp);

        // Byte 10
        Temp = (uint8_t)((SD_Handle.CSD[2] & 0x0000FF00) >> 8);

        SD_CardInfo.CardCapacity  = ((uint64_t)SD_CardInfo.SD_csd.DeviceSize + 1) * 1024;
        SD_CardInfo.CardBlockSize = 512;
    }
    else
    {
        // Not supported card type
        ErrorState = SD_ERROR;
    }

    SD_CardInfo.SD_csd.EraseGrSize = (Temp & 0x40) >> 6;
    SD_CardInfo.SD_csd.EraseGrMul  = (Temp & 0x3F) << 1;

    // Byte 11
    Temp = (uint8_t)(SD_Handle.CSD[2] & 0x000000FF);
    SD_CardInfo.SD_csd.EraseGrMul     |= (Temp & 0x80) >> 7;
    SD_CardInfo.SD_csd.WrProtectGrSize = (Temp & 0x7F);

    // Byte 12
    Temp = (uint8_t)((SD_Handle.CSD[3] & 0xFF000000) >> 24);
    SD_CardInfo.SD_csd.WrProtectGrEnable = (Temp & 0x80) >> 7;
    SD_CardInfo.SD_csd.ManDeflECC        = (Temp & 0x60) >> 5;
    SD_CardInfo.SD_csd.WrSpeedFact       = (Temp & 0x1C) >> 2;
    SD_CardInfo.SD_csd.MaxWrBlockLen     = (Temp & 0x03) << 2;

    // Byte 13
    Temp = (uint8_t)((SD_Handle.CSD[3] & 0x00FF0000) >> 16);
    SD_CardInfo.SD_csd.MaxWrBlockLen      |= (Temp & 0xC0) >> 6;
    SD_CardInfo.SD_csd.WriteBlockPaPartial = (Temp & 0x20) >> 5;
    SD_CardInfo.SD_csd.Reserved3           = 0;
    SD_CardInfo.SD_csd.ContentProtectAppli = (Temp & 0x01);

    // Byte 14
    Temp = (uint8_t)((SD_Handle.CSD[3] & 0x0000FF00) >> 8);
    SD_CardInfo.SD_csd.FileFormatGrouop = (Temp & 0x80) >> 7;
    SD_CardInfo.SD_csd.CopyFlag         = (Temp & 0x40) >> 6;
    SD_CardInfo.SD_csd.PermWrProtect    = (Temp & 0x20) >> 5;
    SD_CardInfo.SD_csd.TempWrProtect    = (Temp & 0x10) >> 4;
    SD_CardInfo.SD_csd.FileFormat       = (Temp & 0x0C) >> 2;
    SD_CardInfo.SD_csd.ECC              = (Temp & 0x03);

    // Byte 15
    Temp = (uint8_t)(SD_Handle.CSD[3] & 0x000000FF);
    SD_CardInfo.SD_csd.CSD_CRC   = (Temp & 0xFE) >> 1;
    SD_CardInfo.SD_csd.Reserved4 = 1;

    // Byte 0
    Temp = (uint8_t)((SD_Handle.CID[0] & 0xFF000000) >> 24);
    SD_CardInfo.SD_cid.ManufacturerID = Temp;

    // Byte 1
    Temp = (uint8_t)((SD_Handle.CID[0] & 0x00FF0000) >> 16);
    SD_CardInfo.SD_cid.OEM_AppliID = Temp << 8;

    // Byte 2
    Temp = (uint8_t)((SD_Handle.CID[0] & 0x000000FF00) >> 8);
    SD_CardInfo.SD_cid.OEM_AppliID |= Temp;

    // Byte 3
    Temp = (uint8_t)(SD_Handle.CID[0] & 0x000000FF);
    SD_CardInfo.SD_cid.ProdName1 = Temp << 24;

    // Byte 4
    Temp = (uint8_t)((SD_Handle.CID[1] & 0xFF000000) >> 24);
    SD_CardInfo.SD_cid.ProdName1 |= Temp << 16;

    // Byte 5
    Temp = (uint8_t)((SD_Handle.CID[1] & 0x00FF0000) >> 16);
    SD_CardInfo.SD_cid.ProdName1 |= Temp << 8;

    // Byte 6
    Temp = (uint8_t)((SD_Handle.CID[1] & 0x0000FF00) >> 8);
    SD_CardInfo.SD_cid.ProdName1 |= Temp;

    // Byte 7
    Temp = (uint8_t)(SD_Handle.CID[1] & 0x000000FF);
    SD_CardInfo.SD_cid.ProdName2 = Temp;

    // Byte 8
    Temp = (uint8_t)((SD_Handle.CID[2] & 0xFF000000) >> 24);
    SD_CardInfo.SD_cid.ProdRev = Temp;

    // Byte 9
    Temp = (uint8_t)((SD_Handle.CID[2] & 0x00FF0000) >> 16);
    SD_CardInfo.SD_cid.ProdSN = Temp << 24;

    // Byte 10
    Temp = (uint8_t)((SD_Handle.CID[2] & 0x0000FF00) >> 8);
    SD_CardInfo.SD_cid.ProdSN |= Temp << 16;

    // Byte 11
    Temp = (uint8_t)(SD_Handle.CID[2] & 0x000000FF);
    SD_CardInfo.SD_cid.ProdSN |= Temp << 8;

    // Byte 12
    Temp = (uint8_t)((SD_Handle.CID[3] & 0xFF000000) >> 24);
    SD_CardInfo.SD_cid.ProdSN |= Temp;

    // Byte 13
    Temp = (uint8_t)((SD_Handle.CID[3] & 0x00FF0000) >> 16);
    SD_CardInfo.SD_cid.Reserved1   |= (Temp & 0xF0) >> 4;
    SD_CardInfo.SD_cid.ManufactDate = (Temp & 0x0F) << 8;

    // Byte 14
    Temp = (uint8_t)((SD_Handle.CID[3] & 0x0000FF00) >> 8);
    SD_CardInfo.SD_cid.ManufactDate |= Temp;

    // Byte 15
    Temp = (uint8_t)(SD_Handle.CID[3] & 0x000000FF);
    SD_CardInfo.SD_cid.CID_CRC   = (Temp & 0xFE) >> 1;
    SD_CardInfo.SD_cid.Reserved2 = 1;

    return ErrorState;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Enables wide bus operation for the requested card if supported by
  *         card.
  * @param  WideMode: Specifies the SD card wide bus mode
  *          This parameter can be one of the following values:
  *            @arg SD_BUS_WIDE_8B: 8-bit data transfer (Only for MMC)
  *            @arg SD_BUS_WIDE_4B: 4-bit data transfer
  *            @arg SD_BUS_WIDE_1B: 1-bit data transfer
  * @retval SD Card error state
  */
static SD_Error_t SD_WideBusOperationConfig(uint32_t WideMode)
{
    SD_Error_t ErrorState = SD_OK;
    uint32_t   Temp;
    uint32_t   SCR[2] = {0, 0};

    if((SD_CardType == SD_STD_CAPACITY_V1_1) || (SD_CardType == SD_STD_CAPACITY_V2_0) ||\
            (SD_CardType == SD_HIGH_CAPACITY))
    {
        if(WideMode == SD_BUS_WIDE_8B)
        {
            ErrorState = SD_UNSUPPORTED_FEATURE;
        }
        else if((WideMode == SD_BUS_WIDE_4B) ||
                (WideMode == SD_BUS_WIDE_1B))
        {
            if((sdmmc_instance->RESP1 & SD_CARD_LOCKED) != SD_CARD_LOCKED)
            {
                // Get SCR Register
                    ErrorState = SD_FindSCR(SCR);
                if(ErrorState == SD_OK)
                {
                    Temp = (WideMode == SD_BUS_WIDE_4B) ? SD_WIDE_BUS_SUPPORT : SD_SINGLE_BUS_SUPPORT;

                    // If requested card supports wide bus operation
                    if((SCR[1] & Temp) != SD_ALLZERO)
                    {
                        // Send CMD55 APP_CMD with argument as card's RCA.
                            ErrorState = SD_TransmitCommand((SD_CMD_APP_CMD | SD_CMD_RESPONSE_SHORT), SD_CardRCA, 1);
                        if(ErrorState == SD_OK)
                        {
                            Temp = (WideMode == SD_BUS_WIDE_4B) ? 2 : 0;

                            // Send ACMD6 APP_CMD with argument as 2 for wide bus mode
                            ErrorState =  SD_TransmitCommand((SD_CMD_APP_SD_SET_BUSWIDTH | SD_CMD_RESPONSE_SHORT), Temp, 1);
                        }
                    }
                    else
                    {
                        ErrorState = SD_REQUEST_NOT_APPLICABLE;
                    }
                }
            }
            else
            {
                ErrorState = SD_LOCK_UNLOCK_FAILED;
            }
        }
        else
        {
            ErrorState = SD_INVALID_PARAMETER;  // WideMode is not a valid argument
        }

        if(ErrorState == SD_OK)
        {
            // Configure the sdmmc_instance peripheral, we need this delay for some reason...
                while ((READ_REG(sdmmc_instance->CLKCR) & 0x800) != WideMode) {
                        MODIFY_REG(sdmmc_instance->CLKCR, CLKCR_CLEAR_MASK, (uint32_t) WideMode);
                }
        }
    }
    else {
            ErrorState = SD_UNSUPPORTED_FEATURE;
    }


    return ErrorState;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Switches the SD card to High Speed mode.
  *         This API must be used after "Transfer State"
  * @retval SD Card error state
  */
/*
SD_Error_t HAL_SD_HighSpeed(void)
{
    SD_Error_t  ErrorState;
    uint8_t     SD_hs[64]  = {0};
    uint32_t    SD_scr[2]  = {0, 0};
    uint32_t    SD_SPEC    = 0;
    uint32_t    Count      = 0;
    uint32_t*   Buffer     = (uint32_t *)SD_hs;

    // Initialize the Data control register
    sdmmc_instance->DCTRL = 0;

    // Get SCR Register
    if((ErrorState = SD_FindSCR(SD_scr)) != SD_OK)
    {
        return ErrorState;
    }

    // Test the Version supported by the card
    SD_SPEC = (SD_scr[1]  & 0x01000000) | (SD_scr[1]  & 0x02000000);

    if(SD_SPEC != SD_ALLZERO)
    {
        // Set Block Size for Card
        if((ErrorState = SD_TransmitCommand((SD_CMD_SET_BLOCKLEN | SDMMC_CMD_RESPONSE_SHORT), 64, 1)) != SD_OK)
        {
            return ErrorState;
        }

        // Configure the SD DPSM (Data Path State Machine)
        SD_DataTransferInit(64, SDMMC_DATABLOCK_SIZE_64B, true);

        // Send CMD6 switch mode
        if((ErrorState =SD_TransmitCommand((SD_CMD_HS_SWITCH | SDMMC_CMD_RESPONSE_SHORT), 0x80FFFF01, 1)) != SD_OK)
        {
            return ErrorState;
        }

        while((sdmmc_instance->STA & (SDMMC_STA_RXOVERR | SDMMC_STA_DCRCFAIL | SDMMC_STA_DTIMEOUT | SDMMC_STA_DBCKEND)) == 0)
        {
            if((sdmmc_instance->STA & SDMMC_STA_RXFIFOHF) != 0)
            {
                for(Count = 0; Count < 8; Count++)
                {
                    *(Buffer + Count) = sdmmc_instance->FIFO;
                }

                Buffer += 8;
            }
        }

        if((sdmmc_instance->STA & SDMMC_STA_DTIMEOUT) != 0)        return SD_DATA_TIMEOUT;
        else if((sdmmc_instance->STA & SDMMC_STA_DCRCFAIL) != 0)   return SD_DATA_CRC_FAIL;
        else if((sdmmc_instance->STA & SDMMC_STA_RXOVERR) != 0)    return SD_RX_OVERRUN;

        Count = SD_DATATIMEOUT;

        while(((sdmmc_instance->STA & SDMMC_STA_RXDAVL) != 0) && (Count > 0))
        {
            *Buffer = sdmmc_instance->FIFO;
            Buffer++;
            Count--;
        }

        // Test if the switch mode HS is ok
        if((SD_hs[13] & 2) != 2)
        {
            ErrorState = SD_UNSUPPORTED_FEATURE;
        }
    }

    return ErrorState;
}

*/


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Gets the current card's data status.
  * @retval Data Transfer state
  */
SD_Error_t SD_GetStatus(void)
{
    SD_Error_t     ErrorState;
    uint32_t       Response1;
    SD_CardState_t CardState;


    // Send Status command
    if((ErrorState = SD_TransmitCommand((SD_CMD_SEND_STATUS | SD_CMD_RESPONSE_SHORT), SD_CardRCA, 1)) == SD_OK)
    {
        Response1 = sdmmc_instance->RESP1;
        CardState = (SD_CardState_t)((Response1 >> 9) & 0x0F);

        // Find SD status according to card state
        if     (CardState == SD_CARD_TRANSFER)  ErrorState = SD_OK;
        else if(CardState == SD_CARD_ERROR)     ErrorState = SD_ERROR;
        else                                    ErrorState = SD_BUSY;
    }
    else
    {
        ErrorState = SD_CARD_ERROR;
    }

    return ErrorState;
}

static SD_Error_t SD_Abort(void) {
    SD_Error_t error = SD_OK;

    // Disable all sdmmc_instance peripheral interrupt sources
    sdmmc_instance->MASK &= ~(SDMMC_MASK_DCRCFAILIE | SDMMC_MASK_DTIMEOUTIE
            | SDMMC_MASK_DATAENDIE |
            SDMMC_MASK_TXFIFOHEIE | SDMMC_MASK_RXFIFOHFIE | SDMMC_MASK_TXUNDERRIE |
            SDMMC_MASK_RXOVERRIE | SDMMC_MASK_IDMABTCIE);

    // Clear all static flags
    sdmmc_instance->ICR |= SDMMC_ICR_STATIC_FLAGS;

    // Disable IDMA
    sdmmc_instance->IDMACTRL &= ~SDMMC_IDMA_IDMAEN;

    if (SD_GetStatus() == SD_BUSY) {
        error = SD_TransmitCommand((SDMMC_CMD_STOP_TRANSMISSION | SDMMC_RESPONSE_SHORT), 0, 1);
    }
    return error;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Gets the SD card status.
  * @retval SD Card error state
  */
SD_Error_t SD_GetCardStatus(SD_CardStatus_t* pCardStatus)
{
    SD_Error_t ErrorState;
    uint32_t   Temp = 0;
    uint32_t   Status[16];
    uint32_t   Count;

    // Check SD response
    if((sdmmc_instance->RESP1 & SD_CARD_LOCKED) == SD_CARD_LOCKED)
    {
        return SD_LOCK_UNLOCK_FAILED;
    }

    // Set block size for card if it is not equal to current block size for card
    if((ErrorState = SD_TransmitCommand((SD_CMD_SET_BLOCKLEN | SD_CMD_RESPONSE_SHORT), 64, 1)) != SD_OK)
    {
        return ErrorState;
    }

    // Send CMD55
    if((ErrorState = SD_TransmitCommand((SD_CMD_APP_CMD | SD_CMD_RESPONSE_SHORT), SD_CardRCA, 1)) != SD_OK)
    {
        return ErrorState;
    }

    // Configure the SD DPSM (Data Path State Machine)
    SD_DataTransferInit(64, SD_DATABLOCK_SIZE_64B, true, true);

    // Send ACMD13 (SD_APP_STAUS)  with argument as card's RCA
    if((ErrorState = SD_TransmitCommand((SD_CMD_SD_APP_STATUS | SD_CMD_RESPONSE_SHORT), 0, 1)) != SD_OK)
    {
        return ErrorState;
    }

    // Get status data
    while((sdmmc_instance->STA & (SDMMC_STA_RXOVERR | SDMMC_STA_DCRCFAIL | SDMMC_STA_DTIMEOUT | SDMMC_STA_DBCKEND)) == 0)
    {
        if((sdmmc_instance->STA & SDMMC_STA_RXFIFOHF) != 0)
        {
            for(Count = 0; Count < 8; Count++)
            {
                Status[Count] = sdmmc_instance->FIFO;
            }
        }
    }

    if((sdmmc_instance->STA & SDMMC_STA_DTIMEOUT) != 0)         return SD_DATA_TIMEOUT;
    else if((sdmmc_instance->STA & SDMMC_STA_DCRCFAIL) != 0)    return SD_DATA_CRC_FAIL;
    else if((sdmmc_instance->STA & SDMMC_STA_RXOVERR) != 0)     return SD_RX_OVERRUN;
    else
    {
    /*
        this part from the HAL is very strange has it is possible to overflow the provide buffer... and this originate from ST HAL

        Count = SD_DATATIMEOUT;
        while(((sdmmc_instance->STA & SDMMC_STA_RXDAVL) != 0) && (Count > 0))
        {
            *pSDstatus = sdmmc_instance->FIFO;
            pSDstatus++;
            Count--;
        }
    */
    }

    // Byte 0
    Temp = (Status[0] & 0xC0) >> 6;
    pCardStatus->DAT_BUS_WIDTH = (uint8_t)Temp;

    // Byte 0
    Temp = (Status[0] & 0x20) >> 5;
    pCardStatus->SECURED_MODE = (uint8_t)Temp;

    // Byte 2
    Temp = (Status[2] & 0xFF);
    pCardStatus->SD_CARD_TYPE = (uint8_t)(Temp << 8);

    // Byte 3
    Temp = (Status[3] & 0xFF);
    pCardStatus->SD_CARD_TYPE |= (uint8_t)Temp;

    // Byte 4
    Temp = (Status[4] & 0xFF);
    pCardStatus->SIZE_OF_PROTECTED_AREA = (uint8_t)(Temp << 24);

    // Byte 5
    Temp = (Status[5] & 0xFF);
    pCardStatus->SIZE_OF_PROTECTED_AREA |= (uint8_t)(Temp << 16);

    // Byte 6
    Temp = (Status[6] & 0xFF);
    pCardStatus->SIZE_OF_PROTECTED_AREA |= (uint8_t)(Temp << 8);

    // Byte 7
    Temp = (Status[7] & 0xFF);
    pCardStatus->SIZE_OF_PROTECTED_AREA |= (uint8_t)Temp;

    // Byte 8
    Temp = (Status[8] & 0xFF);
    pCardStatus->SPEED_CLASS = (uint8_t)Temp;

    // Byte 9
    Temp = (Status[9] & 0xFF);
    pCardStatus->PERFORMANCE_MOVE = (uint8_t)Temp;

    // Byte 10
    Temp = (Status[10] & 0xF0) >> 4;
    pCardStatus->AU_SIZE = (uint8_t)Temp;

    // Byte 11
    Temp = (Status[11] & 0xFF);
    pCardStatus->ERASE_SIZE = (uint8_t)(Temp << 8);

    // Byte 12
    Temp = (Status[12] & 0xFF);
    pCardStatus->ERASE_SIZE |= (uint8_t)Temp;

    // Byte 13
    Temp = (Status[13] & 0xFC) >> 2;
    pCardStatus->ERASE_TIMEOUT = (uint8_t)Temp;

    // Byte 13
    Temp = (Status[13] & 0x3);
    pCardStatus->ERASE_OFFSET = (uint8_t)Temp;

    return SD_OK;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Enquires cards about their operating voltage and configures clock
  *         controls and stores SD information that will be needed in future
  *         in the SD handle.
  * @retval SD Card error state
  */
static SD_Error_t SD_PowerON(void)
{
    SD_Error_t ErrorState;
    uint32_t   Response;
    uint32_t   Count;
    uint32_t   ValidVoltage;
    uint32_t   SD_Type;
    //uint32_t   TickStart;

    Count        = 0;
    ValidVoltage = 0;
    SD_Type      = SD_RESP_STD_CAPACITY;

    // TODO: Fix this sequence
    // Power ON Sequence -------------------------------------------------------
    sdmmc_instance->POWER  |= SDMMC_POWER_PWRCTRL;       // Set Power State to ON

    // 1ms: required power up waiting time before starting the SD initialization sequence (make it 2 to be safe)
    HAL_Delay(2);

    // CMD0: GO_IDLE_STATE -----------------------------------------------------
    // No CMD response required
    if((ErrorState = SD_TransmitCommand(SD_CMD_GO_IDLE_STATE, 0, 0)) != SD_OK)
    {
        // CMD Response Timeout (wait for CMDSENT flag)
        return ErrorState;
    }

    // CMD8: SEND_IF_COND ------------------------------------------------------
    // Send CMD8 to verify SD card interface operating condition
    // Argument: - [31:12]: Reserved (shall be set to '0')
    //- [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
    //- [7:0]: Check Pattern (recommended 0xAA)
    // CMD Response: R7 */
    if((ErrorState = SD_TransmitCommand((SD_CMD_HS_SEND_EXT_CSD | SD_CMD_RESPONSE_SHORT), SD_CHECK_PATTERN, 7)) == SD_OK)
    {
        // SD Card 2.0
        SD_CardType = SD_STD_CAPACITY_V2_0;
        SD_Type     = SD_RESP_HIGH_CAPACITY;
    }

    // Send CMD55
    // If ErrorState is Command Timeout, it is a MMC card
    // If ErrorState is SD_OK it is a SD card: SD card 2.0 (voltage range mismatch) or SD card 1.x
    if((ErrorState = SD_TransmitCommand((SD_CMD_APP_CMD | SD_CMD_RESPONSE_SHORT), 0, 1)) == SD_OK)
    {
        // SD CARD
        // Send ACMD41 SD_APP_OP_COND with Argument 0x80100000
        while((ValidVoltage == 0) && (Count < SD_MAX_VOLT_TRIAL))
        {
            // SEND CMD55 APP_CMD with RCA as 0
            if((ErrorState = SD_TransmitCommand((SD_CMD_APP_CMD | SD_CMD_RESPONSE_SHORT), 0, 1)) != SD_OK)
            {
                return ErrorState;
            }

            // Send CMD41
            if((ErrorState = SD_TransmitCommand((SD_CMD_SD_APP_OP_COND | SD_CMD_RESPONSE_SHORT), SD_VOLTAGE_WINDOW_SD | SD_Type, 3)) != SD_OK)
            {
                return ErrorState;
            }

            Response = sdmmc_instance->RESP1;                               // Get command response
            ValidVoltage = (((Response >> 31) == 1) ? 1 : 0);       // Get operating voltage
            Count++;
        }

        if(Count >= SD_MAX_VOLT_TRIAL)
        {
            return SD_INVALID_VOLTRANGE;
        }

        if((Response & SD_RESP_HIGH_CAPACITY) == SD_RESP_HIGH_CAPACITY)
        {
            SD_CardType = SD_HIGH_CAPACITY;
        }
    } // else MMC Card

    return ErrorState;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Turns the sdmmc_instance output signals off.
  * @retval SD Card error state
  */
#if 0
static void SD_PowerOFF(void)
{
   // Set Power State to OFF
   sdmmc_instance->POWER = (uint32_t)0;
}
#endif


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Finds the SD card SCR register value.
  * @param  pSCR: pointer to the buffer that will contain the SCR value
  * @retval SD Card error state
  */
static SD_Error_t SD_FindSCR(uint32_t *pSCR)
{
    SD_Error_t ErrorState;
    uint32_t Index = 0;
    uint32_t tempscr[2] = {0, 0};

    // Set Block Size To 8 Bytes
    // Send CMD55 APP_CMD with argument as card's RCA
    if((ErrorState = SD_TransmitCommand((SD_CMD_SET_BLOCKLEN | SD_CMD_RESPONSE_SHORT), 8, 1)) == SD_OK)
    {
        // Send CMD55 APP_CMD with argument as card's RCA
        if((ErrorState = SD_TransmitCommand((SD_CMD_APP_CMD | SD_CMD_RESPONSE_SHORT), SD_CardRCA, 1)) == SD_OK)
        {
            SD_DataTransferInit(8, SD_DATABLOCK_SIZE_8B, true, true);

            // Send ACMD51 SD_APP_SEND_SCR with argument as 0
            if((ErrorState = SD_TransmitCommand((SD_CMD_SD_APP_SEND_SCR | SD_CMD_RESPONSE_SHORT), 0, 1)) == SD_OK)
            {
                while((sdmmc_instance->STA & (SDMMC_STA_RXOVERR | SDMMC_STA_DCRCFAIL | SDMMC_STA_DTIMEOUT | SDMMC_STA_DBCKEND | SDMMC_STA_DATAEND)) == 0)
                {
                    if((sdmmc_instance->STA & SDMMC_STA_RXFIFOE) == 0)
                    {
                        *(tempscr + Index) = sdmmc_instance->FIFO;
                        Index++;
                    }
                }

                if     ((sdmmc_instance->STA & SDMMC_STA_DTIMEOUT) != 0) ErrorState = SD_DATA_TIMEOUT;
                else if((sdmmc_instance->STA & SDMMC_STA_DCRCFAIL) != 0) ErrorState = SD_DATA_CRC_FAIL;
                else if((sdmmc_instance->STA & SDMMC_STA_RXOVERR)  != 0) ErrorState = SD_RX_OVERRUN;
                else
                {
                    *(pSCR + 1) = ((tempscr[0] & SD_0TO7BITS) << 24)  | ((tempscr[0] & SD_8TO15BITS) << 8) |
                                  ((tempscr[0] & SD_16TO23BITS) >> 8) | ((tempscr[0] & SD_24TO31BITS) >> 24);

                    *(pSCR) = ((tempscr[1] & SD_0TO7BITS) << 24)  | ((tempscr[1] & SD_8TO15BITS) << 8) |
                              ((tempscr[1] & SD_16TO23BITS) >> 8) | ((tempscr[1] & SD_24TO31BITS) >> 24);
                }

            }
        }
    }

    return ErrorState;
}


/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Checks if the SD card is in programming state.
  * @param  pStatus: pointer to the variable that will contain the SD card state
  * @retval SD Card error state
  */
/*
static SD_Error_t SD_IsCardProgramming(uint8_t *pStatus)
{
    uint32_t Response_R1;

    SD_TransmitCommand((SD_CMD_SEND_STATUS | SDMMC_CMD_RESPONSE_SHORT), SD_CardRCA, 0);
    if((sdmmc_instance->STA & SDMMC_STA_CTIMEOUT) != 0)         return SD_CMD_RSP_TIMEOUT;
    else if((sdmmc_instance->STA & SDMMC_STA_CCRCFAIL) != 0)    return SD_CMD_CRC_FAIL;
    if((uint32_t)sdmmc_instance->RESPCMD != SD_CMD_SEND_STATUS) return SD_ILLEGAL_CMD;  // Check if is of desired command
    Response_R1 = sdmmc_instance->RESP1;                                                // We have received response, retrieve it for analysis
    *pStatus = (uint8_t)((Response_R1 >> 9) & 0x0000000F);                      // Find out card status

    return CheckOCR_Response(Response_R1);
}
*/

/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  Initialize the sdmmc_instance module, DMA, and IO
  */
void SD_Initialize_LL(SDMMC_TypeDef *sdmmc)
{
    sdmmc_instance = sdmmc;

     // Reset sdmmc_instance Module
    if (sdmmc_instance == SDMMC1) {
        RCC->AHB2RSTR |=  RCC_AHB3RSTR_SDMMC1RST;
        HAL_Delay(1);
        RCC->AHB2RSTR &= ~RCC_AHB3RSTR_SDMMC1RST;
        HAL_Delay(1);

        // Enable sdmmc_instance clock
        RCC->AHB3ENR |= RCC_AHB3ENR_SDMMC1EN;

        //Configure Pins
        RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN | RCC_AHB4ENR_GPIODEN;

        IO_PinInit(SDMMC1_D0_GPIO_Port, SDMMC1_D0_CLK, SDMMC1_D0_Pin, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_PP, GPIO_OSPEEDR_VERY_HIGH, GPIO_AF12_SDMMC1);
    #ifdef SDMMC_4BIT
        IO_PinInit(SDMMC1_D1_GPIO_Port, SDMMC1_D1_CLK, SDMMC1_D1_Pin, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_PP, GPIO_OSPEEDR_VERY_HIGH, GPIO_AF12_SDMMC1);
        IO_PinInit(SDMMC1_D2_GPIO_Port, SDMMC1_D2_CLK, SDMMC1_D2_Pin, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_PP, GPIO_OSPEEDR_VERY_HIGH, GPIO_AF12_SDMMC1);
        IO_PinInit(SDMMC1_D3_GPIO_Port, SDMMC1_D3_CLK, SDMMC1_D3_Pin, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_PP, GPIO_OSPEEDR_VERY_HIGH, GPIO_AF12_SDMMC1);
    #endif
        IO_PinInit(SDMMC1_CLK_GPIO_Port, SDMMC1_CLK_CLK, SDMMC1_CLK_Pin, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_PP, GPIO_OSPEEDR_VERY_HIGH, GPIO_AF12_SDMMC1);
        IO_PinInit(SDMMC1_CMD_GPIO_Port, SDMMC1_CMD_CLK, SDMMC1_CMD_Pin, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_PP, GPIO_OSPEEDR_VERY_HIGH, GPIO_AF12_SDMMC1);

        // NVIC configuration for SDIO interrupts
        NVIC_SetPriority(SDMMC1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
        NVIC_EnableIRQ(SDMMC1_IRQn);
    } else {
        RCC->AHB2RSTR |=  RCC_AHB2RSTR_SDMMC2RST;
        HAL_Delay(1);
        RCC->AHB2RSTR &= ~RCC_AHB2RSTR_SDMMC2RST;
        HAL_Delay(1);

        // Enable sdmmc_instance clock
        RCC->AHB2ENR |= RCC_AHB2ENR_SDMMC2EN;

        IO_PinInit(SDMMC2_D0_GPIO_Port, SDMMC2_D0_CLK, SDMMC2_D0_Pin, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_PP, GPIO_OSPEEDR_VERY_HIGH, GPIO_AF9_SDMMC2);
    #ifdef SDMMC_4BIT
        IO_PinInit(SDMMC2_D1_GPIO_Port, SDMMC2_D1_CLK, SDMMC2_D1_Pin, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_PP, GPIO_OSPEEDR_VERY_HIGH, GPIO_AF9_SDMMC2);
        IO_PinInit(SDMMC2_D2_GPIO_Port, SDMMC2_D2_CLK, SDMMC2_D2_Pin, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_PP, GPIO_OSPEEDR_VERY_HIGH, GPIO_AF9_SDMMC2);
        IO_PinInit(SDMMC2_D3_GPIO_Port, SDMMC2_D3_CLK, SDMMC2_D3_Pin, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_PP, GPIO_OSPEEDR_VERY_HIGH, GPIO_AF9_SDMMC2);
    #endif
        IO_PinInit(SDMMC2_CLK_GPIO_Port, SDMMC2_CLK_CLK, SDMMC2_CLK_Pin, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_PP, GPIO_OSPEEDR_VERY_HIGH, GPIO_AF9_SDMMC2);
        IO_PinInit(SDMMC2_CMD_GPIO_Port, SDMMC2_CMD_CLK, SDMMC2_CMD_Pin, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_PP, GPIO_OSPEEDR_VERY_HIGH, GPIO_AF9_SDMMC2);

        // NVIC configuration for SDIO interrupts
        NVIC_SetPriority(SDMMC2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
        NVIC_EnableIRQ(SDMMC2_IRQn);
    }
}


/** -----------------------------------------------------------------------------------------------------------------*/
bool SD_GetState(void)
{
    // Check SDCARD status
    if(SD_GetStatus() == SD_OK) return true;
    return false;
}


/** -----------------------------------------------------------------------------------------------------------------*/
bool SD_Init(uint8_t clk_div)
{
    SD_Error_t ErrorState;

    // Check if SD card is present
    if(SD_IsDetected() != SD_PRESENT)
    {
        return false;
    }

    // Initialize sdmmc_instance peripheral interface with default configuration for SD card initialization
    MODIFY_REG(sdmmc_instance->CLKCR, CLKCR_CLEAR_MASK, (uint32_t) SDMMC_INIT_CLK_DIV);
    MODIFY_REG(sdmmc_instance->POWER, SDMMC_POWER_DIRPOL_Msk, SDMMC_POWER_DIRPOL);

    if((ErrorState = SD_PowerON()) == SD_OK)                    // Identify card operating voltage
    {
        if((ErrorState = SD_InitializeCard()) == SD_OK)         // Initialize the present card and put them in idle state
        {
            if((ErrorState = SD_GetCardInfo()) == SD_OK)        // Read CSD/CID MSD registers
            {
                // Select the Card - Send CMD7 SDMMC_SEL_DESEL_CARD
                ErrorState = SD_TransmitCommand((SD_CMD_SEL_DESEL_CARD | SD_CMD_RESPONSE_SHORT), SD_CardRCA, 1);
                MODIFY_REG(sdmmc_instance->CLKCR, CLKCR_CLEAR_MASK, (uint32_t) clk_div); // Configure sdmmc_instance peripheral interface
            }
        }
    }

    // Configure SD Bus width
    if(ErrorState == SD_OK)
    {
        // Enable wide operation
#ifdef USE_SDIO_1BIT
        ErrorState = SD_WideBusOperationConfig(SD_BUS_WIDE_1B);
#else
        ErrorState = SD_WideBusOperationConfig(SD_BUS_WIDE_4B);
#endif

    }

    // Configure the SDCARD device
    return ErrorState;
}

/** -----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief  This function handles SD card interrupt request.
  */


static inline void SDMMC_IRQHandler(void) {
    // Check for sdmmc_instance interrupt flags
    uint32_t status = sdmmc_instance->STA;
    // Disable all sdmmc_instance peripheral interrupt sources
    sdmmc_instance->MASK &= ~(SDMMC_MASK_DCRCFAILIE | SDMMC_MASK_DTIMEOUTIE
            | SDMMC_MASK_DATAENDIE |
            SDMMC_MASK_TXFIFOHEIE | SDMMC_MASK_RXFIFOHFIE | SDMMC_MASK_TXUNDERRIE |
            SDMMC_MASK_RXOVERRIE | SDMMC_MASK_IDMABTCIE);

    if ((status & SDMMC_STA_DATAEND) != 0) {

        // Disable CMDTRANS
        sdmmc_instance->CMD &= ~(SDMMC_CMD_CMDTRANS);

        /* Disable the DMA transfer for transmit request by setting the DMAEN bit
        in the SD DCTRL register */
        sdmmc_instance->IDMACTRL &= ~(SDMMC_IDMA_IDMAEN);

        if ((SD_Handle.Operation & 0x01) == SD_MULTIPLE_BLOCK) {
            /* Send stop command in multiblock write */
            SD_TransmitCommand((SD_CMD_STOP_TRANSMISSION | SD_CMD_RESPONSE_SHORT), 0, 1);
        }

        if ((SD_Handle.Operation & 0x02) == (SDMMC_DIR_TX << 1)) {
            /* Transfer is complete */
            SD_Handle.TXCplt = 0;
        }

        if ((SD_Handle.Operation & 0x02) == (SDMMC_DIR_RX << 1)) {
#ifdef SDMMC_CACHE_MAINTANANCE
            // Invalidate cache
            if (SCB->CCR & SCB_CCR_DC_Msk) {
                uint32_t alignedAddr = (uint32_t)sdmmc_instance->IDMABASE0 & ~0x1F;
                SCB_InvalidateDCache_by_Addr((uint32_t*)alignedAddr, sdmmc_instance->DLEN + ((uint32_t)sdmmc_instance->IDMABASE0 - alignedAddr));
            }
#endif

            /* Transfer is complete */
            SD_Handle.RXCplt = 0;
        }

        SD_Handle.TransferComplete = 1;
        SD_Handle.TransferError = SD_OK;       // No transfer error
    }
    else if ((status & SDMMC_STA_IDMATE) != 0) {
        sdmmc_instance->IDMACTRL &= ~(SDMMC_IDMA_IDMAEN);
        SD_Handle.TransferError = SD_IDMA_ERROR;
    }
    else if ((status & SDMMC_STA_DCRCFAIL) != 0)
        SD_Handle.TransferError = SD_DATA_CRC_FAIL;
    else if ((status & SDMMC_STA_DTIMEOUT) != 0)
        SD_Handle.TransferError = SD_DATA_TIMEOUT;
    else if ((status & SDMMC_STA_RXOVERR) != 0)
        SD_Handle.TransferError = SD_RX_OVERRUN;
    else if ((status & SDMMC_STA_TXUNDERR) != 0)
        SD_Handle.TransferError = SD_TX_UNDERRUN;

    sdmmc_instance->ICR = SDMMC_ICR_STATIC_FLAGS;
}

void SDMMC1_IRQHandler(void) {
    SDMMC_IRQHandler();
}

void SDMMC2_IRQHandler(void) {
    SDMMC_IRQHandler();
}

/* ------------------------------------------------------------------------------------------------------------------*/
#endif
