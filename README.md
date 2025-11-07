# Cyclone V SoC ALSA MUX

## If this project is constructive, welcome to donate a drink to PayPal.

<img src="images/qrcode.png" style="height:20%; width:20%">

or 

paypal.me/briansune

# Introduction

This project aim to support split capture and playback devices when utilizing this [ALSA driver](https://github.com/briansune/Cyclone-V-SoC-ALSA).

This work opens a better flexibilities on the Codec devcies, the default driver can only support unified Codec.

With this work the I2S can be shared by Codecs w/o wasting FPGA fabric resources and HPS DMA channels.

This driver can also applied to other FPSoC.

# Developers and Assistants

1) BrianSune

2) ChatGPT 5

3) ChatGPT 4

# Device Tree Example

```
	sound {
		compatible = "briansune,i2s-mic-amp";
		cpu-dai = <&i2s>;
		playback-codec = <&max98357a>;
		capture-codec  = <&ics43432>;
	};

	ics43432: ics43432 {
		compatible = "invensense,ics43432";
		#sound-dai-cells = <0>;
	};

	max98357a: max98357a {
		compatible = "maxim,max98357a";
		#sound-dai-cells = <0>;
		sdmode-gpios = <&led_pio 0 GPIO_ACTIVE_HIGH>;
	};
```
