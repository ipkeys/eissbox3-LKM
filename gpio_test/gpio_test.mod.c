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
	{ 0xc1514a3b, "free_irq" },
	{ 0xc3869d44, "gpiod_unexport" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0xe44751b, "gpiod_to_irq" },
	{ 0x1ab1341d, "gpiod_set_debounce" },
	{ 0xdfc74a45, "gpiod_direction_input" },
	{ 0xf54bbc25, "gpiod_export" },
	{ 0x6bac0ba8, "gpiod_direction_output_raw" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xc5850110, "printk" },
	{ 0xffc7a944, "gpiod_get_raw_value" },
	{ 0xa9edd399, "gpiod_set_raw_value" },
	{ 0x9a60200, "gpio_to_desc" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "2C01FB0C54BBE121E760C12");
