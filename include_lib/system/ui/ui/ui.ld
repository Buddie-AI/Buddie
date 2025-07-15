   	    lcd_interface_begin = .;
        KEEP(*(.lcd_if_info))
        lcd_interface_end = .;

		ui_style_begin = .;
		KEEP(*(.ui_style))
		ui_style_end = .;


        elm_event_handler_begin_JL = .;
        KEEP(*(.elm_event_handler_JL))
		elm_event_handler_end_JL = .;

        elm_event_handler_begin_UPGRADE = .;
        KEEP(*(.elm_event_handler_UPGRADE))
        elm_event_handler_end_UPGRADE = .;



		elm_event_handler_begin_DIAL = .;
        KEEP(*(.elm_event_handler_DIAL))
		elm_event_handler_end_DIAL = .;


		control_event_handler_begin = .;
        KEEP(*(.control_event_handler))
		control_event_handler_end = .;

        control_ops_begin = .;
        KEEP(*(.control_ops))
        control_ops_end = .;

        battery_notify_begin = .;
        *(.battery_notify)
        battery_notify_end = .;


