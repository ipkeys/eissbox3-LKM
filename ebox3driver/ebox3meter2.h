#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/kobject.h>

static unsigned int gpioMeterIn_2  = 110;
static unsigned int gpioMeterOut_2 = 111;
static int irqMeter_2;
static unsigned int meterPulses_2 = 0;
static struct timespec ts_meterLastTime_2;

static ssize_t meterPulses_2_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%u\n", meterPulses_2);
}

static ssize_t meterPulses_2_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%u", &meterPulses_2);
   return count;
}

static ssize_t meterLastTime_2_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%lu\n", ts_meterLastTime_2.tv_sec);
}

static struct kobj_attribute meterPulses_2_attr = __ATTR(counter, 0644, meterPulses_2_show, meterPulses_2_store); 
static struct kobj_attribute meterLastTime_2_attr  = __ATTR(lastTime, 0444, meterLastTime_2_show, NULL);

static struct attribute *meter_attrs_2[] = {
    &meterPulses_2_attr.attr,
    &meterLastTime_2_attr.attr,
    NULL,
};

static struct attribute_group meter_group_2 = {
    .name  = "m2",
    .attrs = meter_attrs_2,
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
static irq_handler_t meter_irq_handler_2(unsigned int irq, void *dev_id, struct pt_regs *regs) {
    getnstimeofday(&ts_meterLastTime_2);
    meterPulses_2++;
    return (irq_handler_t)IRQ_HANDLED;
}

static int meter_init_2(unsigned long IRQflags) {
    // Set up the ALL Meter Outputs to HIGH = 1
    gpio_request(gpioMeterOut_2, "sysfs");
    gpio_direction_output(gpioMeterOut_2, 1);
    gpio_export(gpioMeterOut_2, false);

    gpio_request(gpioMeterIn_2, "sysfs");
    gpio_direction_input(gpioMeterIn_2);
    gpio_set_debounce(gpioMeterIn_2, DEBOUNCE_TIME);
    gpio_export(gpioMeterIn_2, false);

    // set the last time to be the current time
    getnstimeofday(&ts_meterLastTime_2);

    // GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
    irqMeter_2 = gpio_to_irq(gpioMeterIn_2);
    printk(KERN_INFO "Ebox3 Driver: The meter2 is mapped to IRQ: %d\n", irqMeter_2);

    return request_irq(irqMeter_2, (irq_handler_t) meter_irq_handler_2, IRQflags, "meter_handler_2", NULL);
}

static void meter_exit_2(void) {
    gpio_set_value(gpioMeterOut_2, 0);
    gpio_unexport(gpioMeterOut_2);
    gpio_free(gpioMeterOut_2);

    gpio_unexport(gpioMeterIn_2);
    gpio_free(gpioMeterIn_2);

    free_irq(irqMeter_2, NULL);
}
