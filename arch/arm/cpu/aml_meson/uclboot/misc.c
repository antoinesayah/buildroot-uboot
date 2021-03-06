// 
//  misc.c
//  u-boot self-decompress
//  
//  Created by Amlogic on 2009-07-21.
//  Copyright 2009 Amlogic. All rights reserved.
// 

extern int uclDecompress(char* op, unsigned o_len, char* ip);

//extern unsigned TEXT_BASE;
//extern void *UCL_TEXT_BASE;
extern void *input_data;
extern void *input_data_end;
extern void clean_invalidable_cache();
	
void start_arcboot_ucl(void)
{
	typedef void (* JumpAddr)(void);	
	unsigned len ;
	serial_puts("ucl decompress ...... "); //mod by steven
	uclDecompress(TEXT_BASE,&len,&input_data);
    serial_puts("decompress finished\n");
    
    clean_invalidable_cache();
    JumpAddr target=(JumpAddr)(TEXT_BASE);
    target();
		
}
