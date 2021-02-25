#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/kobject.h>

static unsigned int gpioMeterIn_5  = 78;
static unsigned int gpioMeterOut_5 = 79;
static int irqMeter_5;
static unsigned int meterPulses_5 = 0;
static struct timespec ts_meterLastTime_5;

static ssize_t meterPulses_5_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%u\n", meterPulses_5);
}

static ssize_t meterPulses_5_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%u", &meterPulses_5);
   return count;
}

static ssize_t meterLastTime_5_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%lu\n", ts_meterLastTime_5.tv_sec);
}

static struct kobj_attribute meterPulses_5_attr = __ATTR(counter, 0644, meterPulses_5_show, meterPulses_5_store); 
static struct kobj_attribute meterLastTime_5_attr  = __ATTR(lastTime, 0444, meterLastTime_5_show, NULL);

static struct attribute *meter_attrs_5[] = {
    &meterPulses_5_attr.attr,
    &meterLastTime_5_attr.attr,
    NULL,
};

static struct attribute_group meter_group_5 = {
    .name  = "m5",
    .attrs = meter_attrs_5,
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
static irq_handler_t meter_irq_handler_5(unsigned int irq, void *dev_id, struct pt_regs *regs) {
    getnstimeofday(&ts_meterLastTime_5);
    meterPulses_5++;
    return (irq_handler_t)IRQ_HANDLED;
}

static int meter_init_5(unsigned long IRQflags) {
    // Set up the ALL Meter Outputs to HIGH = 1
    gpio_request(gpioMeterOut_5, "sysfs");
    gpio_direction_output(gpioMeterOut_5, 1);
    gpio_export(gpioMeterOut_5, false);

    gpio_request(gpioMeterIn_5, "sysfs");
    gpio_direction_input(gpioMeterIn_5);
    gpio_set_debounce(gpioMeterIn_5, DEBOUNCE_TIME);
    gpio_export(gpioMeterIn_5, false);

    // set the last time to be the current time
    getnstimeofday(&ts_meterLastTime_5);

    // GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
    irqMeter_5 = gpio_to_irq(gpioMeterIn_5);
    printk(KERN_INFO "Ebox3 Driver: The meter5 is mapped to IRQ: %d\n", irqMeter_5);

    return request_irq(irqMeter_5, (irq_handler_t) meter_irq_handler_5, IRQflags, "meter_handler_5", NULL);
}

static void meter_exit_5(void) {
    gpio_set_value(gpioMeterOut_5, 0);
    gpio_unexport(gpioMeterOut_5);
    gpio_free(gpioMeterOut_5);

    gpio_unexport(gpioMeterIn_5);
    gpio_free(gpioMeterIn_5);

    free_irq(irqMeter_5, NULL);
}
