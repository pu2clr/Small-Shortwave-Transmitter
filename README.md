# Shortwave Arduino Transmiter

## IT IS UNDER CONSTRUCTION...

It is a shortwave trasmitter with SI5351 oscillator from Silicon Labs controlled by Arduino.


This project is originally based  on the [Stefan0719](https://youtu.be/7fe_GlJI5WI) project that uses a crystal oscillator. Instead of a crystal oscillator, this project uses a SI5351 signal generator tha can be controlled by Arduino. The idea is to be able to transmit on any frequency in the HF band (3 ˜ 30 MHz).

Recently I have been developing some Arduino libraries to control DSP receivers. The main motivation for building this shortwave transmitter is to be able to do experiments and tests during the development of the library. This is because my location does not allow good reception of shortwave broadcast stations most of the time. 

The list below shows the Arduino Libraries I have developed to control DSP receivers/ 

1. [PU2CLR Si4735 Library for Arduino](https://pu2clr.github.io/SI4735/). This library was built based on “Si47XX PROGRAMMING GUIDE; AN332” and it has support to FM, AM and SSB modes (LW, MW and SW). It also can be used on all members of the SI47XX family respecting, of course, the features available for each IC version;
2. [PU2CLR SI4844 Arduino Library](https://github.com/pu2clr/SI4844). This is an Arduino library for the SI4844, BROADCAST ANALOG TUNING DIGITAL DISPLAY AM/FM/SW RADIO RECEIVER,  IC from Silicon Labs.  It is available on Arduino IDE. This library is intended to provide an easier interface for controlling the SI4844.
3. [PU2CLR AKC695X Arduino Library](https://pu2clr.github.io/AKC695X/). The AKC695X is a family of IC DSP receiver from AKC technology. The AKC6955 and AKC6959sx support AM and FM modes. On AM mode the AKC6955 and AKC6959sx work on LW, MW and SW. On FM mode they work from 64MHz to 222MHz.
4. [PU2CLR KT0915 Arduino Library](https://pu2clr.github.io/KT0915/). The KT0915 is a full band AM (LW, MW and SW) and FM DSP receiver that can provide you a easy way to build a high quality radio with low cost.



