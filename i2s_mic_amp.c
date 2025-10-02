// SPDX-License-Identifier: GPL-2.0
/*
 * Minimal ASoC machine driver for a single opencores i2s CPU DAI
 * with separate playback (max98357a) and capture (ics43432) codecs.
 *
 * Expects a device tree node like:
 *
 * sound {
 *     compatible = "briansune,i2s-mic-amp";
 *     cpu-dai = <&i2s>;
 *     playback-codec = <&max98357a>;
 *     capture-codec  = <&ics43432>;
 * };
 */

#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <sound/soc.h>

struct oc_i2s_mach {
	struct snd_soc_card card;
	struct snd_soc_dai_link dai_links[2];
	struct snd_soc_dai_link_component cpu;
	struct snd_soc_dai_link_component codec_playback;
	struct snd_soc_dai_link_component codec_capture;
	struct snd_soc_dai_link_component platform; /* optional, reuse cpu node */
	struct device *dev;
};

static int oc_i2s_card_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct oc_i2s_mach *mach;
	struct snd_soc_dai_link *links;
	int ret;

	mach = devm_kzalloc(&pdev->dev, sizeof(*mach), GFP_KERNEL);
	if (!mach)
		return -ENOMEM;

	mach->dev = &pdev->dev;
	mach->card.dev = &pdev->dev;
	mach->card.owner = THIS_MODULE;

	/* card name */
	if (!of_property_read_string(np, "model", &mach->card.name))
		; /* use model if present */
	else
		mach->card.name = "opencores-i2s-card";

	/* Parse phandles: cpu-dai, playback-codec, capture-codec */
	mach->cpu.of_node = of_parse_phandle(np, "cpu-dai", 0);
	if (!mach->cpu.of_node) {
		dev_err(&pdev->dev, "Missing cpu-dai phandle\n");
		return -EPROBE_DEFER;  // defer until cpu-dai ready
	}
	mach->cpu.dai_name = "opencores-i2s";  // set your CPU dai name here

	mach->codec_playback.of_node = of_parse_phandle(np, "playback-codec", 0);
	if (!mach->codec_playback.of_node) {
		dev_err(&pdev->dev, "Missing playback-codec phandle\n");
		ret = -EPROBE_DEFER;  // defer until playback codec ready
		goto err_put_cpu;
	}
	mach->codec_playback.dai_name = "HiFi";  // set playback codec dai name here

	mach->codec_capture.of_node = of_parse_phandle(np, "capture-codec", 0);
	if (!mach->codec_capture.of_node) {
		dev_err(&pdev->dev, "Missing capture-codec phandle\n");
		ret = -EPROBE_DEFER;  // defer until capture codec ready
		goto err_put_codec_playback;
	}
	mach->codec_capture.dai_name = "ics43432-hifi";  // set capture codec dai name here

	/* platform: use cpu node for platform too (common for simple boards) */
	mach->platform.of_node = mach->cpu.of_node;
	mach->platform.dai_name = "opencores-i2s";  // usually platform dai_name matches CPU dai_name
//	mach->platform.dai_name = mach->cpu.dai_name;  // usually platform dai_name matches CPU dai_name

	/* build dai links array */
	links = mach->dai_links;

	/* Playback link (link 0) */
	links[0].name = "I2S-Playback";
	links[0].stream_name = "Playback";
	links[0].id = 0;
	links[0].cpus = &mach->cpu;
	links[0].num_cpus = 1;
	links[0].codecs = &mach->codec_playback;
	links[0].num_codecs = 1;
	links[0].platforms = &mach->platform;
	links[0].num_platforms = 1;
	links[0].playback_only = true;
	links[0].capture_only = false;
	links[0].dai_fmt = SND_SOC_DAIFMT_I2S |
			   SND_SOC_DAIFMT_NB_NF |
			   SND_SOC_DAIFMT_CBC_CFC;

	/* Capture link (link 1) */
	links[1].name = "I2S-Capture";
	links[1].stream_name = "Capture";
	links[1].id = 1;
	links[1].cpus = &mach->cpu;
	links[1].num_cpus = 1;
	links[1].codecs = &mach->codec_capture;
	links[1].num_codecs = 1;
	links[1].platforms = &mach->platform;
	links[1].num_platforms = 1;
	links[1].playback_only = false;
	links[1].capture_only = true;
	links[1].dai_fmt = SND_SOC_DAIFMT_I2S |
			   SND_SOC_DAIFMT_NB_NF |
			   SND_SOC_DAIFMT_CBC_CFC;

	mach->card.dai_link = links;
	mach->card.num_links = ARRAY_SIZE(mach->dai_links);

	platform_set_drvdata(pdev, mach);

	ret = devm_snd_soc_register_card(&pdev->dev, &mach->card);
	if (ret) {
		dev_err(&pdev->dev, "devm_snd_soc_register_card failed: %d\n", ret);
		goto err_put_codec_capture;
	}

	dev_info(&pdev->dev, "opencores-i2s machine registered\n");
	return 0;

err_put_codec_capture:
	of_node_put(mach->codec_capture.of_node);
err_put_codec_playback:
	of_node_put(mach->codec_playback.of_node);
err_put_cpu:
	of_node_put(mach->cpu.of_node);
	return ret;
}


static void oc_i2s_card_remove(struct platform_device *pdev)
{
	struct oc_i2s_mach *mach = platform_get_drvdata(pdev);

	if (!mach)
		return;

	of_node_put(mach->cpu.of_node);
	of_node_put(mach->codec_playback.of_node);
	of_node_put(mach->codec_capture.of_node);

	return;
}

static const struct of_device_id oc_i2s_of_match[] = {
	{ .compatible = "briansune,i2s-mic-amp", },
	{ }
};
MODULE_DEVICE_TABLE(of, oc_i2s_of_match);

static struct platform_driver oc_i2s_driver = {
	.driver = {
		.name = "oc-i2s-machine",
		.of_match_table = oc_i2s_of_match,
	},
	.probe = oc_i2s_card_probe,
	.remove = oc_i2s_card_remove,
};
module_platform_driver(oc_i2s_driver);

MODULE_AUTHOR("BrianSune & ChatGPT-5");
MODULE_DESCRIPTION("ASoC machine driver: opencores I2S (playback=max98357a, capture=ics43432)");
MODULE_LICENSE("GPL v2");

