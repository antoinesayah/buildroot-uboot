#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/arch/firm/reg_addr.h>

#if 0
static void enable_arc(void * arccode)
{
    spi_init();
    //copy ARM code
    memcpy(0x49008000,0x49000000,16*1024);
    writel((0x49008000>>14)&0xf,0xc810001c);
    //copy ARC code
    memcpy(0x49008000,arccode,16*1024);
    writel(0x1<<4,0xc8100020);
    writel(0x7fffffff,P_AO_RTI_STATUS_REG0);
    serial_puts("start up ARC\n");
    writel(0x51001,0xc8100030);
    writel(1,P_AO_RTI_STATUS_REG1);
    writel(0x1,0xc1109964);
    writel(0x0,0xc1109964);
    unsigned a,b;
    unsigned timer_base;
    a=b=0x7fffffff;
    printf("ARM is Live\n");
    timer_base=get_timer(0);
    do{
        a=readl(P_AO_RTI_STATUS_REG0);
        if((a&0x80000000)|| ((a==b)&&(get_timer(timer_base)<10000000)))
        {
            continue;
        }
        timer_base=get_timer(0);
        b=a;
        printf("ARM is Live%d\n",a);
        switch(a&0xffff)
        {
            case 0: 
                printf("ARM Exit Sleep Mode\n");
                break;
            
        }
       
        
    }while(a);
    
    
}
int do_enablearc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    char * ep;
    unsigned addr=(unsigned)simple_strtoul (argv[1], &ep, 16);
    printf("Boot ARC From 0x%x\n",addr);
    enable_arc(addr);
    __asm__ __volatile__("wfi");
	return 0;
}

U_BOOT_CMD(
	arc,	2,	1,	do_enablearc,
	"Enable ARC CPU",
	"addr\n"
	"    - enable ARC CPU"
);
#endif

int run_arc(unsigned addr)
{
	char c;
    /** copy ARM code*/
    //change arm mapping
    memcpy(0x49008000,0x49000000,16*1024);
 		//remap arm memory
    writel((0x49008000>>14)&0xf,P_AO_REMAP_REG0);
    /** copy ARC code*/
    //copy code to 49000000 and remap to zero
    memcpy(0x49008000,addr,16*1024);
    writel(0x1<<4,P_AO_REMAP_REG1);
    
    writel(0x7fffffff,P_AO_RTI_STATUS_REG0);
    printf("start up ARC\n");
    //switch to ARC jtag
 //   writel(0x51001,0xc8100030);
    
    writel(1,P_AO_RTI_STATUS_REG1);
        
    //reset arc
    writel(RESET_ARC625,CBUS_REG_ADDR(RESET2_REGISTER));
    __udelay(1000);
    
   	//enable arc
    writel(1,CBUS_REG_ADDR(MEDIA_CPU_CTL));
    writel(0,CBUS_REG_ADDR(MEDIA_CPU_CTL)); 
    
    unsigned a,b;
    unsigned timer_base;
    a=b=0x7fffffff;
    printf("ARM is Live\n");
    timer_base=get_timer(0);
    do{
        a=readl(P_AO_RTI_STATUS_REG0);
        if((a&0x80000000)|| ((a==b)&&(get_timer(timer_base)<10000000)))
        {
            continue;
        }
        timer_base=get_timer(0);
        b=a;
        printf("ARM is Live: %x",a);
        switch(a&0xffff)
        {
            case 0: 
                printf("ARM Exit Sleep Mode\n");
                break;
            
        }
    }while(a);
       
     return 0;
}

int do_arc (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned	addr, rc;
	int     rcode = 0;

	if (argc < 2)
		return cmd_usage(cmdtp);

	addr = simple_strtoul(argv[1], NULL, 16);

	printf ("## Starting ARC application at 0x%08lX ...\n", addr);

	rc = run_arc(addr);

	printf ("## Application terminated, rc = 0x%lX\n", rc);
	return rcode;
}

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
  arc, CONFIG_SYS_MAXARGS, 1,	do_arc,
	"start ARC application at address 'addr'",
	"addr [arg ...]\n    - start ARC application at address 'addr'\n"
	"      passing 'arg' as arguments"
);


/* --------------------------------------------------------------------- */
inline static void rbt_save_env()
{		
	  /* save return pc */
	  asm volatile ("ldr r0,=0x4900EF38");
	  asm volatile ("str lr,[r0]");
	  /* save vector base addr */
	  asm volatile ("mrc p15,0,r1,c12,c0,0");
	  asm volatile ("str r1,[r0,#4]");
	  /* save monitor vector base addr */
	  asm volatile ("mrc p15,0,r1,c12,c0,1");
	  asm volatile ("str r1,[r0,#8]");
}

inline static void rbt_save_sp()
{
 	asm volatile ("ldr r0,=0x4900EF00");
	asm volatile ("str sp, [r0]");
	
 // save cache status
	asm volatile ("mrc p15,0,r1,c1,c0,0");
	asm volatile ("str r1,[r0,#4]");
	
	// save mmu table
	asm volatile ("mrc p15,0,r1,c2,c0,0");
	asm volatile ("str r1,[r0,#8]");
}

inline static void rbt_save_regs()
{	/* save general registers */
	asm volatile ("ldr r0,=0x4900EF0c");
	asm volatile ("str r2, [r0]");
	asm volatile ("str r3, [r0,#4]");
	asm volatile ("str r4, [r0,#8]");
	asm volatile ("str r5, [r0,#12]");
	asm volatile ("str r6, [r0,#16]");
	asm volatile ("str r7, [r0,#20]");
	asm volatile ("str r8, [r0,#24]");
	asm volatile ("str r9, [r0,#28]");
	asm volatile ("str r10, [r0,#32]");
	asm volatile ("str r11, [r0,#36]");
	asm volatile ("str r12, [r0,#40]");
}

int run_testpd(unsigned addr)
{
	  char cmd;
	  
		rbt_save_env();
		
    /** copy ARM code*/
    //change arm mapping
    memcpy(0x49008000,0x49000000,16*1024);
    
 		//remap arm memory
    writel((0x49008000>>14)&0xf,P_AO_REMAP_REG0);
    
    /** copy ARC code*/
    //copy code to 49000000 and remap to zero
    memcpy(0x49008000,addr,16*1024);
    writel(0x1<<4,P_AO_REMAP_REG1);
    
    printf("start up ARC\n");
   
    //switch to ARC jtag
 //   writel(0x51001,0xc8100030);
        
    //reset arc
    writel(RESET_ARC625,CBUS_REG_ADDR(RESET2_REGISTER));
    __udelay(1000);
    
   	//enable arc
    writel(1,CBUS_REG_ADDR(MEDIA_CPU_CTL));
    writel(0,CBUS_REG_ADDR(MEDIA_CPU_CTL)); 
    
    do{
     		printf("cmd >");
    		cmd = getc();
				writel((unsigned)cmd,P_AO_RTI_STATUS_REG0);
				__udelay(1000);
				__udelay(1000);		
				if(cmd == 't'){
					asm volatile ("wfi");
				}
				else{
    				while(readl(P_AO_RTI_STATUS_REG0) != 0)
    				{
    					__udelay(1000);
    				}
 				}			    
    }while(cmd != 'q');
    return 0;
}

int do_testpd (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned	addr, rc;
	int     rcode = 0;
	int i = 0;
	

	if (argc < 2)
		return cmd_usage(cmdtp);
		
	rbt_save_sp();
#ifndef CONFIG_DCACHE_OFF
	  dcache_disable();
	  asm volatile ("mcr p15, 0, %0, c7, c5, 0": :"r" (0));
#endif	  
	addr = simple_strtoul(argv[1], NULL, 16);

	printf ("## Starting ARC application at 0x%08lX ...\n", addr);

	/* save stack pointer */
	rbt_save_regs();
	
 	rc = run_testpd(addr);
	
	//restore AHB SDRAM address map
	writel(0,P_AHB_ARBDEC_REG);
//writel(0,P_AO_REMAP_REG0); //needn't restore the map

#ifndef CONFIG_DCACHE_OFF
	  dcache_flush();
#endif	
    for(i = 0; i < 100000; i++){
    } 

	return rcode;
}
/* --------------------------------------------------------------------- */

U_BOOT_CMD(
	testpd, CONFIG_SYS_MAXARGS, 1,	do_testpd,
	"test power down mode, ARC application at address 'addr'",
	"addr [arg ...]\n    - start ARC application at address 'addr'\n"
	"      passing 'arg' as arguments"
);