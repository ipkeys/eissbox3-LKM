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
	{ 0x15ee38f7, "class_unregister" },
	{ 0xa332765e, "device_destroy" },
	{ 0xe346f67a, "__mutex_init" },
	{ 0x837c5de5, "class_destroy" },
	{ 0xeedda6b2, "device_create" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x898e229b, "__class_create" },
	{ 0x37907b2c, "__register_chrdev" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0x5f754e5a, "memset" },
	{ 0x2cfde9a2, "warn_slowpath_fmt" },
	{ 0xd9ce8f0c, "strnlen" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xdd4ffa9b, "mutex_trylock" },
	{ 0xc5850110, "printk" },
	{ 0x67ea780, "mutex_unlock" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "B370E03D5261C8627FA701A");
