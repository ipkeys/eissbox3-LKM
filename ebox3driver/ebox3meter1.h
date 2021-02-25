#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/kobject.h>

static unsigned int gpioMeterIn_1  = 112;
static unsigned int gpioMeterOut_1 = 113;
static int irqMeter_1;
static unsigned int meterPulses_1 = 0;
static struct timespec ts_meterLastTime_1;

static ssize_t meterPulses_1_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%u\n", meterPulses_1);
}

static ssize_t meterPulses_1_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    sscanf(buf, "%u", &meterPulses_1);
    return count;
}

static ssize_t meterLastTime_1_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%lu\n", ts_meterLastTime_1.tv_sec);
}

static struct kobj_attribute meterPulses_1_attr = __ATTR(counter, 0644, meterPulses_1_show, meterPulses_1_store); 
static struct kobj_attribute meterLastTime_1_attr  = __ATTR(lastTime, 0444, meterLastTime_1_show, NULL);

static struct attribute *meter_attrs_1[] = {
    &meterPulses_1_attr.attr,
    &meterLastTime_1_attr.attr,
    NULL,
};

static struct attribute_group meter_group_1 = {
    .name  = "m1",
    .attrs = meter_attrs_1,
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
static irq_handler_t meter_irq_handler_1(unsigned int irq, void *dev_id, struct pt_regs *regs) {
    getnstimeofday(&ts_meterLastTime_1);
    meterPulses_1++;
    return (irq_handler_t)IRQ_HANDLED;
}

static int meter_init_1(unsigned long IRQflags) {
    // Set up the ALL Meter Outputs to HIGH = 1
    gpio_request(gpioMeterOut_1, "sysfs");
    gpio_direction_output(gpioMeterOut_1, 1);
    gpio_export(gpioMeterOut_1, false);

    gpio_request(gpioMeterIn_1, "sysfs");
    gpio_direction_input(gpioMeterIn_1);
    gpio_set_debounce(gpioMeterIn_1, DEBOUNCE_TIME);
    gpio_export(gpioMeterIn_1, false);

    // set the last time to be the current time
    getnstimeofday(&ts_meterLastTime_1);

    // GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
    irqMeter_1 = gpio_to_irq(gpioMeterIn_1);
    printk(KERN_INFO "Ebox3 Driver: The meter1 is mapped to IRQ: %d\n", irqMeter_1);

    return request_irq(irqMeter_1, (irq_handler_t) meter_irq_handler_1, IRQflags, "meter_handler_1", NULL);
}

static void meter_exit_1(void) {
    gpio_set_value(gpioMeterOut_1, 0);
    gpio_unexport(gpioMeterOut_1);
    gpio_free(gpioMeterOut_1);

    gpio_unexport(gpioMeterIn_1);
    gpio_free(gpioMeterIn_1);

    free_irq(irqMeter_1, NULL);
}
