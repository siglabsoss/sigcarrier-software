#!/bin/bash

amixer sset "DAC L1 Mux" "IF1 DAC1"
amixer sset "DAC R1 Mux" "IF1 DAC1"
amixer sset "DAC1 MIXL DAC1" on
amixer sset "DAC1 MIXR DAC1" on
amixer sset "Stereo DAC MIXL DAC L1" on
amixer sset "Stereo DAC MIXR DAC R1" on
amixer sset "DAC L1 Source" "Stereo DAC Mixer"
amixer sset "DAC R1 Source" "Stereo DAC Mixer"
amixer sset "HPO L" on
amixer sset "HPO R" on
amixer sset "RECMIX1L BST2" on
amixer sset "RECMIX1R BST2" on
amixer sset "Stereo1 ADC Source" "ADC1"
amixer sset "Stereo1 ADC1 Source" "ADC"
amixer sset "Stereo1 ADC MIXL ADC1" on
amixer sset "Stereo1 ADC MIXR ADC1" on
amixer sset "DAC1" 80%
amixer sset "IN1 Boost" 80%
amixer sset "IN2 Boost" 80%
amixer sset "Mono ADC" 80%
