
#include <common.h>
#include <asm/mach-types.h>
#include <asm/arch/memory.h>
#include <asm/arch/am_reg_addr.h>
#include <asm/arch/aml_card_sd.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/am_eth_reg.h>
#include <asm/arch/nand.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/mtd.h>
/**
 * Routine: board_init
 * Description: Early hardware init.
 */
 

static struct aml_nand_platform aml_nand_mid_platform[] = {
	{
		.name = NAND_BOOT_NAME,
		.chip_enable_pad = AML_NAND_CE0,
		.ready_busy_pad = AML_NAND_CE0,
		.platform_nand_data = {
			.chip =  {
				.nr_chips = 1,
				.options = (NAND_TIMING_MODE5 | NAND_ECC_BCH16_MODE),
			},
    	},
		.T_REA = 20,
		.T_RHOH = 15,
	},
	{
		.name = NAND_NORMAL_NAME,
		.chip_enable_pad = AML_NAND_CE0,
		.ready_busy_pad = AML_NAND_CE0,
		.platform_nand_data = {
			.chip =  {
				.nr_chips = 1,
				.options = (NAND_TIMING_MODE5 | NAND_ECC_BCH16_MODE | NAND_TWO_PLANE_MODE),
			},
    	},
		.T_REA = 20,
		.T_RHOH = 15,
	}
};

struct aml_nand_device aml_nand_mid_device = {
	.aml_nand_platform = aml_nand_mid_platform,
	.dev_num = ARRAY_SIZE(aml_nand_mid_platform),
};

int board_init(void)
{
    /*
    @todo implement this function
    */
	DECLARE_GLOBAL_DATA_PTR;
	gd->bd->bi_arch_number=MACH_TYPE_MESON_8626M;
	gd->bd->bi_boot_params=BOOT_PARAMS_OFFSET;

#ifdef CONFIG_CMD_KGDB

	kgdb_init();
#endif

	return 0;
}

/*
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 */
int misc_init_r(void)
{
/*
    @todo implement this function
    */

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
    /*
    @todo implement this function
    */
}

/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *		Ethernet hardware.
 */
static void setup_net_chip(void)
{
    /*
    @todo implement this function
    */
	
}

extern int aml_eth_init(bd_t *bis);
int board_eth_init(bd_t *bis)
{
    /*
    @todo implement this function
    */
          
  //  	eth_clk_set(ETH_CLKSRC_SYS_D3,900*CLK_1M/3,50*CLK_1M);
	//eth_clk_set(ETH_CLKSRC_SYS_D3,get_cpu_clk()*2/3,50*CLK_1M);
	eth_clk_set(ETH_CLKSRC_APLL_CLK,400*CLK_1M,50*CLK_1M);
	///GPIOD15-24 for 8626M;
	///GPIOD12	nRst;
	///GPIOD13    n_int;
//output 50M
	eth_set_pinmux(ETH_BANK1_GPIOD2_D11,ETH_CLK_OUT_GPIOD7_REG4_20,0);
//input 50M
//	eth_set_pinmux(ETH_BANK1_GPIOD2_D11,ETH_CLK_IN_GPIOD7_REG4_19,0);
	/*disalbe*/
	//reset:LCD_G5
	writel(readl(ETH_PLL_CNTL) & ~(1 << 0), ETH_PLL_CNTL); // Disable the Ethernet clocks     
	// ---------------------------------------------
	// Test 50Mhz Input Divide by 2
	// ---------------------------------------------
	// Select divide by 2
	writel(readl(ETH_PLL_CNTL) | (0 << 3), ETH_PLL_CNTL); // desc endianess "same order"   
	writel(readl(ETH_PLL_CNTL) | (0 << 2), ETH_PLL_CNTL); // data endianess "little"    
	writel(readl(ETH_PLL_CNTL) | (1 << 1), ETH_PLL_CNTL); // divide by 2 for 100M     
	writel(readl(ETH_PLL_CNTL) | (1 << 0), ETH_PLL_CNTL);  // enable Ethernet clocks   
	udelay(100);
	/*reset*/
	CLEAR_CBUS_REG_MASK(PREG_FGPIO_EN_N,1<<19);
	SET_CBUS_REG_MASK(PREG_FGPIO_O,1<<19);
	CLEAR_CBUS_REG_MASK(PREG_FGPIO_O,1<<19);
	udelay(1000);
	SET_CBUS_REG_MASK(PREG_FGPIO_O,1<<19);
	udelay(10);	//waiting reset end;
	//mode2 GPIOC_18
	//CLEAR_CBUS_REG_MASK(PREG_FGPIO_EN_N,1<<18);
	//CLEAR_CBUS_REG_MASK(PREG_FGPIO_O,1<<18);/*mode to low*/
	
	aml_eth_init(bis);
	return 0;
}


u32 get_board_rev(void)
{
    /*
    @todo implement this function
    */
	return 0x20;
}
static int  sdio_init(unsigned port)
{
	setbits_le32(P_PREG_AGPIO_EN_N,1<<11);

    return cpu_sdio_init(port);
}
static int  sdio_detect(unsigned port)
{
	return (readl(P_PREG_AGPIO_I)&(1<<11))?1:0;
}
static void sdio_pwr_on(unsigned port)
{
	clrbits_le32(P_PREG_AGPIO_O,(1<<9));
	clrbits_le32(P_PREG_AGPIO_EN_N,(1<<9));
    /// @todo NOT FINISH
}
static void sdio_pwr_off(unsigned port)
{
    setbits_le32(P_PREG_AGPIO_O,(1<<9));
	clrbits_le32(P_PREG_AGPIO_EN_N,(1<<9));//test_n
	
	/// @todo NOT FINISH
}

static void sdio_pwr_prepare(unsigned port)
{
    cpu_sdio_pwr_prepare(port);
}
static void board_mmc_register(unsigned port)
{
    AML_CARD_SD_INFO_t *aml_priv=cpu_sdio_get(port);
    if(aml_priv==NULL)
        return;
    struct mmc *mmc = (struct mmc *)malloc(sizeof(struct mmc));
    strncpy(mmc->name,aml_priv->name,31);
    mmc->priv = aml_priv;
	aml_priv->removed_flag = 1;
	aml_priv->inited_flag = 0;
	aml_priv->sdio_init=sdio_init;
	aml_priv->sdio_detect=sdio_detect;
	aml_priv->sdio_pwr_off=sdio_pwr_off;
	aml_priv->sdio_pwr_on=sdio_pwr_on;
	aml_priv->sdio_pwr_prepare=sdio_pwr_prepare;
	mmc->send_cmd = aml_sd_send_cmd;
	mmc->set_ios = aml_sd_cfg_swth;
	mmc->init = aml_sd_init;
	mmc->rca = 1;
	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->host_caps =  MMC_MODE_4BIT | MMC_MODE_HS;
	//mmc->host_caps = MMC_MODE_4BIT;
	mmc->bus_width = 1;
	mmc->clock = 200000;
	mmc->f_min = 200000;
	mmc->f_max = 24000000;
	mmc_register(mmc);
}
int board_mmc_init(bd_t	*bis)
{
	board_mmc_register(SDIO_PORT_A);
//	board_mmc_register(SDIO_PORT_B);
//	board_mmc_register(SDIO_PORT_C);
//	board_mmc_register(SDIO_PORT_B1);
	return 0;
}
