#include "norflash.h"
#include <string.h>

xdata u8 norflash_cmd[8];

// 写入到norflash中
void Nor_Flash_write(u32 addr, u8 *buff, u16 len)
{
	write_dgus_vp(CACHE_ADDR, buff, len); // 将数据从缓冲区写到CACHE_ADDR中
	norflash_cmd[0] = NORFLASH_WRITE;	  // 操作模式

	norflash_cmd[1] = (u8)(addr >> 16); // norflash地址
	norflash_cmd[2] = (u8)(addr >> 8);
	norflash_cmd[3] = (u8)addr;

	norflash_cmd[4] = (u8)(CACHE_ADDR >> 8); // dugs变量数据地址
	norflash_cmd[5] = (u8)CACHE_ADDR;

	norflash_cmd[6] = (u8)(len >> 8); // 读写字节长度
	norflash_cmd[7] = (u8)(len);

	write_dgus_vp(NORFLASH_ADDR, norflash_cmd, 4); // 写入系统变量接口地址0x0008中
	while (1)									   // 等待清零
	{
		read_dgus_vp(NORFLASH_ADDR, norflash_cmd, 2); // 读取操作模式位 0x5A=读 0xA5=写， CPU 操作完清零。
		if (norflash_cmd[0] == 0)					  // 清零
		{
			break;
		}
		delay_ms(1);
	}
}
// 从norflash中读取

void Nor_Flash_read(u32 addr, u8 *buff, u16 len)
{
	norflash_cmd[0] = NORFLASH_READ; // 操作模式

	norflash_cmd[1] = (u8)(addr >> 16); // norflash地址
	norflash_cmd[2] = (u8)(addr >> 8);
	norflash_cmd[3] = (u8)addr;

	norflash_cmd[4] = (u8)(CACHE_ADDR >> 8); // dugs变量数据地址
	norflash_cmd[5] = (u8)CACHE_ADDR;

	norflash_cmd[6] = (u8)(len >> 8); // 读写字节长度
	norflash_cmd[7] = (u8)(len);

	write_dgus_vp(NORFLASH_ADDR, norflash_cmd, 4);
	while (1)
	{
		read_dgus_vp(NORFLASH_ADDR, norflash_cmd, 2);
		if (norflash_cmd[0] == 0)
		{
			break;
		}
		delay_ms(1);
	}
	read_dgus_vp(CACHE_ADDR, buff, len); // 将数据从CACHE_ADDR中读到buff中
}
