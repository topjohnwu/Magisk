macro(split_list listname)
  string(REPLACE ";" " " ${listname} "${${listname}}")
endmacro()
