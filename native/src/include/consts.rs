// Expose constant strings as macros so that we can use concat!() on these values

#[macro_export]
macro_rules! LOGFILE {
    () => {
        "/cache/magisk.log"
    };
}
