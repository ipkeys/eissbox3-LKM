/**
 * @file   ebox3driver.c
 * @author Yuriy Kozhynov
 * @date   20 January 2021
 * @brief  A kernel module for controlling relays and meters that is connected to a GPIO.
 * The sysfs entry appears at
 * /sys/ebox3/relays/r1..4
 * /sys/ebox3/meters/m1..6/...
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/kobject.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yuriy Kozhynov <ykozhynov@ipkeys.com>");
MODULE_DESCRIPTION("Driver for EISSbox3");
MODULE_VERSION("0.1");

#define  DEBOUNCE_TIME 80     // The default bounce time - 80ms

#include "ebox3relays.h"
#include "ebox3meter1.h"
#include "ebox3meter2.h"
#include "ebox3meter3.h"
#include "ebox3meter4.h"
#include "ebox3meter5.h"
#include "ebox3meter6.h"

static struct kobject *ebox3_kobj;
static struct kobject *meters_kobj;

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point. In this example this
 *  function sets up the GPIOs and the IRQ
 *  @return returns 0 if successful
 */
static int __init ebox3driver_init(void) {
    int result = 0;
    unsigned long IRQflags = IRQF_TRIGGER_RISING; // The default is a rising-edge interrupt

    printk(KERN_INFO "Ebox3 Driver: Init\n");

    // create the kobject sysfs entry at /sys/ebox3
    ebox3_kobj = kobject_create_and_add("ebox3", kernel_kobj->parent); // kernel_kobj points to /sys/kernel
    if (!ebox3_kobj){
        printk(KERN_ALERT "Ebox3 Driver: failed to create ebox3 kobject mapping\n");
        return -ENOMEM;
    }
    // add the attributes to /sys/ebox3/relays/r1..4
    result = sysfs_create_group(ebox3_kobj, &relays_group);
    if (result) {
        printk(KERN_ALERT "Ebox3 Driver: failed to create sysfs group for relays\n");
        kobject_put(ebox3_kobj);
        return result;
    }

    meters_kobj = kobject_create_and_add("meters", ebox3_kobj);
   
    // add the attributes to /sys/ebox3/meters/m1..6/...
    result = sysfs_create_group(meters_kobj, &meter_group_1);
    if (result) {
        printk(KERN_ALERT "Ebox3 Driver: failed to create sysfs group for meter1\n");
        kobject_put(meters_kobj);
        kobject_put(ebox3_kobj);
        return result;
    }
    result = sysfs_create_group(meters_kobj, &meter_group_2);
    if (result) {
        printk(KERN_ALERT "Ebox3 Driver: failed to create sysfs group for meter2\n");
        kobject_put(meters_kobj);
        kobject_put(ebox3_kobj);
        return result;
    }
    result = sysfs_create_group(meters_kobj, &meter_group_3);
    if (result) {
        printk(KERN_ALERT "Ebox3 Driver: failed to create sysfs group for meter3\n");
        kobject_put(meters_kobj);
        kobject_put(ebox3_kobj);
        return result;
    }
    result = sysfs_create_group(meters_kobj, &meter_group_4);
    if (result) {
        printk(KERN_ALERT "Ebox3 Driver: failed to create sysfs group for meter4\n");
        kobject_put(meters_kobj);
        kobject_put(ebox3_kobj);
        return result;
    }
    result = sysfs_create_group(meters_kobj, &meter_group_5);
    if (result) {
        printk(KERN_ALERT "Ebox3 Driver: failed to create sysfs group for meter5\n");
        kobject_put(meters_kobj);
        kobject_put(ebox3_kobj);
        return result;
    }
    result = sysfs_create_group(meters_kobj, &meter_group_6);
    if (result) {
        printk(KERN_ALERT "Ebox3 Driver: failed to create sysfs group for meter6\n");
        kobject_put(meters_kobj);
        kobject_put(ebox3_kobj);
        return result;
    }

    relays_init();

    result = meter_init_1(IRQflags);
    if (result) {
        printk(KERN_ALERT "Ebox3 Driver: failed to init meter1\n");
        kobject_put(meters_kobj);
        kobject_put(ebox3_kobj);
        return result;
    }
    result = meter_init_2(IRQflags);
    if (result) {
        printk(KERN_ALERT "Ebox3 Driver: failed to init meter2\n");
        kobject_put(meters_kobj);
        kobject_put(ebox3_kobj);
        return result;
    }
    result = meter_init_3(IRQflags);
    if (result) {
        printk(KERN_ALERT "Ebox3 Driver: failed to init meter3\n");
        kobject_put(meters_kobj);
        kobject_put(ebox3_kobj);
        return result;
    }
    result = meter_init_4(IRQflags);
    if (result) {
        printk(KERN_ALERT "Ebox3 Driver: failed to init meter4\n");
        kobject_put(meters_kobj);
        kobject_put(ebox3_kobj);
        return result;
    }
    result = meter_init_5(IRQflags);
    if (result) {
        printk(KERN_ALERT "Ebox3 Driver: failed to init meter5\n");
        kobject_put(meters_kobj);
        kobject_put(ebox3_kobj);
        return result;
    }
    result = meter_init_6(IRQflags);
    if (result) {
        printk(KERN_ALERT "Ebox3 Driver: failed to init meter6\n");
        kobject_put(meters_kobj);
        kobject_put(ebox3_kobj);
        return result;
    }

    return result;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit ebox3driver_exit(void) {
    // clean up -- remove the kobject sysfs entry
    kobject_put(meters_kobj);
    kobject_put(ebox3_kobj);

    relays_exit();
    meter_exit_1();
    meter_exit_2();
    meter_exit_3();
    meter_exit_4();
    meter_exit_5();
    meter_exit_6();

    printk(KERN_INFO "Ebox3 Driver: Exit\n");
}


// This next calls are  mandatory
// They identify the initialization function and the cleanup function.
module_init(ebox3driver_init);
module_exit(ebox3driver_exit);
