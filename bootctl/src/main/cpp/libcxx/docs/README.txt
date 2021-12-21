libc++ Documentation
====================

The libc++ documentation is written using the Sphinx documentation generator. It is
currently tested with Sphinx 1.1.3.

To build the documents into html configure libc++ with the following cmake options:

  * -DLLVM_ENABLE_SPHINX=ON
  * -DLIBCXX_INCLUDE_DOCS=ON

After configuring libc++ with these options the make rule `docs-libcxx-html`
should be available.
