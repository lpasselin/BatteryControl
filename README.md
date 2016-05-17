# Battery Control Documentation

#Introduction

Over my short 4 month work term at McMaster University Automotive Resource Centre, I was tasked to interface a battery control system amongst different IC manufacturers.
More specifically, I was tasked to interface the DC2100A with the TMS320F28377D development board via SPI (serial peripheral interface) bus.
I had no previous experience in this field of embedded programming before and thought it would be a great way for me to learn and expose myself the embedded world.
As a disclaimer, I take no claim that this project is optimal in how I implemented it and that I treated it as a learning experience while meeting deadlines throughout the term. 

#Purpose

Generally TI boards come with Simulink interfaces so that low level coding would be unnecessary for the project. However, due to how new the TI320F28377D development board was at the time, 
no such interface was implemented as of yet. As a result, I was tasked to create the 'high-level' interface with the board so that my team could implement battery balancing strategies to his
battery modules. In our specific case, we were interested in balancing 18650 batteries tied to the DC2100A board.

#Firmware Design

The purpose of my firmware design was to set up the required registries and code functionality for my colleague to implement high level battery balancing algorithms.
The firmware consisted of SPI bus communications from the TMS320F28377D to the LTC3300 and LTC6804 ICs. In order for the TI chip to communicate to such boards, specific SPI signals 
were generated depending on the type of balancing algorithm wanted. In our case, we were most interested in using the LTC3300 chips since they would allow for active battery balancing throughout
the batteries. The LTC3300 chips would require a special set of SPI instructions and PEC redundancy check signal protocols in order for the chip to implement correctly work. 

My initial approach to the SPI bus communication between ICs was to consistently poll data across the bus and IC to ensure IC compatibility. Eventually however, 
I was able to utilize timer interrupts which would allow for more efficient hardware utilization and effectively implement parallel balancing strategies to the board.
As a result, I developed high level modularity code for the TI 320F28377D board and efficient device usage with hardware interrupt conditions.



