#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/kobject.h>

static unsigned int gpioMeterIn_4  = 46;
static unsigned int gpioMeterOut_4 = 47;
static int irqMeter_4;
static unsigned int meterPulses_4 = 0;
static struct timespec ts_meterLastTime_4;

static ssize_t meterPulses_4_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%u\n", meterPulses_4);
}

static ssize_t meterPulses_4_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%u", &meterPulses_4);
   return count;
}

static ssize_t meterLastTime_4_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%lu\n", ts_meterLastTime_4.tv_sec);
}

static struct kobj_attribute meterPulses_4_attr = __ATTR(counter, 0644, meterPulses_4_show, meterPulses_4_store); 
static struct kobj_attribute meterLastTime_4_attr  = __ATTR(lastTime, 0444, meterLastTime_4_show, NULL);

static struct attribute *meter_attrs_4[] = {
    &meterPulses_4_attr.attr,
    &meterLastTime_4_attr.attr,
    NULL,
};

static struct attribute_group meter_group_4 = {
    .name  = "m4",
    .attrs = meter_attrs_4,
};

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
static irq_handler_t meter_irq_handler_4(unsigned int irq, void *dev_id, struct pt_regs *regs) {
    getnstimeofday(&ts_meterLastTime_4);
    meterPulses_4++;
    return (irq_handler_t)IRQ_HANDLED;
}

static int meter_init_4(unsigned long IRQflags) {
    // Set up the ALL Meter Outputs to HIGH = 1
    gpio_request(gpioMeterOut_4, "sysfs");
    gpio_direction_output(gpioMeterOut_4, 1);
    gpio_export(gpioMeterOut_4, false);

    gpio_request(gpioMeterIn_4, "sysfs");
    gpio_direction_input(gpioMeterIn_4);
    gpio_set_debounce(gpioMeterIn_4, DEBOUNCE_TIME);
    gpio_export(gpioMeterIn_4, false);

    // set the last time to be the current time
    getnstimeofday(&ts_meterLastTime_4);

    // GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
    irqMeter_4 = gpio_to_irq(gpioMeterIn_4);
    printk(KERN_INFO "Ebox3 Driver: The meter4 is mapped to IRQ: %d\n", irqMeter_4);

    return request_irq(irqMeter_4, (irq_handler_t) meter_irq_handler_4, IRQflags, "meter_handler_4", NULL);
}

static void meter_exit_4(void) {
    gpio_set_value(gpioMeterOut_4, 0);
    gpio_unexport(gpioMeterOut_4);
    gpio_free(gpioMeterOut_4);

    gpio_unexport(gpioMeterIn_4);
    gpio_free(gpioMeterIn_4);

    free_irq(irqMeter_4, NULL);
}
