// We expose constant values as macros so that all constants are literals
// An advantage for doing this is that we can use concat!() on these values

#[macro_export]
macro_rules! LOGFILE {
    () => {
        "/cache/magisk.log"
    };
}
