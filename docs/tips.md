# Tips and Tricks
## Remove Files
How to efficiently remove a file systemless-ly? To actually make the file **disappear** within the folder is complicated (possible, not worth the effort). **Replacing it with a dummy file should be good enough**! Create an empty file with the same name and place it in the same path within a module, then you're basically done!

## Remove Folders
Same as mentioned above, actually making the folder to **disappear** is not worth the effort. **Replacing it with an empty folder should be good enough**! A handy trick for module developers using [Magisk Module Template](https://github.com/topjohnwu/magisk-module-template) is to add the folder you want to remove into the `REPLACE` list within `config.sh`. If your module doesn't provide a correspond folder, it will create an empty folder, and automatically add `.replace` into the empty folder so the dummy folder will properly replace the one in `/system`.
