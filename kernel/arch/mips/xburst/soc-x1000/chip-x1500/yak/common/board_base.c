#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/input.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/interrupt.h>
#include <linux/jz_dwc.h>
#include <linux/delay.h>
#include <mach/jzsnd.h>
#include <mach/platform.h>
#include <mach/jzfb.h>
#include <mach/jzmmc.h>
#include <mach/jzssi.h>
#include <gpio.h>
#include <mach/jz_efuse.h>
#include "board_base.h"


struct jz_platform_device
{
	struct platform_device *pdevices;
	void *pdata;
	int size;
};

static struct jz_platform_device platform_devices_array[] __initdata = {
#define DEF_DEVICE(DEVICE, DATA, SIZE)  \
	{ .pdevices = DEVICE,   \
		.pdata = DATA, .size = SIZE,}

#ifdef CONFIG_KEYBOARD_GPIO
	DEF_DEVICE(&jz_button_device, 0, 0),
#endif

#ifdef CONFIG_LEDS_GPIO
	DEF_DEVICE(&jz_led_rgb, 0, 0),
#endif

#ifdef CONFIG_SERIAL_JZ47XX_UART0
	DEF_DEVICE(&jz_uart0_device, 0, 0),
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART1
	DEF_DEVICE(&jz_uart1_device, 0, 0),
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART2
	DEF_DEVICE(&jz_uart2_device, 0, 0),
#endif
#ifdef CONFIG_JZMMC_V11_MMC1
	DEF_DEVICE(&jz_msc1_device,&tf_pdata,sizeof(struct jzmmc_platform_data)),
#endif
#ifdef CONFIG_JZMMC_V12_MMC0
	DEF_DEVICE(&jz_msc0_device,&tf_pdata,sizeof(struct jzmmc_platform_data)),
#endif
#ifdef CONFIG_JZMMC_V12_MMC1
	DEF_DEVICE(&jz_msc1_device,&sdio_pdata,sizeof(struct jzmmc_platform_data)),
#endif
#ifdef CONFIG_JZMMC_V11_MMC2
	DEF_DEVICE(&jz_msc2_device,&sdio_pdata,sizeof(struct jzmmc_platform_data)),
#endif

#ifdef CONFIG_BROADCOM_RFKILL
	DEF_DEVICE(&bt_power_device,0,0),
	DEF_DEVICE(&bluesleep_device,0,0),
#endif

#ifdef CONFIG_BCM_AP6212_RFKILL
	DEF_DEVICE(&bt_power_device,0,0),
#endif

#ifdef CONFIG_BCM_43438_RFKILL
	DEF_DEVICE(&bt_power_device,0,0),
#endif

#ifdef CONFIG_USB_JZ_DWC2
	DEF_DEVICE(&jz_dwc_otg_device,0,0),
#endif

#ifdef CONFIG_JZ_PWM
       DEF_DEVICE(&jz_pwm_device, 0, 0),
#endif

#ifdef CONFIG_XBURST_DMAC_V13
	DEF_DEVICE(&jz_pdma_device, 0, 0),
#endif
#ifdef CONFIG_XBURST_DMAC
	DEF_DEVICE(&jz_pdma_device, 0, 0),
#endif

#ifdef CONFIG_USB_OHCI_HCD
	DEF_DEVICE(&jz_ohci_device,0,0),
#endif

#ifdef CONFIG_SPI_GPIO
	DEF_DEVICE(&jz_spi_gpio_device, 0,0),
#endif

#ifdef CONFIG_JZ_SPI0
	DEF_DEVICE(&jz_ssi0_device, &spi0_info_cfg, sizeof(struct jz_spi_info)),
#endif

#ifdef CONFIG_RTC_DRV_JZ
	DEF_DEVICE(&jz_rtc_device, 0, 0),
#endif
#ifdef CONFIG_JZ_EFUSE_V13
	DEF_DEVICE(&jz_efuse_device, &jz_efuse_pdata, sizeof(struct jz_efuse_platform_data)),
#endif
#ifdef	CONFIG_JZ_SFC
	DEF_DEVICE(&jz_sfc_device,&sfc_info_cfg, sizeof(struct jz_sfc_info)),
#endif
#ifdef CONFIG_SOFT_I2C0_GPIO_V12_JZ
	DEF_DEVICE(&i2c0_gpio_device, 0, 0),
#endif
#ifdef CONFIG_SOFT_I2C1_GPIO_V12_JZ
	DEF_DEVICE(&i2c1_gpio_device, 0, 0),
#endif
#ifdef	CONFIG_I2C0_V12_JZ
	DEF_DEVICE(&jz_i2c0_device,0,0),
#endif
#ifdef	CONFIG_I2C1_V12_JZ
	DEF_DEVICE(&jz_i2c1_device,0,0),
#endif
#ifdef	CONFIG_I2C2_V12_JZ
	DEF_DEVICE(&jz_i2c2_device,0,0),
#endif
#ifdef	CONFIG_MFD_JZ_SADC_V12
	DEF_DEVICE(&jz_adc_device, 0, 0),
#endif
#ifdef	CONFIG_JZ_VPU_V13
	DEF_DEVICE(&jz_vpu_device, 0, 0),
#endif


#ifdef	CONFIG_I2C0_DMA_V12
	DEF_DEVICE(&jz_i2c0_dma_device, 0, 0),
#endif
#ifdef	CONFIG_I2C1_DMA_V12
	DEF_DEVICE(&jz_i2c1_dma_device, 0, 0),
#endif
#ifdef	CONFIG_I2C2_DMA_V12
	DEF_DEVICE(&jz_i2c2_dma_device, 0, 0),
#endif
#ifdef CONFIG_JZ_PWM_GENERIC
	DEF_DEVICE(&jz_pwm_devs_platform_device, 0, 0),
#endif


};

static int __init board_base_init(void)
{
	int pdevices_array_size, i;


	pdevices_array_size = ARRAY_SIZE(platform_devices_array);
	for(i = 0; i < pdevices_array_size; i++) {
		if(platform_devices_array[i].size)
			platform_device_add_data(platform_devices_array[i].pdevices,
					platform_devices_array[i].pdata, platform_devices_array[i].size);
		platform_device_register(platform_devices_array[i].pdevices);
	}

#if defined(CONFIG_JZ_SPI0) || defined(CONFIG_SPI_GPIO)
	spi_register_board_info(jz_spi0_board_info, jz_spi0_devs_size);
#endif

#if (defined(CONFIG_SOFT_I2C2_GPIO_V12_JZ) || defined(CONFIG_I2C2_V12_JZ))
     i2c_register_board_info(2, jz_i2c2_devs, jz_i2c2_devs_size);
#endif

#if (defined(CONFIG_SOFT_I2C0_GPIO_V12_JZ) || defined(CONFIG_I2C0_V12_JZ))
     i2c_register_board_info(0, jz_i2c0_devs, jz_i2c0_devs_size);
#endif


	return 0;
}


/*
 *  * Called by arch/mips/kernel/proc.c when 'cat /proc/cpuinfo'.
 *   * Android requires the 'Hardware:' field in cpuinfo to setup the init.%hardware%.rc.
 *    */
const char *get_board_type(void)
{
	return CONFIG_PRODUCT_NAME;
}


arch_initcall(board_base_init);
