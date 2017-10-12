/*
 *  USB bosto_2g tablet support
 *
 *  Original Copyright (c) 2010 Xing Wei <weixing@hanwang.com.cn>
 *
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/usb.h>


#define USB_VENDOR_ID_HANWANG		0x0b57
#define USB_PRODUCT_BOSTO22HD		0x9016
#define USB_PRODUCT_BOSTO14WA		0x9018
#define USB_PRODUCT_ART_MASTER_III	0x852A
#define USB_PRODUCT_ART_MASTER_HD	0x8401

#define STYLUS_DEVICE_ID	0x02
#define ERASER_DEVICE_ID	0x0A

#define PKGLEN_MAX 10
#define MAX_DEVICE_NAME 30

MODULE_AUTHOR("Xing Wei <weixing@hanwang.com.cn>");
MODULE_AUTHOR("Aidan Walton <aidan@wires3.net>");
MODULE_AUTHOR("Leslie Viljoen <leslieviljoen@gmail.com>");
MODULE_AUTHOR("Tomasz Flis <tflis84@gmail.com>");
MODULE_AUTHOR("Mark Riedesel <mark@klowner.com>");

MODULE_DESCRIPTION("USB Bosto(2nd Gen) tablet driver");
MODULE_LICENSE("GPL");

struct bosto_2g {
	const struct bosto_2g_features *features;
	struct input_dev *stylus;
	struct input_dev *rubber;
	unsigned int current_tool;
	unsigned int current_id;
	unsigned int tool_update;
	char stylus_name[MAX_DEVICE_NAME];
	char rubber_name[MAX_DEVICE_NAME];
	bool stylus_btn_bosto_2g;
	bool stylus_prox;
};

enum bosto_2g_tablet_type {
	HANWANG_BOSTO_22HD,
	HANWANG_BOSTO_14WA,
	HANWANG_ART_MASTER_III,
	HANWANG_ART_MASTER_HD,
};

struct bosto_2g_features {
	unsigned short product_id;
	char* name;
	enum bosto_2g_tablet_type type;
	int max_x;
	int max_y;
	int max_pressure;
	int res_x;
	int res_y;
};

static const struct bosto_2g_features features_array[] = {
	{ USB_PRODUCT_BOSTO22HD, "Bosto Kingtee 22HD", HANWANG_BOSTO_22HD,
		0xdbe8, 0x7bb3, 2048, 55, 55 },
};

static const int hw_btnevents[] = {
	BTN_STYLUS,
	BTN_TOOL_PEN,
	BTN_TOOL_RUBBER,
	BTN_TOUCH,
};

static const int hw_absevents[] = {
	ABS_MISC,
	ABS_PRESSURE,
	ABS_X,
	ABS_Y,
};

static const int hw_mscevents[] = {
	MSC_SERIAL,
};

static int bosto_2g_raw_event(struct hid_device *hid, struct hid_report *report,
		u8 *data, int size)
{
	struct hid_input *hidinput;
	struct bosto_2g *bosto_2g;
	unsigned int pkt_type = 0x00;
	static u16 x = 0;
	static u16 y = 0;
	static u16 p = 0;
	if (data[1] == 0x80) pkt_type = 1; // Idle or tool status update in next few packets
	if (data[1] == 0xC2) pkt_type = 2; // Tool status update packet
	if (((data[1] & 0xF0) == 0xA0) | ((data[1] & 0xF0) == 0xE0)) pkt_type = 3; // In proximity float 0x0A or touch 0xE0

	dev_dbg(&hid->dev, "Bosto packet:  [B0:-:B8] %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x type %d\n",
			data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], pkt_type);

	bosto_2g = hid_get_drvdata(hid);

	hidinput = list_first_entry(&hid->inputs, struct hid_input, list);

	switch (data[0]) {
		/* pen event */
		case 0x02:
			switch (pkt_type) {
				/* tool proximity out */
				case 1:
					bosto_2g->stylus_btn_bosto_2g = false;
					if (bosto_2g->stylus_prox) { // Three 0x80 indicates stylus out of proximity
						input_report_key(hidinput->input, BTN_STYLUS, 0);
						input_report_key(hidinput->input, BTN_TOUCH, 0);
						input_report_key(hidinput->input, BTN_TOOL_PEN, 0);
						input_report_key(hidinput->input, BTN_TOOL_RUBBER, 0);

						bosto_2g->current_id   = 0;
						bosto_2g->current_tool = 0;
						bosto_2g->tool_update  = 1;
						bosto_2g->stylus_prox  = false;
						dev_dbg(&hid->dev, "bosto tool out");
					}
					break;

				/* button event */
				case 2:
					bosto_2g->stylus_prox = true;
					switch (data[3] & 0xF0) {
						// Stylus tip in proximity (Bosto 22HD)
						case 0x20:
							if ((bosto_2g->current_id == ERASER_DEVICE_ID) | (bosto_2g->current_id == 0)) {
								bosto_2g->current_id = STYLUS_DEVICE_ID;
								bosto_2g->current_tool = BTN_TOOL_PEN;
								bosto_2g->tool_update = 1;
								input_report_key(hidinput->input, BTN_TOOL_PEN, 1);
								dev_dbg(&hid->dev, "Bosto TOOL ID: STYLUS");
							}
							break;

						// Stylus eraser in proximity (Bosto 22HD)
						case 0xA0:
							if ((bosto_2g->current_id == STYLUS_DEVICE_ID) | (bosto_2g->current_id == 0)) {
								bosto_2g->current_id = ERASER_DEVICE_ID;
								bosto_2g->current_tool = BTN_TOOL_RUBBER;
								bosto_2g->tool_update = 1;
								input_report_key(hidinput->input, BTN_TOOL_RUBBER, 1);
								dev_dbg(&hid->dev, "Bosto TOOL ID: ERASER");
							}
							break;

						default:
							bosto_2g->current_id = 0;
							dev_dbg(&hid->dev, "Bosto Unknown tablet tool %02x ", data[0]);
					}
					break;

				/* tool proximity in */
				case 3:
					bosto_2g->stylus_prox = true;
					x = (data[2] << 8) | data[3]; // Set x ABS
					y = (data[4] << 8) | data[5]; // Set y ABS
					if ((data[1] & 0xF0) == 0xE0) {
						input_report_key(hidinput->input, BTN_TOUCH, 1);
						dev_dbg(&hid->dev, "Bosto TOOL: TOUCH");
						p = (data[7] >> 5) | (data[6] << 3) | (data[1] & 0x1); // Set 2048 Level pressure sensitivity.
						p = le16_to_cpup((__le16 *)&p);
					} else {
						p = 0;
						input_report_key(hidinput->input, BTN_TOUCH, 0);
						dev_dbg(&hid->dev, "Bosto TOOL: FLOAT");
					}

					if ((data[1] >> 1) & 1) {
						if (!bosto_2g->stylus_btn_bosto_2g) {
							input_report_key(hidinput->input, BTN_STYLUS, 1);
							bosto_2g->stylus_btn_bosto_2g = true;
							dev_dbg(&hid->dev, "Bosto BUTTON: BTN_STYLUS pressed");
						}
					}
					else if (bosto_2g->stylus_btn_bosto_2g) {
						input_report_key(hidinput->input, BTN_STYLUS, 0);
						bosto_2g->stylus_btn_bosto_2g = false;
						dev_dbg(&hid->dev, "Bosto BUTTON: BTN_STYLUS released");
					}
					break;

				default:
					dev_dbg(&hid->dev, "Error packet. Packet data[1]: %02x ", data[1]);
			}
			break;

		case 0x0c:
			dev_dbg(&hid->dev, "Bosto BUTTON: Tablet button pressed");

			break;

		default:
			dev_dbg(&hid->dev, "Error packet. Packet data[0]: %02x ", data[0]);
			break;
	}

	if (x > bosto_2g->features->max_x) { x = bosto_2g->features->max_x; }
	if (y > bosto_2g->features->max_y) { y = bosto_2g->features->max_y; }
	if (p > bosto_2g->features->max_pressure) { p = bosto_2g->features->max_pressure; }

	if (bosto_2g->tool_update == 0) {
		dev_dbg(&hid->dev, "pos: %d,%d pres: %d max: %d,%d", x, y, p, bosto_2g->features->max_x, bosto_2g->features->max_y);
		input_report_abs(hidinput->input, ABS_X, le16_to_cpup((__le16 *)&x));
		input_report_abs(hidinput->input, ABS_Y, le16_to_cpup((__le16 *)&y));
		input_report_abs(hidinput->input, ABS_PRESSURE, p);
	}
	input_report_abs(hidinput->input, ABS_MISC, bosto_2g->current_id);
	input_event(hidinput->input, EV_MSC, MSC_SERIAL, bosto_2g->features->product_id);
	input_sync(hidinput->input);
	bosto_2g->tool_update = 0;

	return 0;
}

static bool get_features(struct hid_device *hid, struct bosto_2g *bosto_2g)
{
	int i;
	struct usb_interface *intf = to_usb_interface(hid->dev.parent);
	struct usb_device *dev = interface_to_usbdev(intf);

	dev_dbg(&hid->dev, "product id: %d %d", le16_to_cpu(dev->descriptor.idVendor),
			le16_to_cpu(dev->descriptor.idProduct));

	for (i = 0; i < ARRAY_SIZE(features_array); i++) {
		if (le16_to_cpu(dev->descriptor.idProduct) == features_array[i].product_id) {
			bosto_2g->features = &features_array[i];
			return true;
		}
	}

	return false;
}

static int bosto_2g_probe(struct hid_device *hid, const struct hid_device_id *id)
{
	struct bosto_2g *bosto_2g;
	struct hid_input *hidinput;
	struct input_dev *input;
	int error;
	int i;
	int ret;

	bosto_2g = devm_kzalloc(&hid->dev, sizeof(*bosto_2g), GFP_KERNEL);
	if (!bosto_2g) {
		return -ENOMEM;
	}

	hid_set_drvdata(hid, bosto_2g);

	if (!get_features(hid, bosto_2g)) {
		hid_err(hid, "failed to get device features\n");
		error = -ENXIO;
		goto fail;
	}

	ret = hid_parse(hid);
	if (ret) {
		hid_err(hid, "hw start failed\n");
		return ret;
	}

	ret = hid_hw_start(hid, HID_CONNECT_DEFAULT);
	if (ret) {
		hid_err(hid, "hw start failed\n");
		return ret;
	}

	hidinput = list_first_entry(&hid->inputs, struct hid_input, list);
	input = hidinput->input;

	input->name = bosto_2g->features->name;

	for (i = 0; i < ARRAY_SIZE(hw_absevents); ++i)
		input_set_capability(input, EV_ABS, hw_absevents[i]);

	for (i = 0; i < ARRAY_SIZE(hw_btnevents); ++i)
		input_set_capability(input, EV_KEY, hw_btnevents[i]);

	for (i = 0; i < ARRAY_SIZE(hw_mscevents); ++i)
		input_set_capability(input, EV_MSC, hw_mscevents[i]);

	input_set_capability(input, EV_REL, 0);
	input_abs_set_res(input, ABS_X, bosto_2g->features->res_x);
	input_abs_set_res(input, ABS_Y, bosto_2g->features->res_y);
	input_set_abs_params(input, ABS_X, 0, bosto_2g->features->max_x, 0, 0);
	input_set_abs_params(input, ABS_Y, 0, bosto_2g->features->max_y, 0, 0);
	input_set_abs_params(input, ABS_PRESSURE, 0, bosto_2g->features->max_pressure, 0, 0);

	return 0;

fail:
	kfree(bosto_2g);
	return error;
}

static const struct hid_device_id bosto_2g_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_HANWANG, USB_PRODUCT_BOSTO22HD) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_HANWANG, USB_PRODUCT_BOSTO14WA) },
	{ }
};
MODULE_DEVICE_TABLE(hid, bosto_2g_devices);

static struct hid_driver bosto_2g_driver = {
	.name = "bosto-2g",
	.id_table = bosto_2g_devices,
	.raw_event = bosto_2g_raw_event,
	.probe = bosto_2g_probe,
};
module_hid_driver(bosto_2g_driver);
