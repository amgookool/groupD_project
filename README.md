# Pulse Rate and SPO2 Measurement on ESP8266

## Resources

- [How to design a better Pulse Oximeter](https://www.analog.com/en/technical-articles/how-to-design-a-better-pulse-oximeter.html)

- [DIY Pulse Oximeter](https://www.instructables.com/DIY-Pulse-Oximeter/)

- [How to Design a better Pulse Oximeter: Specification](https://www.embedded.com/how-to-design-a-better-pulse-oximeter-specifications/)

- [How to design a better pulse oximeter: Implementation](https://www.embedded.com/how-to-design-a-better-pulse-oximeter-implementation/)

- [Design and Fabrication of a Pulse Oximeter](https://www.researchgate.net/publication/335058012_Design_and_Fabrication_of_a_Pulse_Oximeter)

- [Pulse Oximeter](https://www.researchgate.net/publication/278730850_Design_and_implementation_of_a_Pulse_Oximeter)

## System Requirements

1. The system should have two modes/states:

   - Measurement

   - Reporting

1. Measurement Mode Requirements:

   1. The system should measure a pulse rate (PR) between the range of 60 bpm - 100 bpm. ([Source 1 for Elderly Person](https://www.healthtap.com/q/normal-pulse-rate-elderly/)) ([Source 2 for Elderly Person BPM](https://healthfully.com/normal-pulse-rate-senior-citizens-5454202.html))

      - The system should have a fixed time for switching on and off the LED and Infrared Light.

      - The system should read both the AC and DC signals from LED driving circuit

      - The system should use a timer for a counter to count the number of ticks.

      - The system should calculate the bpm based on the number of ticks counted.

   1. The system should estimate the user's oxygen saturation level as a percentage(95% and above). ([Oxygen Saturation](https://www.ncbi.nlm.nih.gov/books/NBK525974/)) ([Elderly Person Oxygen Saturation](https://www.griswoldhomecare.com/blog/2021/august/oxygen-levels-in-elderly-adults-when-to-be-conce/))

   1. The system should transmit the measurement reading to serial terminal once reading is complete.

   1. The system should trigger the buzzer if measurement reading is hazardous to elderly persons

      - The buzzer should trigger if pulse rate is below 60 bpm.

      - The buzzer should trigger if oxygen saturation level is below 95 %

1. Reporting Mode Requirements:

   1. The system should allow the user to view previous (stored) measurements.

   1. The system should allow input from user via serial interface (UART).

      - The system should allow input from user to set the intervals at which measurements are stored.

      - The system should allow input from user to set the maximum number of measurements stored.

      - The system should allow input from user to set the storage format of the measurement data.

1. General Requirements:

   1. The system should store measurements in non-volatile memory.

   1. The system should be able to switch between measurement and reporting mode.

   1. The system should operate on a 9V battery.

   1. The system should allow the user to turn off the system without losing measurement readings.

   1. The system should be designed for a maximum power consumption of 100mA at any point in time.

   1. The system should be able to produce a precise measurement of the pulse rate using an accuracy of ± 5%.
