################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
main_lnk.obj: ../main_lnk.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/Program Files (x86)/TI/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/bin/cl430" -vmsp --abi=eabi --use_hw_mpy=none --include_path="C:/Program Files (x86)/TI/ccsv6/ccs_base/msp430/include" --include_path="C:/Data/TTU/AAED13/5. IAY0030 Sardsysteemid/Labor/MSP430G2553_MRF89XA_Transmitter_Example_Project/LNK" --include_path="C:/Data/TTU/AAED13/5. IAY0030 Sardsysteemid/Labor/MSP430G2553_MRF89XA_Transmitter_Example_Project/NWK" --include_path="C:/Data/TTU/AAED13/5. IAY0030 Sardsysteemid/Labor/MSP430G2553_MRF89XA_Transmitter_Example_Project/Config" --include_path="C:/Data/TTU/AAED13/5. IAY0030 Sardsysteemid/Labor/MSP430G2553_MRF89XA_Transmitter_Example_Project/Drivers" --include_path="C:/Data/TTU/AAED13/5. IAY0030 Sardsysteemid/Labor/MSP430G2553_MRF89XA_Transmitter_Example_Project/Drivers/Communication" --include_path="C:/Program Files (x86)/TI/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/include" --advice:power=all -g --define=__MSP430G2553__ --diag_warning=225 --diag_wrap=off --display_error_number --printf_support=minimal --preproc_with_compile --preproc_dependency="main_lnk.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


