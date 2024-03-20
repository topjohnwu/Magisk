// Expose constant strings as macros so that we can use concat!() on these values

#[macro_export]
macro_rules! LOGFILE {
    () => {
        "/cache/magisk.log"
    };
}

#[macro_export]
macro_rules! INTLROOT {
    () => {
        ".magisk"
    };
}

#[macro_export]
macro_rules! LOG_PIPE {
    () => {
        concat!($crate::INTLROOT!(), "/device/log")
    };
}

#[macro_export]
macro_rules! MAIN_CONFIG {
    () => {
        concat!($crate::INTLROOT!(), "/config")
    };
}
