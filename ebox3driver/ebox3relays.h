#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/kobject.h>

static unsigned int gpioRelay1 = 69;
static unsigned int gpioRelay2 = 68;
static unsigned int gpioRelay3 = 67;
static unsigned int gpioRelay4 = 66;

static int r1 = 0;
static int r2 = 0;
static int r3 = 0;
static int r4 = 0;

/** @brief A callback function to output the relayXstate variable
 *  @param kobj represents a kernel object device that appears in the sysfs filesystem
 *  @param attr the pointer to the kobj_attribute struct
 *  @param buf the buffer to which to write the number of presses
 *  @return return the total number of characters written to the buffer (excluding null)
 */
static ssize_t r1_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%d\n", r1);
}
static ssize_t r2_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%d\n", r2);
}
static ssize_t r3_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%d\n", r3);
}
static ssize_t r4_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%d\n", r4);
}

/** @brief A callback function to read in the relayX variable
 *  @param kobj represents a kernel object device that appears in the sysfs filesystem
 *  @param attr the pointer to the kobj_attribute struct
 *  @param buf the buffer from which to read the number of presses (e.g., reset to 0).
 *  @param count the number characters in the buffer
 *  @return return should return the total number of characters used from the buffer
 */
static ssize_t r1_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%u", &r1);
   gpio_set_value(gpioRelay1, r1); 
   return count;
}
static ssize_t r2_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%u", &r2);
   gpio_set_value(gpioRelay2, r2); 
   return count;
}
static ssize_t r3_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%u", &r3);
   gpio_set_value(gpioRelay3, r3); 
   return count;
}
static ssize_t r4_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%u", &r4);
   gpio_set_value(gpioRelay4, r4); 
   return count;
}

static struct kobj_attribute r1_attr = __ATTR_RW(r1);
static struct kobj_attribute r2_attr = __ATTR_RW(r2);
static struct kobj_attribute r3_attr = __ATTR_RW(r3);
static struct kobj_attribute r4_attr = __ATTR_RW(r4);

static struct attribute *relays_attrs[] = {
   &r1_attr.attr,
   &r2_attr.attr,
   &r3_attr.attr,
   &r4_attr.attr,
   NULL,
};

static struct attribute_group relays_group = {
   .name  = "relays",
   .attrs = relays_attrs,
};

static void relays_init(void) {
    // Set up the all relays to OFF = 0
   // Causes all gpio to appear in /sys/class/gpio the bool argument prevents the direction from being changed
   gpio_request(gpioRelay1, "sysfs");
   gpio_direction_output(gpioRelay1, 0);
   gpio_export(gpioRelay1, false);

   gpio_request(gpioRelay2, "sysfs");
   gpio_direction_output(gpioRelay2, 0);
   gpio_export(gpioRelay2, false);

   gpio_request(gpioRelay3, "sysfs");
   gpio_direction_output(gpioRelay3, 0);
   gpio_export(gpioRelay3, false);

   gpio_request(gpioRelay4, "sysfs");
   gpio_direction_output(gpioRelay4, 0);
   gpio_export(gpioRelay4, false);
}

static void relays_exit(void) {
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
}