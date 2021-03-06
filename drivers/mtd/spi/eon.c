/*
 * Copyright C 2009 by Amlogic, Inc. All Rights Reserved.
 * Description: eon spi chips support  
 * Author:	pfs
 */
#define DEBUG

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_aml_internal.h"
#include <asm/arch/am_reg_addr.h>

struct eon_spi_flash_params {
	uint32_t	id;
	uint32_t	sector_size;
	uint32_t	block_size;
	uint32_t	chip_size;
	const char	*name;
};

/* spi_flash needs to be first so upper layers can free() it */
struct eon_spi_flash {
	struct spi_flash flash;
	const struct eon_spi_flash_params *params;
};

static inline struct eon_spi_flash * to_eon_spi_flash(struct spi_flash *flash)
{
	return container_of(flash, struct eon_spi_flash, flash);
}


#define EON_ID_EN25B16 0x2015
#define EON_ID_M25X32 0x2016
#define EON_ID_M25X64 0x2017
#define EON_ID_EN25F16 0x3115
#define EON_ID_EN25F40 0x3113

static const struct eon_spi_flash_params eon_spi_flash_table[] = {
	{
		.id			= EON_ID_EN25F16,
		.sector_size= 4*1024,
		.block_size	=64*1024 ,
		.chip_size	= 2*1024*1024,
		.name			= "EN25F16",
	},
	{
		.id			= EON_ID_EN25B16,
		.sector_size= 4*1024,
		.block_size	=64*1024 ,
		.chip_size	= 2*1024*1024,
		.name			= "EN25B16",
	},
	{
		.id			= EON_ID_EN25F40,
		.sector_size= 4*1024,
		.block_size	=64*1024 ,
		.chip_size	= 512*1024,
		.name			= "EN25F40",
	},
    /*
	{
		.id			= MXIC_ID_M25X16,
		.sector_size= 4*1024,
		.block_size	=64*1024 ,
		.chip_size	= 2*1024*1024,
		.name			= "M25X16",
	},
	{
		.id			= MXIC_ID_M25X32,
		.sector_size= 4*1024,
		.block_size	=64*1024 ,
		.chip_size	=4*1024*1024,
		.name		= "M25X32",
	},
	{
		.id			= MXIC_ID_M25X64,
		.sector_size= 4*1024,
		.block_size	= 64*1024 ,
		.chip_size	= 8*1024*1024,
		.name	 	= "M25X64",
	},
	{
		.id			= 0x2415,
		.sector_size= 4*1024,
		.block_size	=64*1024 ,
		.chip_size	= 2*1024*1024,
		.name			= "M25L1635D",
	},
	{
		.id			= 0x2416,
		.sector_size= 4*1024,
		.block_size	=64*1024 ,
		.chip_size	=4*1024*1024,
		.name		= "M25L3235D",
	},*/
};





static int eon_read(struct spi_flash *flash,u32 offset, size_t len, void *buf){

//	struct eon_spi_flash *stm = to_eon_spi_flash(flash);

	spi_claim_bus(flash->spi);

	spi_flash_read_aml(flash, offset,len,buf);
	
	spi_release_bus(flash->spi);
	
	return  0;
}



static int eon_write(struct spi_flash *flash,u32 offset, size_t len, const void *buf){

//	struct eon_spi_flash *stm = to_eon_spi_flash(flash);

	spi_claim_bus(flash->spi);
	
	spi_flash_write_aml(flash, offset, len,buf);
	
	spi_release_bus(flash->spi);
	
	return 0;

}




	
/*
 * This function currently uses sector erase only.
 * probably speed things up by using bulk erase
 * when possible.
 */
int eon_erase(struct spi_flash *flash, u32 offset, size_t len){

	struct eon_spi_flash *stm = to_eon_spi_flash(flash);
	struct spi_slave * slave=flash->spi;
	unsigned long sector_size;
	unsigned long page_addr;
	size_t actual;
	int ret;
	unsigned var;		

	sector_size = stm->params->sector_size;

	if (offset % sector_size || len % sector_size) {
		debug("SF: Erase offset/length not multiple of sector size\n");
		return -1;
	}

	page_addr = offset / sector_size;

	spi_claim_bus(flash->spi);
  //	CLEAR_PERI_REG_BITS(PERIPHS_SPI_FLASH_CTRL, SPI_ENABLE_AHB);

#ifdef SPI_WRITE_PROTECT
	spi_disable_write_protect();
#endif

	for (actual = 0; actual < len; actual+=sector_size) {
		
		debug("Erase:%x\n",actual);
			
		var=(offset+actual) & 0xffffff;
		spi_flash_adr_write(slave,var);	
	
		var=1<<SPI_FLASH_WREN;
		spi_flash_cmd(slave,var,NULL,0);	
		
		var=1<<SPI_FLASH_SE;
		spi_flash_cmd(slave,var,NULL,0);	
	
		ret=1;
  		while ( (ret&1)==1 ) {
    	
		var=1<<SPI_FLASH_RDSR;
		spi_flash_cmd(slave,var,&ret,2);		//2 byte status	
  		}
			
	
	}

	debug("SF: MXIC: Successfully erased %u bytes @ 0x%x\n",
			len, offset);
	ret = 0;


	spi_release_bus(flash->spi);
//	SET_PERI_REG_MASK(P_SPI_FLASH_CTRL, SPI_ENABLE_AHB);
	

	return ret;
}

struct spi_flash *spi_flash_probe_eon(struct spi_slave *spi, u8 *idcode)
{
	const struct eon_spi_flash_params *params;
	unsigned long sector_size;
	struct eon_spi_flash *stm;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(eon_spi_flash_table); i++) {
		params = &eon_spi_flash_table[i];
		if (params->id == ((idcode[1] << 8) | idcode[2]))
			break;
	}

	if (i == ARRAY_SIZE(eon_spi_flash_table)) {
		debug("SF: Unsupported MXIC ID %02x%02x\n",
				idcode[1], idcode[2]);
		return NULL;
	}

	stm = malloc(sizeof(struct eon_spi_flash));
	if (!stm) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	stm->params = params;
	stm->flash.spi = spi;
	stm->flash.name = params->name;


	stm->flash.write = eon_write;
	stm->flash.erase = eon_erase;
	stm->flash.read =  eon_read;
	stm->flash.size =  params->chip_size ;
	sector_size     =  params->sector_size;	

	debug("SF: Detected %s with sector size %u, total %u bytes\n",
			params->name, sector_size, stm->flash.size);

	return &stm->flash;
}
