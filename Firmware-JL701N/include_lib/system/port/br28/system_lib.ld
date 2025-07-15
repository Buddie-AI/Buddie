SECTIONS
{
    .data : ALIGN(4)
    {
		. = ALIGN(4);
        system_data_start = .;

		_video_subdev_begin = .;
		PROVIDE(video_subdev_begin = .);
		KEEP(*(.video_subdev.0))
		KEEP(*(.video_subdev.1))
		KEEP(*(.video_subdev.2))
		KEEP(*(.video_subdev.3))
		KEEP(*(.video_subdev.4))
		KEEP(*(.video_subdev.5))
		_video_subdev_end = .;
		PROVIDE(video_subdev_end = .);

		_audio_subdev_begin = .;
		PROVIDE(audio_subdev_begin = .);
		KEEP(*(.audio_subdev.0))
		KEEP(*(.audio_subdev.1))
		KEEP(*(.audio_subdev.2))
		KEEP(*(.audio_subdev.3))
		_audio_subdev_end = .;
		PROVIDE(audio_subdev_end = .);

		_iic_device_begin = .;
		PROVIDE(iic_device_begin = .);
		KEEP(*(.iic))
		_iic_device_end = .;
		PROVIDE(iic_device_end = .);

        _avin_spi_device_begin = .;
		PROVIDE(avin_spi_device_begin = .);
		KEEP(*(.sw_spi))
		_avin_spi_device_end = .;
		PROVIDE(avin_spi_device_end = .);

		_video_dev_begin = .;
		PROVIDE(video_dev_begin = .);
		KEEP(*(.video_device))
		_video_dev_end = .;
		PROVIDE(video_dev_end = .);

		_key_driver_ops_begin = .;
		PROVIDE(key_driver_ops_begin = .);
		KEEP(*(.key_driver_ops))
		_key_driver_ops_end = .;
		PROVIDE(key_driver_ops_end = .);

		_touch_driver_begin = .;
		PROVIDE(touch_driver_begin = .);
		KEEP(*(.touch_driver))
		_touch_driver_end = .;
		PROVIDE(touch_driver_end = .);

		_static_hi_timer_begin = .;
		PROVIDE(static_hi_timer_begin = .);
		KEEP(*(.hi_timer))
		_static_hi_timer_end = .;
		PROVIDE(static_hi_timer_end = .);

		_sys_cpu_timer_begin = .;
		PROVIDE(sys_cpu_timer_begin = .);
		KEEP(*(.sys_cpu_timer))
		_sys_cpu_timer_end = .;
		PROVIDE(sys_cpu_timer_end = .);

	    _sys_config_begin = .;
	    PROVIDE(sys_config_begin = .);
	    KEEP(*(.sys_cfg))
	    _sys_config_end = .;
	    PROVIDE(sys_config_end = .);

	    _sys_fat_begin = .;
	    PROVIDE(sys_fat_begin = .);
	    KEEP(*(.fs_fat))
	    _sys_fat_end = .;
	    PROVIDE(sys_fat_end = .);

		_app_begin = .;
		PROVIDE(app_begin = .);
		KEEP(*(.app))
		_app_end = .;
		PROVIDE(app_end = .);

	    *(.crypto_ecdh_data)
	    *(.crypto_data)

	    *(.mem_data)
        *(.os_port_data)

		*(.os_str)
        *(.os_data)

	    *(.uECC_data)
	    *(.ECDH_sample_data)

         __movable_slot_start = .;
         *(movable.slot.*);
         __movable_slot_end = .;

        system_data_end = .;

    } > ram0

    .bss (NOLOAD) :ALIGN(4)
    {
        system_bss_start = .;
        . = ALIGN(4);
        *(.os_bss)
        *(.mem_heap)
		*(.memp_memory_x)
        *(.mem_bss)
        *(.os_port_bss)

        *(.uECC_bss)
        *(.ECDH_sample_bss)

        system_bss_end = .;

    } > ram0

    .text : ALIGN(4)
    {
		. = ALIGN(4);
        system_text_start = .;

		 _device_node_begin = .;
   		PROVIDE(device_node_begin = .);
   		KEEP(*(.device))
   		_device_node_end = .;
   		PROVIDE(device_node_end = .);

		config_target_begin = .;
   		PROVIDE(config_target_begin = .);
   		KEEP(*(.config_target))
		config_target_end = .;
   		PROVIDE(config_target_end = .);

	    system_code_begin = .;
	    KEEP(*(.system.*.text))
	    system_code_end = .;
		. = ALIGN(4);
		system_code_size = system_code_end - system_code_begin;

		vfs_ops_begin = .;
		KEEP(*(.vfs_operations))
		vfs_ops_end = .;

	    _lib_version_begin = .;
	    PROVIDE(lib_version_begin = .);
	    KEEP(*(.lib_version))
	    _lib_version_end = .;
	    PROVIDE(lib_version_end = .);

		_initcall_begin = .;
		PROVIDE(initcall_begin = .);
	    KEEP(*(.initcall))
		_initcall_end = .;
		PROVIDE(initcall_end = .);

		_early_initcall_begin = .;
		PROVIDE(early_initcall_begin = .);
		KEEP(*(.early.initcall))
		_early_initcall_end = .;
		PROVIDE(early_initcall_end = .);

		_late_initcall_begin = .;
		PROVIDE(late_initcall_begin = .);
		KEEP(*(.late.initcall))
		_late_initcall_end = .;
		PROVIDE(late_initcall_end = .);

		_platform_initcall_begin = .;
		PROVIDE(platform_initcall_begin = .);
		KEEP(*(.platform.initcall))
		_platform_initcall_end = .;
		PROVIDE(platform_initcall_end = .);

		_module_initcall_begin = .;
		PROVIDE(module_initcall_begin = .);
		KEEP(*(.module.initcall))
		_module_initcall_end = .;
		PROVIDE(module_initcall_end = .);

		_sys_event_handler_begin = .;
		PROVIDE(sys_event_handler_begin = .);
		KEEP(*(.sys_event.4.handler))
		KEEP(*(.sys_event.3.handler))
		KEEP(*(.sys_event.2.handler))
		KEEP(*(.sys_event.1.handler))
		KEEP(*(.sys_event.0.handler))
		_sys_event_handler_end = .;
		PROVIDE(sys_event_handler_end = .);

		_syscfg_arg_begin = .;
		PROVIDE(syscfg_arg_begin = .);
		KEEP(*(.syscfg.arg))
		_syscfg_arg_end = .;
		PROVIDE(syscfg_arg_end = .);

		_syscfg_handler_begin = .;
		PROVIDE(syscfg_handler_begin = .);
		KEEP(*(.syscfg.handler))
		_syscfg_handler_end = .;
		PROVIDE(syscfg_handler_end = .);

		_syscfg_ops_begin = .;
		PROVIDE(syscfg_ops_begin = .);
		KEEP(*(.syscfg.2.ops))
		KEEP(*(.syscfg.1.ops))
		KEEP(*(.syscfg.0.ops))
		_syscfg_ops_end = .;
		PROVIDE(syscfg_ops_end = .);

		_server_info_begin = .;
		PROVIDE(server_info_begin = .);
		KEEP(*(.server_info))
		_server_info_end = .;
		PROVIDE(server_info_end = .);

		_bus_device_begin = .;
		PROVIDE(bus_device_begin = .);
		KEEP(*(.bus_device))
		_bus_device_end = .;
		PROVIDE(bus_device_end = .);

	    _sys_power_hal_ops_begin = .;
	    PROVIDE(sys_power_hal_ops_begin = .);
	    KEEP(*(.sys_power_hal_ops))
	    _sys_power_hal_ops_end = .;
	    PROVIDE(sys_power_hal_ops_end = .);



	    crypto_begin = .;
	    *(.crypto_ecdh_code)
	    *(.crypto_ecdh_const)

	    *(.crypto_bigint_code)
	    *(.crypto_bigint_const)

	    *(.crypto_code)
	    *(.crypto_const)

	    *(.ECDH_sample_code)
	    *(.ECDH_sample_const)

	    *(.uECC_code)
	    *(.uECC_const)

	    *(.hmac_code)
	    *(.hmac_const)

	    *(.hash_sample_code)
	    *(.hash_sample_const)

	    *(.aes_cmac_sample_code)
	    *(.aes_cmac_sample_const)
	    crypto_end = .;
	    crypto_size = . - crypto_begin;

        *(.mem_code)
        *(.mem_const)

        *(.os_port_code)
        *(.os_port_const)

        *(.os_const)

        *(.math_fast_funtion_code)

        __movable_function_start = .;
        *(movable.text.*);
        *(movable.stub.*);
        *(movable.region.*);
        /* *(.movable.code*) */
        __movable_function_end = .;
        __movable_function_size = __movable_function_end - __movable_function_start;

        system_text_end = .;


        system_code_total_size = system_text_end - system_text_start;
    } > code0

	.data_code ALIGN(32):
	{
		*(.os_code)
    } > ram0
}

