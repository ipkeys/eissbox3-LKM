#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0x1bf8c29, "module_layout" },
	{ 0xfe990052, "gpio_free" },
	{ 0xc3869d44, "gpiod_unexport" },
	{ 0xf54bbc25, "gpiod_export" },
	{ 0x6bac0ba8, "gpiod_direction_output_raw" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xedbfbd3f, "kobject_put" },
	{ 0x8b3faaa4, "sysfs_create_group" },
	{ 0x14798c46, "kobject_create_and_add" },
	{ 0x81528894, "kernel_kobj" },
	{ 0xc5850110, "printk" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xa9edd399, "gpiod_set_raw_value" },
	{ 0x9a60200, "gpio_to_desc" },
	{ 0xbcab6ee6, "sscanf" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "6B70EFACD8E1F15934581BB");
