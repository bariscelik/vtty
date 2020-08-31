#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/serial.h>
#include <linux/sched/signal.h>
#include <linux/seq_file.h>
#include <linux/tty_flip.h>
#include "chip.h"

#define M_AUTHOR "Barış Çelik"
#define M_DESCRIPTION "Virtual TTY driver for CROSS Zlín, Inc."
#define M_VERSION "1.0"
#define M_LICENSE "GPL"
#define M_DEV_NAME "cross_virtual_tty"
#define M_DEV_MAJOR		232
#define M_DEV_PREFIX "ttyv"
#define M_TAG "SHATTY"

// printk with tag prefix
#define VTTY_PRINT(level,message) printk(level M_TAG ": " message)
#define VTTY_PRINTF(level,message,...) printk(level M_TAG ": " message, __VA_ARGS__)

MODULE_AUTHOR(M_AUTHOR);
MODULE_DESCRIPTION(M_DESCRIPTION);
MODULE_LICENSE(M_LICENSE);

#define SHA_DIGEST_SIZE 32 // length of the sha256 digest
#define SHA_DIGEST_HD_SIZE SHA_DIGEST_SIZE * 2 // for HexaDecimal output

#define VTTY_MAX_ROOM 4096

static struct tty_struct *vtty;
static struct tty_port vtty_port;

// allow only one port at a time
static bool port_is_opened = false;

// cursor for parsing text input
static int cursor = 0;

// line buffer
unsigned char *line_buff = NULL;

/**
 * calls when there is an attempt to open a port
 * @param tty
 * @param file
 * @return
 */
static int vtty_open(struct tty_struct *tty, struct file *file)
{

    if (vtty == NULL) {
        vtty = kmalloc(sizeof(*vtty), GFP_KERNEL);

        if (!vtty)
            return -ENOMEM;
    }else{
        // reject second connection attempt
        if(port_is_opened)
            return -EBUSY;
    }

    port_is_opened = true;

    return tty_port_open(&vtty_port, tty, file);
}
/**
 * calls when closing a port
 * @param tty
 * @param file
 */
static void vtty_close(struct tty_struct *tty, struct file *file)
{
    if (unlikely(!tty)) {
        VTTY_PRINT(KERN_ERR, "Invalid TTY\n");
        return;
    }

    if (unlikely(tty->count > 1))
        return;

    if (unlikely(!tty->port)) {
        VTTY_PRINT(KERN_ERR, "Port not found\n");
        return;
    }

    tty_port_close(tty->port,tty,file);

    // the port was closed
    port_is_opened = false;
}

/**
 * Generates and sends hash to related port
 * @param digest generated hash in form of binary
 * @param digest_hd converted hash into hexadecimal for represent
 */
static void vtty_generate_digest(void)
{
    size_t i;

    unsigned char digest[SHA_DIGEST_SIZE];
    unsigned char digest_hd[SHA_DIGEST_HD_SIZE+2];

    int hash = generate_hash(line_buff, cursor, digest);

    if(hash!=0)
    {
        VTTY_PRINT(KERN_ERR, "Failed to generate hash\n");
        goto release;
    }

    // convert binary into hexadecimal
    for (i = 0; i < SHA_DIGEST_SIZE; i++)
        sprintf(&digest_hd[i*2], "%02x", digest[i]);

    // append delimiters
    digest_hd[SHA_DIGEST_HD_SIZE] = '\r';
    digest_hd[SHA_DIGEST_HD_SIZE+1] = '\n';

    // transmit to vtty_port
    if (tty_buffer_request_room(&vtty_port, SHA_DIGEST_HD_SIZE + 2))
        tty_flip_buffer_push(&vtty_port);
    tty_insert_flip_string(&vtty_port, digest_hd, SHA_DIGEST_HD_SIZE + 2);

    tty_flip_buffer_push(&vtty_port);

    release:
    // release buffer
    kfree(line_buff);
    line_buff = NULL;
    cursor = 0;

}

/**
 * Invokes when the data are available
 * @param tty
 * @param buffer
 * @param count
 * @return
 */
static int vtty_write(struct tty_struct *tty,
                      const unsigned char *buffer, int count)
{
    int i;

    if (line_buff == NULL) {

        // initialize line buffer
        line_buff = kmalloc(count + 1, GFP_KERNEL);

        if (!line_buff)
            return -ENOMEM;
    }else{

        // resize buffer
        line_buff = krealloc(line_buff, cursor + count + 1, GFP_KERNEL);

        if (!line_buff)
            return -ENOMEM;
    }

    if (!vtty)
        return -ENODEV;

    for (i = 0; i < count; ++i)
    {
        // check all characters in buffer to find NL
        if(buffer[i] == '\n')
            vtty_generate_digest();
        else{
            line_buff[cursor] = buffer[i];
            cursor++;
        }
    }

    return count;
}

static int vtty_write_room(struct tty_struct *tty)
{
    if (!vtty)
        return -ENODEV;

    if (unlikely(!tty->port)) {
        VTTY_PRINT(KERN_ERR, "Port not found\n");
        return -EINVAL;
    }

    return VTTY_MAX_ROOM;

}
static const struct tty_operations serial_ops = {
        .open = vtty_open,
        .close = vtty_close,
        .write = vtty_write,
        .write_room = vtty_write_room,
};
static const struct tty_port_operations null_ops = { };
static struct tty_driver *vtty_driver;

static int __init tiny_init(void)
{
    int retval;

    // allocate the tty driver
    vtty_driver = alloc_tty_driver(1);

    if (!vtty_driver)
        return -ENOMEM;

    tty_port_init(&vtty_port);

    vtty_port.ops = &null_ops;

    // initialize the tty driver
    vtty_driver->owner = THIS_MODULE;
    vtty_driver->driver_name = M_DEV_NAME;
    vtty_driver->name = M_DEV_PREFIX;
    vtty_driver->major = M_DEV_MAJOR,
            vtty_driver->type = TTY_DRIVER_TYPE_SERIAL,
            vtty_driver->subtype = SERIAL_TYPE_NORMAL,
            vtty_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV,
            vtty_driver->init_termios = tty_std_termios;
    vtty_driver->init_termios.c_cflag = B115200 | CS8 | CREAD | HUPCL | CLOCAL;
    tty_set_operations(vtty_driver, &serial_ops);

    tty_port_link_device(&vtty_port, vtty_driver, 0);

    // register the tty driver
    retval = tty_register_driver(vtty_driver);
    if (retval) {
        VTTY_PRINT(KERN_DEBUG, "failed to vtty driver register");
        put_tty_driver(vtty_driver);
        return retval;
    }

    tty_register_device(vtty_driver, 0, NULL);

    VTTY_PRINT(KERN_INFO, M_DESCRIPTION " " M_VERSION);

    return retval;
}

static void __exit tiny_exit(void)
{
    int err=0;

    tty_unregister_device(vtty_driver, 0);

    err = tty_unregister_driver(vtty_driver);
    put_tty_driver(vtty_driver);

    if (vtty)
        kfree(vtty);
}

module_init(tiny_init);
module_exit(tiny_exit);