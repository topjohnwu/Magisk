SECTIONS {
  .init_array : {
    *(SORT_BY_INIT_PRIORITY(.init_array.*))
    *(EXCLUDE_FILE(*crtend_android.o) .init_array)
  }
} INSERT AFTER .fini_array;
SECTIONS {
  .fini_array : {
    *(SORT_BY_INIT_PRIORITY(.fini_array.*))
    *(EXCLUDE_FILE(*crtend_android.o) .fini_array)
  }
} INSERT BEFORE .init_array;
