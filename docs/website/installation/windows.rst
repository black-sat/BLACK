Installation on Windows
========================

Binary package
---------------

.. note:: 
   The latest version (v0.10.8) is currently unavailable on Windows.
   The following instructions are for version v0.10.7

On Windows, we provide a binary package in the form of a `zip` file containing
the `black` command-line tool, the `libblack.dll` library, and all the needed
header files and import libraries. No installation is needed, the `zip` archive
is self-contained.

.. list-table::

   * - |WindowsBadge|_

.. |WindowsBadge| image:: https://badgen.net/badge/Download%20v0.10.7/.zip/green
.. _WindowsBadge: https://github.com/black-sat/black/releases/download/v0.10.7/black-0.10.7-win-x64.zip


The binary package includes the Z3 backend. To use other backends, BLACK needs
to be built from source. Instructions from :doc:`Linux <linux>` can be easily
adapted, excepting that dependencies must be installed by hand.