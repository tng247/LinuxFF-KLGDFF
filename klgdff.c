#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include "klgd_ff_plugin.h"

#define EFFECT_COUNT 8

static struct kobject *klgdff_obj;

static struct input_dev *dev;
static struct klgd_main klgd;
static struct klgd_plugin *ff_plugin;

static int klgdff_erase(struct klgd_command_stream *s, const struct ff_effect *effect)
{
	char *text = kasprintf(GFP_KERNEL, "Erasing effect, type %d, id %d", effect->type, effect->id);
	size_t len = strlen(text);
	struct klgd_command *c = klgd_alloc_cmd(len + 1);

	if (!c)
		return -ENOMEM;

	memcpy(c->bytes, text, len);
	kfree(text);
	return klgd_append_cmd(s, c);
}

static int klgdff_owr_start(struct klgd_command_stream *s, const struct ff_effect *effect, const struct ff_effect *old_effect, const int repeat)
{
	char *text = kasprintf(GFP_KERNEL, "Overwriting effect to STARTED state, type %d, id %d, old type %d, repeat %d", effect->type, effect->id, old_effect->type, repeat);
	size_t len = strlen(text);
	struct klgd_command *c = klgd_alloc_cmd(len + 1);

	if (!c)
		return -ENOMEM;

	memcpy(c->bytes, text, len);
	kfree(text);
	return klgd_append_cmd(s, c);
}

static int klgdff_owr_upload(struct klgd_command_stream *s, const struct ff_effect *effect, const struct ff_effect *old_effect)
{
	char *text = kasprintf(GFP_KERNEL, "Overwriting effect to UPLOADED state, type %d, id %d, old type %d", effect->type, effect->id, old_effect->type);
	size_t len = strlen(text);
	struct klgd_command *c = klgd_alloc_cmd(len + 1);

	if (!c)
		return -ENOMEM;

	memcpy(c->bytes, text, len);
	kfree(text);
	return klgd_append_cmd(s, c);
}

static int klgdff_er_stop(struct klgd_command_stream *s, const struct ff_effect *effect)
{
	char *text = kasprintf(GFP_KERNEL, "Stopping and erasing effect, type %d, id %d", effect->type, effect->id);
	size_t len = strlen(text);
	struct klgd_command *c = klgd_alloc_cmd(len + 1);

	if (!c)
		return -ENOMEM;

	memcpy(c->bytes, text, len);
	kfree(text);
	return klgd_append_cmd(s, c);
}

static int klgdff_start(struct klgd_command_stream *s, const struct ff_effect *effect, const int repeat)
{
	char *text = kasprintf(GFP_KERNEL, "Playing effect, type %d, id %d, repeat %d", effect->type, effect->id, repeat);
	size_t len = strlen(text);
	struct klgd_command *c = klgd_alloc_cmd(len + 1);

	if (!c)
		return -ENOMEM;

	memcpy(c->bytes, text, len);
	kfree(text);
	return klgd_append_cmd(s, c);
}

static int klgdff_stop(struct klgd_command_stream *s, const struct ff_effect *effect)
{
	char *text = kasprintf(GFP_KERNEL, "Stopping effect, type %d, id %d", effect->type, effect->id);
	size_t len = strlen(text);
	struct klgd_command *c = klgd_alloc_cmd(len + 1);

	if (!c)
		return -ENOMEM;

	memcpy(c->bytes, text, len);
	kfree(text);
	return klgd_append_cmd(s, c);
}

static int klgdff_update(struct klgd_command_stream *s, const struct ff_effect *effect)
{
	char *text = kasprintf(GFP_KERNEL, "Updating effect, type %d, id %d", effect->type, effect->id);
	size_t len = strlen(text);
	struct klgd_command *c = klgd_alloc_cmd(len + 1);

	if (!c)
		return -ENOMEM;

	memcpy(c->bytes, text, len);
	kfree(text);
	return klgd_append_cmd(s, c);
}

static int klgdff_upload(struct klgd_command_stream *s, const struct ff_effect *effect)
{
	char *text = kasprintf(GFP_KERNEL, "Uploading effect, type %d, id %d", effect->type, effect->id);
	size_t len = strlen(text);
	struct klgd_command *c = klgd_alloc_cmd(len + 1);

	if (!c)
		return -ENOMEM;

	memcpy(c->bytes, text, len);
	kfree(text);
	return klgd_append_cmd(s, c);
}

static int klgdff_up_start(struct klgd_command_stream *s, const struct ff_effect *effect, const int repeat)
{
	char *text = kasprintf(GFP_KERNEL, "Uploading and starting effect, type %d, id %d, repeat %d", effect->type, effect->id, repeat);
	size_t len = strlen(text);
	struct klgd_command *c = klgd_alloc_cmd(len + 1);

	if (!c)
		return -ENOMEM;

	memcpy(c->bytes, text, len);
	kfree(text);
	return klgd_append_cmd(s, c);
}

int klgdff_callback(void *data, const struct klgd_command_stream *s)
{
	size_t idx;

	printk(KERN_NOTICE "KLGDTM - EFF...\n");
	for (idx = 0; idx < s->count; idx++)
		printk(KERN_NOTICE "KLGDFF-TD: EFF %s\n", s->commands[idx]->bytes);

	/* Simulate default USB polling rate of 125 Hz */
	/*usleep_range(7500, 8500);*/
	/* Long delay to test more complicated steps */
	usleep_range(25000, 35000);

	return 0;
}

int klgdff_control(struct input_dev *dev, struct klgd_command_stream *s, const enum ffpl_control_command cmd, const union ffpl_control_data data)
{
	if (!s)
		return -EINVAL;

	if (!data.effects.cur) {
		printk(KERN_WARNING "KLGDFF-TM: NULL effect, this _cannot_ happen!\n");
		return -EINVAL;
	}

	switch (cmd) {
	case FFPL_EMP_TO_UPL:
		return klgdff_upload(s, data.effects.cur);
		break;
	case FFPL_UPL_TO_SRT:
		return klgdff_start(s, data.effects.cur, data.effects.repeat);
		break;
	case FFPL_SRT_TO_UPL:
		return klgdff_stop(s, data.effects.cur);
		break;
	case FFPL_UPL_TO_EMP:
		return klgdff_erase(s, data.effects.cur);
		break;
	case FFPL_SRT_TO_UDT:
		return klgdff_update(s, data.effects.cur);
		break;
	/* "Uploadless/eraseless" commands */
	case FFPL_EMP_TO_SRT:
		return klgdff_up_start(s, data.effects.cur, data.effects.repeat);
		break;
	case FFPL_SRT_TO_EMP:
		return klgdff_er_stop(s, data.effects.cur);
		break;
	/* "Direct" replacing commands */
	case FFPL_OWR_TO_SRT:
		return klgdff_owr_start(s, data.effects.cur, data.effects.old, data.effects.repeat);
		break;
        case FFPL_OWR_TO_UPL:
		return klgdff_owr_upload(s, data.effects.cur, data.effects.old);
		break;
	default:
		printk(KERN_NOTICE "KLGDFF-TD - Unhandled command\n");
		break;
	}

	return 0;
}

static void __exit klgdff_exit(void)
{
	input_unregister_device(dev);
	klgd_deinit(&klgd);
	kobject_put(klgdff_obj);
	printk(KERN_NOTICE "KLGD FF sample module removed\n");
}

static int __init klgdff_init(void)
{
	unsigned long ffbits = FFPL_EFBIT(FF_CONSTANT) |
			       FFPL_EFBIT(FF_RUMBLE) |
			       FFPL_EFBIT(FF_PERIODIC) | FFPL_EFBIT(FF_SINE)
						       | FFPL_EFBIT(FF_SQUARE)
						       | FFPL_EFBIT(FF_SAW_UP)
						       | FFPL_EFBIT(FF_SAW_DOWN)
						       | FFPL_EFBIT(FF_TRIANGLE) |
			       FFPL_EFBIT(FF_RAMP);
	int ret;

	klgdff_obj = kobject_create_and_add("klgdff_obj", kernel_kobj);
	if (!klgdff_obj)
		return -ENOMEM;

	ret = klgd_init(&klgd, NULL, klgdff_callback, 1);
	if (ret) {
		printk(KERN_ERR "KLGDFF-TD: Cannot initialize KLGD\n");
		goto errout_klgd;
	}

	dev = input_allocate_device();
	if (!dev) {
		ret = -ENODEV;
		printk(KERN_ERR "KLGDFF-TD: Cannot allocate input device\n");
		goto errout_idev;
	}
	dev->id.bustype = BUS_VIRTUAL;
	dev->id.vendor = 0xffff;
	dev->id.product = 0x8807;
	dev->id.version = 0x8807;
	dev->name = kasprintf(GFP_KERNEL, "KLGD-FF TestModule");
	dev->uniq = kasprintf(GFP_KERNEL, "KLGD-FF TestModule-X");
	dev->dev.parent = NULL;

	input_set_capability(dev, EV_ABS, ABS_X);
	input_set_capability(dev, EV_ABS, ABS_Y);
	input_set_capability(dev, EV_KEY, BTN_0);
	input_set_capability(dev, EV_KEY, BTN_TRIGGER);
	input_set_abs_params(dev, ABS_X, -0x7fff, 0x7fff, 0, 0);
	input_set_abs_params(dev, ABS_Y, -0x7fff, 0x7fff, 0, 0);

	ret = ffpl_init_plugin(&ff_plugin, dev, EFFECT_COUNT, ffbits,
			       FFPL_HAS_EMP_TO_SRT | FFPL_REPLACE_STARTED,
			       klgdff_control);
	if (ret) {
		printk(KERN_ERR "KLGDFF-TD: Cannot init plugin\n");
		goto errout_idev;
	}
	ret = input_register_device(dev);
	if (ret) {
		printk(KERN_ERR "KLGDFF-TD: Cannot register input device\n");
		goto errout_regdev;
	}
	
	ret = klgd_register_plugin(&klgd, 0, ff_plugin, true);
	if (ret) {
		printk(KERN_ERR "KLGDFF-TD: Cannot register plugin\n");
		goto errout_idev;
	}



	printk(KERN_NOTICE "KLGDFF-TD: Sample module loaded\n");
	return 0;

errout_regdev:
	input_free_device(dev);
errout_idev:
	klgd_deinit(&klgd);
errout_klgd:
	kobject_put(klgdff_obj);
	return ret;
}

module_exit(klgdff_exit)
module_init(klgdff_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michal \"MadCatX\" Maly");
MODULE_DESCRIPTION("KLGD FF TestModule");


