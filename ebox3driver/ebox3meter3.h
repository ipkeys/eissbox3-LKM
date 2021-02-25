#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/kobject.h>

static unsigned int gpioMeterIn_3  = 44;
static unsigned int gpioMeterOut_3 = 45;
static int irqMeter_3;
static unsigned int meterPulses_3 = 0;
static struct timespec ts_meterLastTime_3;

static ssize_t meterPulses_3_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
   return sprintf(buf, "%u\n", meterPulses_3);
}

static ssize_t meterPulses_3_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
   sscanf(buf, "%u", &meterPulses_3);
   return count;
}

static ssize_t meterLastTime_3_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%lu\n", ts_meterLastTime_3.tv_sec);
}

static struct kobj_attribute meterPulses_3_attr = __ATTR(counter, 0644, meterPulses_3_show, meterPulses_3_store); 
static struct kobj_attribute meterLastTime_3_attr  = __ATTR(lastTime, 0444, meterLastTime_3_show, NULL);

static struct attribute *meter_attrs_3[] = {
    &meterPulses_3_attr.attr,
    &meterLastTime_3_attr.attr,
    NULL,
};

static struct attribute_group meter_group_3 = {
    .name  = "m3",
    .attrs = meter_attrs_3,
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
static irq_handler_t meter_irq_handler_3(unsigned int irq, void *dev_id, struct pt_regs *regs) {
    getnstimeofday(&ts_meterLastTime_3);
    meterPulses_3++;
    return (irq_handler_t)IRQ_HANDLED;
}

static int meter_init_3(unsigned long IRQflags) {
    // Set up the ALL Meter Outputs to HIGH = 1
    gpio_request(gpioMeterOut_3, "sysfs");
    gpio_direction_output(gpioMeterOut_3, 1);
    gpio_export(gpioMeterOut_3, false);

    gpio_request(gpioMeterIn_3, "sysfs");
    gpio_direction_input(gpioMeterIn_3);
    gpio_set_debounce(gpioMeterIn_3, DEBOUNCE_TIME);
    gpio_export(gpioMeterIn_3, false);

    // set the last time to be the current time
    getnstimeofday(&ts_meterLastTime_3);

    // GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
    irqMeter_3 = gpio_to_irq(gpioMeterIn_3);
    printk(KERN_INFO "Ebox3 Driver: The meter3 is mapped to IRQ: %d\n", irqMeter_3);

    return request_irq(irqMeter_3, (irq_handler_t) meter_irq_handler_3, IRQflags, "meter_handler_3", NULL);
}

static void meter_exit_3(void) {
    gpio_set_value(gpioMeterOut_3, 0);
    gpio_unexport(gpioMeterOut_3);
    gpio_free(gpioMeterOut_3);

    gpio_unexport(gpioMeterIn_3);
    gpio_free(gpioMeterIn_3);

    free_irq(irqMeter_3, NULL);
}
