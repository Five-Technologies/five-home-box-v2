drop table OPTION;

create table OPTION(
    id_option int auto_increment,
    label varchar(255) not null,
    type varchar(255) not null,
    read_only bool,
    description varchar(255),

    primary key (id_option),
    unique (label)
);

-- door/window
insert into OPTION (label, type) values ("basic", "int");
insert into OPTION (label, type) values ("sensor", "bool");
insert into OPTION (label, type) values ("zwave+_version", "int");
insert into OPTION (label, type) values ("installer_icon", "int");
insert into OPTION (label, type) values ("user_icon", "int");
insert into OPTION (label, type) values ("configuring_the_off_delay", "int");
insert into OPTION (label, type) values ("basic_set_level", "int");
insert into OPTION (label, type) values ("access_control", "char");
insert into OPTION (label, type) values ("previous_event_cleared": "int");
insert into OPTION (label, type) values ("loaded_config_revision", "int");
insert into OPTION (label, type) values ("config_file_revision", "int");
insert into OPTION (label, type) values ("latest_available_config_file_revision": "int");
insert into OPTION (label, type) values ("device_id", "char");
insert into OPTION (label, type) values ("powerlevel", "char");
insert into OPTION (label, type) values ("timeout", "int");
insert into OPTION (label, type) values ("set_powerlevel", "bool");
insert into OPTION (label, type) values ("test_node", "int");
insert into OPTION (label, type) values ("test_powerlevel", "char");
insert into OPTION (label, type) values ("frame_count", "int");
insert into OPTION (label, type) values ("test", "bool");
insert into OPTION (label, type) values ("report", "bool");
insert into OPTION (label, type) values ("test_status", "char");
insert into OPTION (label, type) values ("acked_frames", "int");
insert into OPTION (label, type) values ("battery_level", "int");
insert into OPTION (label, type) values ("wake-up_interval", "int");
insert into OPTION (label, type) values ("minimum_wake-up_interval", "int");
insert into OPTION (label, type) values ("maximum_wake-up_interval", "int");
insert into OPTION (label, type) values ("default_wake-up_interval", "int");
insert into OPTION (label, type) values ("wake-up_interval_step", "int");
insert into OPTION (label, type) values ("library_version", "int");
insert into OPTION (label, type) values ("protocol_version", "float");
insert into OPTION (label, type) values ("application_version", "float");
-- thermostat
insert into OPTION (label, type) values ("air_temperature", "int");
insert into OPTION (label, type) values ("air_temperature_units", "char");
insert into OPTION (label, type) values ("mode", "char");
insert into OPTION (label, type) values ("operating_state", "char");
insert into OPTION (label, type) values ("cooling_1", "float");
insert into OPTION (label, type) values ("day", "char");
insert into OPTION (label, type) values ("hour", "int");
insert into OPTION (label, type) values ("minute", "int");
insert into OPTION (label, type) values ("level", "int");
insert into OPTION (label, type) values ("bright", "bool");
insert into OPTION (label, type) values ("dim", "bool");
insert into OPTION (label, type) values ("ignore_start_level", "bool");
insert into OPTION (label, type) values ("start_level", "int");
insert into OPTION (label, type) values ("diming_duration", "int");
insert into OPTION (label, type) values ("switch_all", "char");
insert into OPTION (label, type) values ("scene", "int");
insert into OPTION (label, type) values ("duration", "int");
insert into OPTION (label, type) values ("color", "char");
insert into OPTION (label, type) values ("color_index", "char");
insert into OPTION (label, type) values ("color_channels", "int");
insert into OPTION (label, type) values ("status_after_power_failure", "char");
insert into OPTION (label, type) values ("notification_when_load_status_change", "char");
insert into OPTION (label, type) values ("enable/disable_the_function_of_using_wall_switch", "char");
insert into OPTION (label, type) values ("advance_mode", "char");
insert into OPTION (label, type) values ("resetting_to_factory_default", "char");
insert into OPTION (label, type) values ("serial_number", "char");
-- rgb
-- pir
insert into OPTION (label, type) values ("illuminance_units", "char");
insert into OPTION (label, type) values ("motion_detection_sensitivity", "char");
insert into OPTION (label, type) values ("motion_detection_on_time", "int");
insert into OPTION (label, type) values ("ambient_illumination_lux_level", "int");
insert into OPTION (label, type) values ("motiotn_detection_blind_time", "int");
insert into OPTION (label, type) values ("illumination_report_interval", "int");
insert into OPTION (label, type) values ("illumination_function", "char");
insert into OPTION (label, type) values ("illumination_report_threshold", "int");
insert into OPTION (label, type) values ("ambient_temperature_differential_level_report_or_led_blink_enable", "int");
insert into OPTION (label, type) values ("motion_event_report_ont_time_enable", "char");
insert into OPTION (label, type) values ("ambient_light_intensity_calibration", "int");
insert into OPTION (label, type) values ("home_security", "char");
