// SPDX-License-Identifier: GPL-2.0
/*
 * Power fail GPIO driver
 *
 * Debounce the active-low power switch, then request an orderly shutdown.
 * The kernel hardware-protection path supplies a bounded poweroff fallback.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/atomic.h>

struct powerfail_suo_data {
    struct device *dev;
    struct gpio_desc *gpiod;
    struct gpio_desc *disabled_gpiod;
    int irq;
    u32 debounce_ms;
    atomic_t triggered;
    struct work_struct work;
};

static void powerfail_suo_work(struct work_struct *work)
{
    struct powerfail_suo_data *data =
        container_of(work, struct powerfail_suo_data, work);
    int val;

    /*
     * Software debounce.
     */
    if (data->debounce_ms)
        msleep(data->debounce_ms);

    /*
     * Use the raw level from irq-gpios:
     *     Actual low level -> power-fail signal is active
     *     Actual high level -> power-fail signal is inactive
     */
    val = gpiod_get_raw_value_cansleep(data->gpiod);
    if (val < 0) {
        dev_err(data->dev, "failed to read powerfail gpio: %d\n", val);

        atomic_set(&data->triggered, 0);
        enable_irq(data->irq);
        return;
    }

    if (val > 0) {
        dev_warn(data->dev,
                 "powerfail irq occurred, but gpio is inactive, val=%d\n",
                 val);

        atomic_set(&data->triggered, 0);
        enable_irq(data->irq);
        return;
    }

    /* Keep the display and filesystems available to init's rcK handlers.
     * A successful request is one-shot until shutdown; rejected GPIO edges
     * above re-arm the IRQ. Do not use the runtime-configurable default
     * protection action: this switch must request poweroff, not reboot.
     */
    dev_info(data->dev, "Power switch off: requesting orderly poweroff\n");
    __hw_protection_trigger("CM0 power switch", 30000,
                            HWPROT_ACT_SHUTDOWN);

}

static irqreturn_t powerfail_suo_irq_handler(int irq, void *dev_id)
{
    struct powerfail_suo_data *data = dev_id;

    /*
     * Prevent repeated triggers.
     */
    if (atomic_xchg(&data->triggered, 1))
        return IRQ_HANDLED;

    /*
     * Disable subsequent interrupts.
     * Power-fail handling is allowed to run only once.
     */
    disable_irq_nosync(data->irq);

    /*
     * Do not run sync/remount/poweroff from the hard IRQ context.
     * Run it from a workqueue instead.
     */
    schedule_work(&data->work);

    return IRQ_HANDLED;
}

static int powerfail_suo_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct powerfail_suo_data *data;
    int ret;

    data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;

    data->dev = dev;
    atomic_set(&data->triggered, 0);
    INIT_WORK(&data->work, powerfail_suo_work);

    of_property_read_u32(dev->of_node, "debounce-ms", &data->debounce_ms);

    /*
     * Corresponding Device Tree property:
     *
     *   irq-gpios = <&gpio 23 0x00>;
     */
    data->gpiod = devm_gpiod_get(dev, "irq", GPIOD_IN);
    if (IS_ERR(data->gpiod)) {
        ret = PTR_ERR(data->gpiod);
        dev_err(dev, "failed to get irq gpio: %d\n", ret);
        return ret;
    }

    /* Legacy DT calls LCD RESET "disabled". Only hold it deasserted here;
     * it is not a power-control pin and must never be asserted on switch-off.
     */
    data->disabled_gpiod = devm_gpiod_get_optional(dev, "disabled",
                                                   GPIOD_OUT_LOW);
    if (IS_ERR(data->disabled_gpiod))
        return dev_err_probe(dev,
                             PTR_ERR(data->disabled_gpiod),
                             "failed to get disabled gpio\n");

    data->irq = platform_get_irq(pdev, 0);
    if (data->irq < 0) {
        dev_err(dev, "failed to get irq from interrupts property: %d\n",
                data->irq);
        return data->irq;
    }

    /*
     * The interrupt trigger type is determined by the Device Tree interrupts property:
     *
     *   interrupt-parent = <&gpio>;
     *   interrupts = <23 2>;  // falling edge
     */
    ret = devm_request_irq(dev,
                           data->irq,
                           powerfail_suo_irq_handler,
                           0,
                           dev_name(dev),
                           data);
    if (ret) {
        dev_err(dev, "failed to request irq %d: %d\n",
                data->irq, ret);
        return ret;
    }

    platform_set_drvdata(pdev, data);

    /* Loading while the switch is already off must not miss the edge. */
    ret = gpiod_get_raw_value_cansleep(data->gpiod);
    if (ret == 0)
        powerfail_suo_irq_handler(data->irq, data);
    else if (ret < 0)
        dev_warn(dev, "Initial power switch read failed: %d\n", ret);

    dev_info(dev,
             "powerfail-suo driver probed, irq=%d, debounce=%u ms\n",
             data->irq, data->debounce_ms);

    return 0;
}

static void powerfail_suo_remove(struct platform_device *pdev)
{
    struct powerfail_suo_data *data = platform_get_drvdata(pdev);

    if (data) {
        /* Keep the IRQ allocated until all worker re-arm calls finish.
         * This extra disable remains in effect if a worker balances the
         * disable performed by the interrupt handler.
         */
        disable_irq(data->irq);
        cancel_work_sync(&data->work);
        devm_free_irq(&pdev->dev, data->irq, data);
    }
}

static const struct of_device_id powerfail_suo_of_match[] = {
    { .compatible = "cardputerzero,powerfail-suo" },
    { }
};
MODULE_DEVICE_TABLE(of, powerfail_suo_of_match);

static struct platform_driver powerfail_suo_driver = {
    .probe  = powerfail_suo_probe,
    .remove = powerfail_suo_remove,
    .driver = {
        .name = "powerfail-suo",
        .of_match_table = powerfail_suo_of_match,
    },
};

module_platform_driver(powerfail_suo_driver);

MODULE_AUTHOR("dianjixz <dianjixz@m5stack.com>");
MODULE_DESCRIPTION("Power switch orderly shutdown driver");
MODULE_LICENSE("GPL");

// Modified by CZLite on 2026-11-17.
