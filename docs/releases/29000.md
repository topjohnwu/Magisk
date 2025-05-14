## 2025.5.14 Magisk v29.0

This release looks minor at the surface, however, the entire codebase has gone through significant refactoring and migration. The native code in Magisk used to be mainly C++, but several contributors and I have been steadily rewriting parts of the code in Rust since April 2022. After years of effort, the Rust-ification of the project slowly began picking up steam, and at the moment of this release, over 40% of the native code has been rewritten in Rust, with several major subsystem rewrites in the PR queue, planned to be merged for the next release.

Many might wonder, why introduce a new language to the project? My reason is actually not to reduce memory safety issues (although it is a nice side benefit), but to be able to develop Magisk using a more modern programming language. After using Rust for a while, it's clear to me that using Rust allows me to write more correct code and makes me happier compared to dealing with C++. People share the [same sentiment as I do](https://threadreaderapp.com/thread/1577667445719912450.html).

## Changelog

- [General] Massive internal refactoring and code migration
- [App] Support downloading module zip files with XZ compression
- [App] Disable app animations when system animations are disabled
- [MagiskMount] Support systemlessly deleting files with modules using blank file nodes
- [MagiskInit] Redesign sepolicy patching and injection logic
- [MagiskSU] Better TTY/PTY support

### Full Changelog: [here](https://topjohnwu.github.io/Magisk/changes.html)
