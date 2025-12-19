/**
 ****************************************************************************************************
 * @file        lcd_ex.c
 * @author      ÕýµãÔ­×ÓÍÅ¶Ó(ALIENTEK)
 * @version     V1.1
 * @date        2023-05-29
 * @brief       lcd_ex.c´æ·Å¸÷¸öLCDÇý¶¯ICµÄ¼Ä´æÆ÷³õÊ¼»¯²¿·Ö´úÂë,ÒÔ¼ò»¯lcd.c,¸Ã.cÎÄ¼þ
 *              ²»Ö±½Ó¼ÓÈëµ½¹¤³ÌÀïÃæ,Ö»ÓÐlcd.c»áÓÃµ½,ËùÒÔÍ¨¹ýincludeµÄÐÎÊ½Ìí¼Ó.(²»ÒªÔÚ
 *              ÆäËûÎÄ¼þÔÙ°üº¬¸Ã.cÎÄ¼þ!!·ñÔò»á±¨´í!) 
 * 
 * @license     Copyright (c) 2020-2032, ¹ãÖÝÊÐÐÇÒíµç×Ó¿Æ¼¼ÓÐÏÞ¹«Ë¾
 ****************************************************************************************************
 * @attention
 *
 * ÊµÑéÆ½Ì¨:ÕýµãÔ­×Ó Ì½Ë÷Õß F407¿ª·¢°å
 * ÔÚÏßÊÓÆµ:www.yuanzige.com
 * ¼¼ÊõÂÛÌ³:www.openedv.com
 * ¹«Ë¾ÍøÖ·:www.alientek.com
 * ¹ºÂòµØÖ·:openedv.taobao.com
 *
 * ÐÞ¸ÄËµÃ÷
 * V1.0 20211016
 * µÚÒ»´Î·¢²¼
 * V1.1 20230529
 * 1£¬ÐÂÔö¶ÔST7796ºÍILI9806 ICÖ§³Ö
 ****************************************************************************************************
 */

#include "lcd.h"



/**
 * @brief       ST7789 �0�4�0�2�0�7�0�3�0�4�0�6�0�1�0�8�0�4�0�3�0�4�0�7���0�0�0�5
 * @param       �0�2�0�7
 * @retval      �0�2�0�7
 */
void lcd_ex_st7789_reginit(void)
{
    lcd_wr_regno(0x11);

    HAL_Delay(120);

    lcd_wr_regno(0x36);
    lcd_wr_data(0x00);

    lcd_wr_regno(0x3A);
    lcd_wr_data(0x05);

    lcd_wr_regno(0xB2);
    lcd_wr_data(0x0C);
    lcd_wr_data(0x0C);
    lcd_wr_data(0x00);
    lcd_wr_data(0x33);
    lcd_wr_data(0x33);

    lcd_wr_regno(0xB7);
    lcd_wr_data(0x35);

    lcd_wr_regno(0xBB); /* vcom */
    lcd_wr_data(0x32);  /* 30 */

    lcd_wr_regno(0xC0);
    lcd_wr_data(0x0C);

    lcd_wr_regno(0xC2);
    lcd_wr_data(0x01);

    lcd_wr_regno(0xC3); /* vrh */
    lcd_wr_data(0x10);  /* 17 0D */

    lcd_wr_regno(0xC4); /* vdv */
    lcd_wr_data(0x20);  /* 20 */

    lcd_wr_regno(0xC6);
    lcd_wr_data(0x0f);

    lcd_wr_regno(0xD0);
    lcd_wr_data(0xA4); 
    lcd_wr_data(0xA1); 

    lcd_wr_regno(0xE0); /* Set Gamma  */
    lcd_wr_data(0xd0);
    lcd_wr_data(0x00);
    lcd_wr_data(0x02);
    lcd_wr_data(0x07);
    lcd_wr_data(0x0a);
    lcd_wr_data(0x28);
    lcd_wr_data(0x32);
    lcd_wr_data(0x44);
    lcd_wr_data(0x42);
    lcd_wr_data(0x06);
    lcd_wr_data(0x0e);
    lcd_wr_data(0x12);
    lcd_wr_data(0x14);
    lcd_wr_data(0x17);


    lcd_wr_regno(0xE1);  /* Set Gamma */
    lcd_wr_data(0xd0);
    lcd_wr_data(0x00);
    lcd_wr_data(0x02);
    lcd_wr_data(0x07);
    lcd_wr_data(0x0a);
    lcd_wr_data(0x28);
    lcd_wr_data(0x31);
    lcd_wr_data(0x54);
    lcd_wr_data(0x47);
    lcd_wr_data(0x0e);
    lcd_wr_data(0x1c);
    lcd_wr_data(0x17);
    lcd_wr_data(0x1b); 
    lcd_wr_data(0x1e);


    lcd_wr_regno(0x2A);
    lcd_wr_data(0x00);
    lcd_wr_data(0x00);
    lcd_wr_data(0x00);
    lcd_wr_data(0xef);

    lcd_wr_regno(0x2B);
    lcd_wr_data(0x00);
    lcd_wr_data(0x00);
    lcd_wr_data(0x01);
    lcd_wr_data(0x3f);

    lcd_wr_regno(0x29); /* display on */
}