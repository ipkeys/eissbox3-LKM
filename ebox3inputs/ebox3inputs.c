/**
 * @file   ebox3input.c
 * @author Yuriy Kozhynov
 * @date   13 January 2021
 * @brief  A kernel module for controlling a input that is connected to
 * a GPIO. It has full support for interrupts and for sysfs entries so that an interface
 * can be created to the input or the input can be configured from Linux userspace.
 * The sysfs entry appears at /sys/ebox3/inputX
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>       // Required for the GPIO functions
#include <linux/interrupt.h>  // Required for the IRQ code
#include <linux/kobject.h>    // Using kobjects for the sysfs bindings
#include <linux/time.h>       // Using the clock to measure time between button presses
#define  DEBOUNCE_TIME 50     // The default bounce time -- 50ms

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yuriy Kozhynov");
MODULE_DESCRIPTION("Driver(LKM) for GPIO Inputs for EISSbox3");
MODULE_VERSION("0.1");

static bool isRising = 1;                   ///< Rising edge is the default IRQ property

static unsigned int gpioMeterIn_1  = 112;       // Meter0 - GPIO input 112
static unsigned int gpioMeterOut_1 = 113;       // Meter0 - GPIO output 113

static char   gpioName[7] = "meter0";      ///< Null terminated default string -- just in case
static int    irqNumber;                    ///< Used to share the IRQ number within this file
static int    numberOfPulses = 0;            ///< For information, store the number of button presses
static struct timespec ts_last, ts_current, ts_diff;  ///< timespecs from linux/time.h (has nano precision)

/// Function prototype for the custom IRQ handler function -- see below for the implementation
static irq_handler_t ebox3gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

/** @brief A callback function to output the numberOfPulses variable
 *  @param kobj represents a kernel object device that appears in the sysfs filesystem
 *  @param attr the pointer to the kobj_attribute struct
 *  @param buf the buffer to which to write the number of presses
 *  @return return the total number of characters written to the buffer (excluding null)
 */
static ssize_t numberOfPulses_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%d\n", numberOfPulses);
}

/** @brief A callback function to read in the numberOfPulses variable
 *  @param kobj represents a kernel object device that appears in the sysfs filesystem
 *  @param attr the pointer to the kobj_attribute struct
 *  @param buf the buffer from which to read the number of presses (e.g., reset to 0).
 *  @param count the number characters in the buffer
 *  @return return should return the total number of characters used from the buffer
 */
static ssize_t numberOfPulses_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%du", &numberOfPulses);
   return count;
}

/** @brief Displays the last time the button was pressed -- manually output the date (no localization) */
static ssize_t lastTime_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%.2lu:%.2lu:%.2lu:%.9lu\n", (ts_last.tv_sec/3600)%24, (ts_last.tv_sec/60) % 60, ts_last.tv_sec % 60, ts_last.tv_nsec );
}

/** @brief Display the time difference in the form secs.nanosecs to 9 places */
static ssize_t diffTime_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%lu.%.9lu\n", ts_diff.tv_sec, ts_diff.tv_nsec);
}

/**  Use these helper macros to define the name and access levels of the kobj_attributes
 *  The kobj_attribute has an attribute attr (name and mode), show and store function pointers
 *  The count variable is associated with the numberPresses variable and it is to be exposed
 *  with mode 0666 using the numberPresses_show and numberPresses_store functions above
 */
//static struct kobj_attribute count_attr = __ATTR(numberOfPulses, 0666, numberOfPulses_show, numberOfPulses_store);
//static struct kobj_attribute debounce_attr = __ATTR(isDebounce, 0666, isDebounce_show, isDebounce_store);
static struct kobj_attribute count_attr = __ATTR_RW(numberOfPulses);

/**  The __ATTR_RO macro defines a read-only attribute. There is no need to identify that the
 *  function is called _show, but it must be present. __ATTR_WO can be  used for a write-only
 *  attribute but only in Linux 3.11.x on.
 */
static struct kobj_attribute time_attr  = __ATTR_RO(lastTime);  ///< the last time pressed kobject attr
static struct kobj_attribute diff_attr  = __ATTR_RO(diffTime);  ///< the difference in time attr

/**  The ebb_attrs[] is an array of attributes that is used to create the attribute group below.
 *  The attr property of the kobj_attribute is used to extract the attribute struct
 */
static struct attribute *meter_attrs[] = {
      &count_attr.attr,                  ///< The number of pulses of input
      &time_attr.attr,                   ///< Time of the last button press in HH:MM:SS:NNNNNNNNN
      &diff_attr.attr,                   ///< The difference in time between the last two presses
      NULL,
};

/**  The attribute group uses the attribute array and a name, which is exposed on sysfs -- in this
 *  case it is gpio115, which is automatically defined in the ebbButton_init() function below
 *  using the custom kernel parameter that can be passed when the module is loaded.
 */
static struct attribute_group attr_group = {
      .name  = gpioName,                 ///< The name is generated in ebox3Inputs_init()
      .attrs = meter_attrs,                ///< The attributes array defined just above
};

static struct kobject *ebox3_kobj;

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point. In this example this
 *  function sets up the GPIOs and the IRQ
 *  @return returns 0 if successful
 */
static int __init ebox3Inputs_init(void) {
   int result = 0;
   unsigned long IRQflags = IRQF_TRIGGER_RISING;      // The default is a rising-edge interrupt

   printk(KERN_INFO "Ebox3 Inputs: Initializing the Ebox3 Inputs LKM\n");
   //sprintf(gpioName, "gpio%d", gpioInput);           // Create the gpio115 name for /sys/ebox3/meter0

   // create the kobject sysfs entry at /sys/ebb -- probably not an ideal location!
   ebox3_kobj = kobject_create_and_add("ebox3", kernel_kobj->parent); // kernel_kobj points to /sys/kernel
   if (!ebox3_kobj){
      printk(KERN_ALERT "Ebox3 Inputs: failed to create kobject mapping\n");
      return -ENOMEM;
   }
   // add the attributes to /sys/ebox3/ -- for example, /sys/ebox3/meter0/numberPresses
   result = sysfs_create_group(ebox3_kobj, &attr_group);
   if (result) {
      printk(KERN_ALERT "Ebox3 Inputs: failed to create sysfs group\n");
      kobject_put(ebox3_kobj);                          // clean up -- remove the kobject sysfs entry
      return result;
   }

   getnstimeofday(&ts_last);    // set the last time to be the current time
   ts_diff = timespec64_to_timespec(timespec64_sub(timespec_to_timespec64(ts_last), timespec_to_timespec64(ts_last)));          // set the initial time difference to be 0

   // Going to set up the ALL Outputs to HIGH.
   gpio_request(gpioMeterOut_1, "sysfs");          // gpioOutput0 is hardcoded to 113, request it
   gpio_direction_output(gpioMeterOut_1, 1);       // Set the gpio to be in output mode and on
                                                // the bool argument prevents the direction from being changed
   gpio_request(gpioMeterIn_1, "sysfs");            // Set up the gpioInput0
   gpio_direction_input(gpioMeterIn_1);             // Set the button GPIO to be an input
   gpio_set_debounce(gpioMeterIn_1, DEBOUNCE_TIME); // Debounce the button with a delay of 50ms
                                                // the bool argument prevents the direction from being changed

   // Causes all gpio to appear in /sys/class/gpio
   // the bool argument prevents the direction from being changed
   gpio_export(gpioMeterIn_1, false);
   gpio_export(gpioMeterOut_1, false);

   // Perform a quick test to see that the button is working as expected on LKM load
   printk(KERN_INFO "Ebox3 Inputs: The meter0 state is currently: %d\n", gpio_get_value(gpioMeterIn_1));

   /// GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
   irqNumber = gpio_to_irq(gpioMeterIn_1);
   printk(KERN_INFO "Ebox3 Inputs: The meter0 is mapped to IRQ: %d\n", irqNumber);

   if (!isRising) {                           // If the kernel parameter isRising=0 is supplied
      IRQflags = IRQF_TRIGGER_FALLING;      // Set the interrupt to be on the falling edge
   }
   // This next call requests an interrupt line
   result = request_irq(irqNumber,             // The interrupt number requested
                        (irq_handler_t) ebox3gpio_irq_handler, // The pointer to the handler function below
                        IRQflags,              // Use the custom kernel param to set interrupt type
                        "ebox3_input_handler",  // Used in /proc/interrupts to identify the owner
                        NULL);                 // The *dev_id for shared interrupt lines, NULL is okay
   return result;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit ebox3Inputs_exit(void) {
   printk(KERN_INFO "Ebox3 Inputs: The Meter0 was pulsed %d times\n", numberOfPulses);
   kobject_put(ebox3_kobj);                 // clean up -- remove the kobject sysfs entry
   gpio_set_value(gpioMeterOut_1, 0);              // Turn the LED off, makes it clear the device was unloaded
   gpio_unexport(gpioMeterOut_1);                  // Unexport the LED GPIO
   free_irq(irqNumber, NULL);               // Free the IRQ number, no *dev_id required in this case
   gpio_unexport(gpioMeterIn_1);                // Unexport the Button GPIO
   gpio_free(gpioMeterIn_1);                      // Free the LED GPIO
   gpio_free(gpioMeterOut_1);                    // Free the Button GPIO
   printk(KERN_INFO "Ebox3 Inputs: Goodbye from the Ebox3 Inputs LKM!\n");
}

/** @brief The GPIO IRQ Handler function
 *  This function is a custom interrupt handler that is attached to the GPIO above. The same interrupt
 *  handler cannot be invoked concurrently as the interrupt line is masked out until the function is complete.
 *  This function is static as it should not be invoked directly from outside of this file.
 *  @param irq    the IRQ number that is associated with the GPIO -- useful for logging.
 *  @param dev_id the *dev_id that is provided -- can be used to identify which device caused the interrupt
 *  Not used in this example as NULL is passed.
 *  @param regs   h/w specific register values -- only really ever used for debugging.
 *  return returns IRQ_HANDLED if successful -- should return IRQ_NONE otherwise.
 */
static irq_handler_t ebox3gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) {
   getnstimeofday(&ts_current);                 // Get the current time as ts_current
   ts_diff = timespec64_to_timespec(timespec64_sub(timespec_to_timespec64(ts_current), timespec_to_timespec64(ts_last))); // Determine the time difference between last 2 presses
   ts_last = ts_current;                        // Store the current time as the last time ts_last
   printk(KERN_INFO "Ebox3 Inputs: The meter0 state is currently: %d\n", gpio_get_value(gpioMeterIn_1));
   numberOfPulses++;                            // Global counter, will be outputted when the module is unloaded
   return (irq_handler_t)IRQ_HANDLED;           // Announce that the IRQ has been handled correctly
}

// This next calls are  mandatory -- they identify the initialization function
// and the cleanup function (as above).
module_init(ebox3Inputs_init);
module_exit(ebox3Inputs_exit);
