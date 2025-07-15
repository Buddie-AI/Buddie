
        CLOCK_CACHE_L1_CODE_START = .;
        *(.clock.text.cache.L1)
        CLOCK_CACHE_L1_SIZE = ABSOLUTE(. - CLOCK_CACHE_L1_CODE_START);
	
	*(.vm.text.cache.L1)
	
		*(.power_driver.text.cache.L1)
        *(.timer.text.cache.L1)
        *(.gpio.text.cache.L1)
        *(.iic_hw.text.cache.L1)
