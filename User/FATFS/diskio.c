﻿/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
#include <string.h>
#include "diskio.h"
#include "stm32f10x.h"
#include "./sdio/bsp_sdio_sdcard.h"
#include "stdio.h"
#include "log.h"
/* 为每个设备定义一个物理编号 */
#define ATA			           0     // SD卡
#define SPI_FLASH		       1     // 预留外部SPI Flash使用

#define SD_BLOCKSIZE     512 

extern  SD_CardInfo SDCardInfo;

/*-----------------------------------------------------------------------*/
/* 获取设备状态                                                          */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
	BYTE pdrv		/* 物理编号 */
)
{
	DSTATUS status = STA_NOINIT;
	
	switch (pdrv) {
		case ATA:	/* SD CARD */
			status &= ~STA_NOINIT;
			break;
    
		case SPI_FLASH:        /* SPI Flash */   
			/* SPI Flash状态检测：读取SPI Flash 设备ID */
//			if(sFLASH_ID == SPI_FLASH_ReadID())
//			{
//			  /* 设备ID读取结果正确 */
//			  status &= ~STA_NOINIT;
//			}
//			else
//			{
//			  /* 设备ID读取结果错误 */
//			  status = STA_NOINIT;;
//			}
			break;

		default:
			status = STA_NOINIT;
	}
	return status;
}

/*-----------------------------------------------------------------------*/
/* 设备初始化                                                            */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
	BYTE pdrv				/* 物理编号 */
)
{
	DSTATUS status = STA_NOINIT;	
	switch (pdrv) {
		case ATA:	         /* SD CARD */
			if(SD_Init()==SD_OK)
			{
				status &= ~STA_NOINIT;
				// status = ~1 & 1
			}
			else 
			{
				status = STA_NOINIT;
			}
		
			break;
    
		case SPI_FLASH:    /* SPI Flash */ 
			break;
      
		default:
			status = STA_NOINIT;
	}
	return status;
}


/*-----------------------------------------------------------------------*/
/* 读扇区：读取扇区内容到指定存储区                                              */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE pdrv,		/* 设备物理编号(0..) */
	BYTE *buff,		/* 数据缓存区 */
	DWORD sector,	/* 扇区首地址 */
	UINT count		/* 扇区个数(1..128) */
)
{
	int i = 0;
	DRESULT status = RES_PARERR;
	SD_Error SD_state = SD_OK;
	DISKIO_DEBUG("\n");
	switch (pdrv) {

		case ATA:	/* SD CARD */						
		  if((DWORD)buff&3)	// 非4字节对齐
			{
				DRESULT res = RES_OK;
				DWORD scratch[SD_BLOCKSIZE / 4];
				DISKIO_DEBUG("\n");

				while (count--)
				{
					res = disk_read(ATA,(void *)scratch, sector++, 1);

					if (res != RES_OK) 
					{
						break;
					}
					memcpy(buff, scratch, SD_BLOCKSIZE);
					buff += SD_BLOCKSIZE;
		    	}
		    return res;
			}
			DISKIO_DEBUG("\n");
			for(i = 0; i < count; i++)
			{
				SD_state = SD_ReadBlock((BYTE*)(&buff[i<<9]), (sector + i) << 9,SD_BLOCKSIZE);
				SD_state = SD_WaitReadOperation();
				while(SD_GetStatus() != SD_TRANSFER_OK);
				if(SD_state!=SD_OK) {
					status = RES_PARERR;
					DISKIO_ERROR("status error %d\n", status);
					return RES_PARERR;
				}
				else
					status = RES_OK;  

			}

//			SD_state=SD_ReadMultiBlocks(buff,(uint64_t)sector*SD_BLOCKSIZE,SD_BLOCKSIZE,count);
//		  	if(SD_state==SD_OK)
//			{
//				/* Check if the Transfer is finished */
//				DISKIO_DEBUG("\n");
//				SD_state=SD_WaitReadOperation();
//				DISKIO_DEBUG("\n");
//				//while(i--);
//				while(SD_GetStatus() != SD_TRANSFER_OK){
//					printf(".");
//					for(i = 0; i < 20; i++){
//						printf("\n");
//					}
//										}
//			}
//		  	DISKIO_DEBUG("\n");
//			if(SD_state!=SD_OK)
//				status = RES_PARERR;
//		  	else
//			  	status = RES_OK;	
			break;   
			
		case SPI_FLASH:
		break;
    
		default:
			status = RES_PARERR;
	}
	DISKIO_DEBUG("\n");
	return status;
}

/*-----------------------------------------------------------------------*/
/* 写扇区：见数据写入指定扇区空间上                                      */
/*-----------------------------------------------------------------------*/
#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			  /* 设备物理编号(0..) */
	const BYTE *buff,	/* 欲写入数据的缓存区 */
	DWORD sector,		  /* 扇区首地址 */
	UINT count			  /* 扇区个数(1..128) */
)
{
	DRESULT status = RES_PARERR;
	SD_Error SD_state = SD_OK;
	int i = 0;
	if (!count) {
		return RES_PARERR;		/* Check parameter */
	}
	DISKIO_DEBUG("\n");
	switch (pdrv) {
		case ATA:	/* SD CARD */  
			if((DWORD)buff&3)	// 4字节对齐 不是4字节对齐的地址 进入
			{
				DRESULT res = RES_OK;
				DWORD scratch[SD_BLOCKSIZE / 4];	// 128

				while (count--) 
				{
					memcpy( scratch,buff,SD_BLOCKSIZE);	
					res = disk_write(ATA,(void *)scratch, sector++, 1);
					if (res != RES_OK) 
					{
						break;
					}					
					buff += SD_BLOCKSIZE;
		    	}
		    return res;
			}
			// 由于SDIO的时钟为24M STMF103的时钟72M， SDIO高速4BIT ，MCU好像读不过来 24X4X2
			for (i = 0; i < count; i++) {
				SD_state = SD_WriteBlock((BYTE*)(&buff[i<<9]), (uint64_t)(sector + i) << 9, SD_BLOCKSIZE);
//				buff =<< 9; 
//				sector =<< 9;
				SD_state = SD_WaitWriteOperation();
				while(SD_GetStatus() != SD_TRANSFER_OK);
			  	if(SD_state!=SD_OK) {
					status = RES_PARERR;
					DISKIO_ERROR("status error %d\n", status);
					return RES_PARERR;
			  	}
				else
					status = RES_OK;  
			}


//			SD_state=SD_WriteMultiBlocks((uint8_t *)buff,(uint64_t)sector*SD_BLOCKSIZE,SD_BLOCKSIZE,count);
//			if(SD_state==SD_OK)
//			{
//				int num = 0;
//				/* Check if the Transfer is finished */
//				SD_state=SD_WaitWriteOperation();
//				DISKIO_DEBUG("\n");
//				/* Wait until end of DMA transfer */
//
//				while(SD_GetStatus() != SD_TRANSFER_OK) {
//					printf(".");
//					num ++;
//					if (num == 20) {
//						num = 0;
//						printf("\n");
//					}
//				}
//			}
//			if(SD_state!=SD_OK)
//				status = RES_PARERR;
//		  	else
//			  	status = RES_OK;	
		break;

		case SPI_FLASH:
		break;
    
		default:
			status = RES_PARERR;
	}
	return status;
}
#endif


/*-----------------------------------------------------------------------*/
/* 其他控制                                                              */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* 物理编号 */
	BYTE cmd,		  /* 控制指令 *//* Command code for disk_ioctrl fucntion */
	void *buff		/* 写入或者读取数据地址指针 */
)
{
	DRESULT status = RES_PARERR;
	SDIO_DEBUG("\n");
	switch (pdrv) {	// 根据设备的类型
		case ATA:	/* SD CARD */
			switch (cmd) 
			{
				// Get R/W sector size (WORD) 
				case GET_SECTOR_SIZE :    
					*(WORD * )buff = SD_BLOCKSIZE;
				break;
				// Get erase block size in unit of sector (DWORD)
				case GET_BLOCK_SIZE :      
					*(DWORD * )buff = 1;
				break;

				case GET_SECTOR_COUNT:
					*(DWORD * )buff = SDCardInfo.CardCapacity/SDCardInfo.CardBlockSize;
					break;
				case CTRL_SYNC :
				break;
			}
			status = RES_OK;
			break;
    
		case SPI_FLASH:		      
		break;
    
		default:
			status = RES_PARERR;
	}
	return status;
}
#endif

							 
__weak DWORD get_fattime(void) {
	/* 返回当前时间戳 */
	return	  ((DWORD)(2015 - 1980) << 25)	/* Year 2015 */
			| ((DWORD)1 << 21)				/* Month 1 */
			| ((DWORD)1 << 16)				/* Mday 1 */
			| ((DWORD)0 << 11)				/* Hour 0 */
			| ((DWORD)0 << 5)				  /* Min 0 */
			| ((DWORD)0 >> 1);				/* Sec 0 */
}

