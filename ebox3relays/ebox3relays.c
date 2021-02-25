/**
 * @file   ebox3relays.c
 * @author Yuriy Kozhynov
 * @date   20 January 2021
 * @brief  A kernel module for controlling a relays that is connected to a GPIO.
 * The sysfs entry appears at /sys/ebox3/relayX
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>       // Required for the GPIO functions
//#include <linux/interrupt.h>  // Required for the IRQ code
#include <linux/kobject.h>    // Using kobjects for the sysfs bindings
//#include <linux/time.h>       // Using the clock to measure time between button presses

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yuriy Kozhynov <ykozhynov@ipkeys.com>");
MODULE_DESCRIPTION("Driver for EISSbox3 Relays");
MODULE_VERSION("0.1");

static unsigned int gpioRelay1 = 69;
static unsigned int gpioRelay2 = 68;
static unsigned int gpioRelay3 = 67;
static unsigned int gpioRelay4 = 66;

static int relay1 = 0;
static int relay2 = 0;
static int relay3 = 0;
static int relay4 = 0;

/** @brief A callback function to output the relayXstate variable
 *  @param kobj represents a kernel object device that appears in the sysfs filesystem
 *  @param attr the pointer to the kobj_attribute struct
 *  @param buf the buffer to which to write the number of presses
 *  @return return the total number of characters written to the buffer (excluding null)
 */
static ssize_t relay1_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%d\n", relay1);
}

static ssize_t relay2_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%d\n", relay2);
}

static ssize_t relay3_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%d\n", relay3);
}

static ssize_t relay4_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%d\n", relay4);
}

/** @brief A callback function to read in the relayX variable
 *  @param kobj represents a kernel object device that appears in the sysfs filesystem
 *  @param attr the pointer to the kobj_attribute struct
 *  @param buf the buffer from which to read the number of presses (e.g., reset to 0).
 *  @param count the number characters in the buffer
 *  @return return should return the total number of characters used from the buffer
 */
static ssize_t relay1_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%du", &relay1);
   gpio_set_value(gpioRelay1, relay1); 
   return count;
}

static ssize_t relay2_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%du", &relay2);
   gpio_set_value(gpioRelay2, relay2); 
   return count;
}

static ssize_t relay3_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%du", &relay3);
   gpio_set_value(gpioRelay3, relay3); 
   return count;
}

static ssize_t relay4_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%du", &relay4);
   gpio_set_value(gpioRelay4, relay4); 
   return count;
}

/**  
 * The name and access levels of the kobj_attributes
 */
static struct kobj_attribute relay1_attr = __ATTR_RW(relay1);
static struct kobj_attribute relay2_attr = __ATTR_RW(relay2);
static struct kobj_attribute relay3_attr = __ATTR_RW(relay3);
static struct kobj_attribute relay4_attr = __ATTR_RW(relay4);

/**
 * The relays_attrs[] is an array of attributes that is used to create the attribute group below.
 * The attr property of the kobj_attribute is used to extract the attribute struct
 */
static struct attribute *relays_attrs[] = {
   &relay1_attr.attr,
   &relay2_attr.attr,
   &relay3_attr.attr,
   &relay4_attr.attr,
   NULL,
};

/**
 * The attribute group uses the attribute array and a name, which is exposed on sysfs -- in this
 * case it is relayX, which is automatically defined in the XYZ_init() function below
 * using the custom kernel parameter that can be passed when the module is loaded.
 */
static struct attribute_group relays_attr_group = {
   .name  = "relays",
   .attrs = relays_attrs,
};

static struct kobject *ebox3_kobj;

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point. In this example this
 *  function sets up the GPIOs and the IRQ
 *  @return returns 0 if successful
 */
static int __init ebox3relays_init(void) {
   int result = 0;

   printk(KERN_INFO "Ebox3 Relays: Initializing of module\n");

   // create the kobject sysfs entry at /sys/ebox3
   ebox3_kobj = kobject_create_and_add("ebox3", kernel_kobj->parent); // kernel_kobj points to /sys/kernel
   if (!ebox3_kobj){
      printk(KERN_ALERT "Ebox3 Relays: failed to create kobject mapping\n");
      return -ENOMEM;
   }
   // add the attributes to /sys/ebox3/ -- for example, /sys/ebox3/relays/relay0..3
   result = sysfs_create_group(ebox3_kobj, &relays_attr_group);
   if (result) {
      printk(KERN_ALERT "Ebox3 Relays: failed to create sysfs group for relays\n");
      kobject_put(ebox3_kobj);
      return result;
   }

   // Going to set up the all relays to OFF
   gpio_request(gpioRelay1, "sysfs");
   gpio_direction_output(gpioRelay1, relay1);

   gpio_request(gpioRelay2, "sysfs");
   gpio_direction_output(gpioRelay2, relay2);

   gpio_request(gpioRelay3, "sysfs");
   gpio_direction_output(gpioRelay3, relay3);

   gpio_request(gpioRelay4, "sysfs");
   gpio_direction_output(gpioRelay4, relay4);

   // Causes all gpio to appear in /sys/class/gpio
   // the bool argument prevents the direction from being changed
   gpio_export(gpioRelay1, false);
   gpio_export(gpioRelay2, false);
   gpio_export(gpioRelay3, false);
   gpio_export(gpioRelay4, false);
 
   return result;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit ebox3relays_exit(void) {
   // clean up -- remove the kobject sysfs entry
   kobject_put(ebox3_kobj);

   // Turn all relays OFF, makes it clear the device was unloaded
   gpio_set_value(gpioRelay1, 0);
   gpio_set_value(gpioRelay2, 0);
   gpio_set_value(gpioRelay3, 0);
   gpio_set_value(gpioRelay4, 0);

   // Unexport all relays GPIO
   gpio_unexport(gpioRelay1);
   gpio_unexport(gpioRelay2);
   gpio_unexport(gpioRelay3);
   gpio_unexport(gpioRelay4);

   // Free the relays GPIO
   gpio_free(gpioRelay1);
   gpio_free(gpioRelay2);
   gpio_free(gpioRelay3);
   gpio_free(gpioRelay4);

   printk(KERN_INFO "Ebox3 Relays: Exit from module.\n");
}

// This next calls are  mandatory
// They identify the initialization function and the cleanup function.
module_init(ebox3relays_init);
module_exit(ebox3relays_exit);
