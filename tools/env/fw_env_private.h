/*
 * (C) Copyright 2002-2008
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* Pull in the current config to define the default environment */
#include <linux/kconfig.h>

#ifndef __ASSEMBLY__
#define __ASSEMBLY__ /* get only #defines from config.h */
#include <config.h>
#undef	__ASSEMBLY__
#else
#include <config.h>
#endif

/*
 * To build the utility with the static configuration
 * comment out the next line.
 * See included "fw_env.config" sample file
 * for notes on configuration.
 */
#define CONFIG_FILE     "/etc/fw_env.config"

#ifndef CONFIG_FILE
#define DEVICE1_NAME	"/dev/mtd1"
#define DEVICE1_OFFSET    0x00000
#define ENV1_SIZE         0x20000
#define DEVICE1_ESIZE     0x20000
#define DEVICE1_ENVSECTORS     2


//#define HAVE_REDUND
#define DEVICE2_NAME	"/dev/mtd2"
#define DEVICE2_OFFSET    0x0000
#define ENV2_SIZE         0x4000
#define DEVICE2_ESIZE     0x4000
#define DEVICE2_ENVSECTORS     2

#endif

#ifndef CONFIG_BAUDRATE
#define CONFIG_BAUDRATE		115200
#endif

#ifndef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY	2	/* autoboot after 5 seconds	*/
#endif

#ifndef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND						\
	"bootp; "							\
	"setenv bootargs root=/dev/nfs nfsroot=${serverip}:${rootpath} "\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; "\
	"bootm"
#endif

#define DFU_ALT_INFO_RAM \
                "dfu_ram_info=" \
        "setenv dfu_alt_info " \
        "image.ub ram $netstart 0x1e00000\0" \
        "dfu_ram=run dfu_ram_info && dfu 0 ram 0\0" \
        "thor_ram=run dfu_ram_info && thordown 0 ram 0\0"

#define DFU_ALT_INFO_MMC \
        "dfu_mmc_info=" \
        "set dfu_alt_info " \
        "${kernel_image} fat 0 1\\\\;" \
        "dfu_mmc=run dfu_mmc_info && dfu 0 mmc 0\0" \
        "thor_mmc=run dfu_mmc_info && thordown 0 mmc 0\0"

#define KLANG_FAILSAFE \
    "resetmio=49\0" \
    "sw_reset=0\0" \
    "setenvmount=envmount=mmc; " \
        "envdev=0; " \
        "envpartid=1; " \
	"setenv bootargs root=/dev/mmcblk0p2 rw rootwait earlyprintk ethaddr=${ethaddr}; " \
        "if test ${sw_reset} -eq 1; then " \
            "echo Software update process started;" \
            "setenv kernel_img failsafe.ub; " \
            "setenv bootargs root=/dev/mmcblk0p3 rw rootwait earlyprintk; " \
            "setenv sw_reset 0; saveenv;" \
        "fi;" \
        "if gpio input ${resetmio} ; then " \
	    "mw 42010004 C0000fff; mw 42010008 C0000fff; mw 4201000C C0000fff; " \
            "setenv kernel_img failsafe.ub; " \
            "setenv bootargs ; " \
            "envmount=usb; " \
	    "echo \"Search for USB failsafe\"; " \
	    "usb start; " \
	    "usb reset; " \
            "run searchenvusb; " \
            "if test ${failsafefound} -eq 0; then " \
                "envmount=mmc; envdev=0; envpartid=0; " \
                "setenv bootargs root=/dev/mmcblk0p3 rw rootwait earlyprintk; " \
            "fi;" \
        "fi;" \
        "if test ${modeboot} = \"sdboot\"; then " \
            "setenv kernel_img failsafe.ub; " \
            "setenv bootargs root=/dev/mmcblk0p3 rw rootwait earlyprintk; " \
        "fi; \0" \
    "searchenvusb=if test ${kernel_img} = \"failsafe.ub\"; then " \
        "failsafefound=0; " \
	"for try in 1 2 3; do " \
	    "if test ${failsafefound} -eq 0; then " \
                "usb reset; " \
		"for sd in 0 1 2 3 4; do " \
		    "echo \"Searching on USB ${sd}:1\"; " \
		    "if test ${failsafefound} -eq 0; then " \
			"if fatload ${envmount} ${envdev}:1 ${netstart} ${kernel_img}; then " \
			    "failsafefound=1; " \
                            "envdev=${sd}; " \
			    "echo \"Failsafe found on USB device ${sd}\"; " \
			"fi; " \
		    "fi; " \
		"done; " \
	    "fi; " \
            "if test ${failsafefound} -eq 0; then " \
    	        "sleep 3; " \
	    "fi; " \
	 "done;fi;\0"

#define UBOOT_ENV_HANDLING \
    "importenv=echo \"Importing additional environment...\"; env import -t ${loadbootenv_addr} $filesize\0" \
    "loadenv=load ${envmount} ${envdev}:${envpartid} ${loadootenv_addr} ${bootenv}\0" \
    "eraseenv=sf probe 0 && sf erase ${bootenvstart} ${bootenvsize}\0"

#define PSSERIAL0	"psserial0=setenv stdout ttyPS0;setenv stdin ttyPS0\0"
#define SERIAL_MULTI	"serial=setenv stdout serial;setenv stdin serial\0"
#define CONSOLE_ARG	"console=console=ttyPS0,115200\0"

/* Extra U-Boot Env settings */
#define CONFIG_EXTRA_ENV_SETTINGS \
	SERIAL_MULTI \
	CONSOLE_ARG \
	DFU_ALT_INFO_RAM \
	DFU_ALT_INFO_MMC \
	PSSERIAL0 \
	"nc=setenv stdout nc;setenv stdin nc;\0" \
	"bootenv=UPDATE.BIN\0" \
	"importbootenv=echo \"Importing environment from SD ...\"; " \
		"env import -t ${loadbootenv_addr} $filesize\0" \
	"loadbootenv=load mmc $sdbootdev:$partid ${loadbootenv_addr} ${bootenv}\0" \
	"sd_uEnvtxt_existence_test=test -e ${envmount} ${envdev}:${envpartid} /${bootenv}\0" \
	"uenvboot=" \
		"if run sd_uEnvtxt_existence_test; then " \
			"run loadbootenv; " \
			"echo Loaded environment from ${bootenv}; " \
			"run importbootenv; " \
			"fi; " \
		"if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...; " \
			"run uenvcmd; " \
		"fi\0" \
	"autoload=no\0" \
	"sdbootdev=0\0" \
	"clobstart=0x10000000\0" \
	"netstart=0x10000000\0" \
	"dtbnetstart=0x23fff000\0" \
	"loadaddr=0x10000000\0" \
	"bootsize=0x500000\0" \
	"bootstart=0x0\0" \
	"bootdelay=1\0" \
	"ethaddr=00:0a:0a:0a:66:66\0" \
	"boot_img=BOOT.BIN\0" \
	"load_boot=tftpboot ${clobstart} ${boot_img}\0" \
	"update_boot=setenv img boot; setenv psize ${bootsize}; setenv installcmd \"install_boot\"; run load_boot test_img; setenv img; setenv psize; setenv installcmd\0" \
	"sd_update_boot=echo Updating boot from SD; fatload ${envmount} ${envdev}:${envpartid} ${clobstart} ${boot_img} && run install_boot\0" \
	"install_boot=sf probe 0 && sf erase ${bootstart} ${bootsize} && " \
		"sf write ${clobstart} ${bootstart} ${filesize}\0" \
	"bootenvsize=0x20000\0" \
	"bootenvstart=0x500000\0" \
	"eraseenv=sf probe 0 && sf erase ${bootenvstart} ${bootenvsize}\0" \
	"jffs2_img=rootfs.jffs2\0" \
	"load_jffs2=tftpboot ${clobstart} ${jffs2_img}\0" \
	"update_jffs2=setenv img jffs2; setenv psize ${jffs2size}; setenv installcmd \"install_jffs2\"; run load_jffs2 test_img; setenv img; setenv psize; setenv installcmd\0" \
	"sd_update_jffs2=echo Updating jffs2; fatload ${envmount} ${envdev}:${envpartid} ${clobstart} ${jffs2_img} && run install_jffs2\0" \
	"install_jffs2=sf probe 0 && sf erase ${jffs2start} ${jffs2size} && " \
		"sf write ${clobstart} ${jffs2start} ${filesize}\0" \
	"kernel_img=image.ub\0" \
	"load_kernel=tftpboot ${clobstart} ${kernel_img}\0" \
	"update_kernel=setenv img kernel; setenv psize ${kernelsize}; setenv installcmd \"install_kernel\"; run load_kernel ${installcmd}; setenv img; setenv psize; setenv installcmd\0" \
	"install_kernel=mmcinfo && fatwrite mmc ${sdbootdev} ${clobstart} ${kernel_img} ${filesize}\0" \
	"cp_kernel2ram=fatload ${envmount} ${envdev}:1 ${netstart} ${kernel_img}\0" \
	"dtb_img=system.dtb\0" \
	"load_dtb=tftpboot ${clobstart} ${dtb_img}\0" \
	"update_dtb=setenv img dtb; setenv psize ${dtbsize}; setenv installcmd \"install_dtb\"; run load_dtb test_img; setenv img; setenv psize; setenv installcmd\0" \
	"sd_update_dtb=echo Updating dtb; fatload ${envmount} ${envdev}:${envpartid} ${clobstart} ${dtb_img} && run install_dtb\0" \
	"loadbootenv_addr=0x00100000\0" \
	"fault=echo ${img} image size is greater than allocated place - partition ${img} is NOT UPDATED\0" \
	"test_crc=if imi ${clobstart}; then run test_img; else echo ${img} Bad CRC - ${img} is NOT UPDATED; fi\0" \
	"test_img=setenv var \"if test ${filesize} -gt ${psize}\\; then run fault\\; else run ${installcmd}\\; fi\"; run var; setenv var\0" \
	"netboot=tftpboot ${netstart} ${kernel_img} && bootm\0" \
	"default_bootcmd=run setenvmount; run uenvboot; run cp_kernel2ram; bootm ${netstart}\0" \
    	"cp_macaddr=env import -t 0xFFFFFC00\0" \
        KLANG_FAILSAFE \
""
