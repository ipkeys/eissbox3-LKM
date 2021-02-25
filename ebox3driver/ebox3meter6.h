#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/kobject.h>

static unsigned int gpioMeterIn_6  = 80;
static unsigned int gpioMeterOut_6 = 81;
static int irqMeter_6;
static unsigned int meterPulses_6 = 0;
static struct timespec ts_meterLastTime_6;

static ssize_t meterPulses_6_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%u\n", meterPulses_6);
}

static ssize_t meterPulses_6_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%u", &meterPulses_6);
   return count;
}

static ssize_t meterLastTime_6_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%lu\n", ts_meterLastTime_6.tv_sec);
}

static struct kobj_attribute meterPulses_6_attr = __ATTR(counter, 0644, meterPulses_6_show, meterPulses_6_store); 
static struct kobj_attribute meterLastTime_6_attr  = __ATTR(lastTime, 0444, meterLastTime_6_show, NULL);

static struct attribute *meter_attrs_6[] = {
    &meterPulses_6_attr.attr,
    &meterLastTime_6_attr.attr,
    NULL,
};

static struct attribute_group meter_group_6 = {
    .name  = "m6",
    .attrs = meter_attrs_6,
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
static irq_handler_t meter_irq_handler_6(unsigned int irq, void *dev_id, struct pt_regs *regs) {
    getnstimeofday(&ts_meterLastTime_6);
    meterPulses_6++;
    return (irq_handler_t)IRQ_HANDLED;
}

static int meter_init_6(unsigned long IRQflags) {
    // Set up the ALL Meter Outputs to HIGH = 1
    gpio_request(gpioMeterOut_6, "sysfs");
    gpio_direction_output(gpioMeterOut_6, 1);
    gpio_export(gpioMeterOut_6, false);

    gpio_request(gpioMeterIn_6, "sysfs");
    gpio_direction_input(gpioMeterIn_6);
    gpio_set_debounce(gpioMeterIn_6, DEBOUNCE_TIME);
    gpio_export(gpioMeterIn_6, false);

    // set the last time to be the current time
    getnstimeofday(&ts_meterLastTime_6);

    // GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
    irqMeter_6 = gpio_to_irq(gpioMeterIn_6);
    printk(KERN_INFO "Ebox3 Driver: The meter6 is mapped to IRQ: %d\n", irqMeter_6);

    return request_irq(irqMeter_6, (irq_handler_t) meter_irq_handler_6, IRQflags, "meter_handler_6", NULL);
}

static void meter_exit_6(void) {
    gpio_set_value(gpioMeterOut_6, 0);
    gpio_unexport(gpioMeterOut_6);
    gpio_free(gpioMeterOut_6);

    gpio_unexport(gpioMeterIn_6);
    gpio_free(gpioMeterIn_6);

    free_irq(irqMeter_6, NULL);
}
