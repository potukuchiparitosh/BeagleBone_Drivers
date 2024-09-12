#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
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
__used
__attribute__((section("__versions"))) = {
	{ 0x516e49f9, "module_layout" },
	{ 0xef34a95d, "driver_unregister" },
	{ 0xfa53f3eb, "__spi_register_driver" },
	{ 0xf1e1e552, "__devm_iio_device_register" },
	{ 0xe0a1cfb8, "spi_sync" },
	{ 0x68f31cbd, "__list_add_valid" },
	{ 0x5f754e5a, "memset" },
	{ 0x136df836, "spi_setup" },
	{ 0x40808e37, "devm_iio_device_alloc" },
	{ 0xdb7305a1, "__stack_chk_fail" },
	{ 0xcab2fae0, "spi_write_then_read" },
	{ 0x7c32d0f0, "printk" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xe707d823, "__aeabi_uidiv" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cbmp280,bmp280_spi");
MODULE_ALIAS("of:N*T*Cbmp280,bmp280_spiC*");
MODULE_ALIAS("spi:bmp280");
