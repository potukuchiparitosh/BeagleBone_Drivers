#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/skbuff.h>

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>


static const struct hci_uart_proto athp = {
	.id		= HCI_UART_HC05,
	.name		= "HC05",
	.manufacturer	= 69,
	.open		= ath_open,
	.close		= ath_close,
	.flush		= ath_flush,
	.setup		= ath_setup,
	.recv		= ath_recv,
	.enqueue	= ath_enqueue,
	.dequeue	= ath_dequeue,
};


int __init hc05_init(void) {
        return hci_uart_register_proto(&hcp);
}

void __exit hc05_exit(void) {
        return hci_uart_unregister_proto(&hcp);
}
