.. _matter_edge_ai_samples_config:

Configuration options
#####################

.. contents::
   :local:
   :depth: 2

This page lists the Kconfig options provided by the |addon|.

To use the add-on in a Matter application, enable the ``CONFIG_MATTER_EDGEAI`` Kconfig option.
This option pulls in the nRF Edge AI library and the integration layer between Matter and Edge AI.

The options are grouped as follows:

* **Add-on options** — Top-level options that enable and configure Matter Edge AI support.
* **Sample configuration options** — Shared options used by the Matter Edge AI samples for wakeword and keyword spotting, DMIC settings, and related features.

|config|
|kconfig_search|

.. _matter_edge_ai_samples_kconfig:

Add-on Kconfig options
**********************

.. options-from-kconfig:: /../Kconfig
   :show-type:

.. option:: CONFIG_FPU

``(bool)`` Enable hardware Floating Point Unit (FPU) support.

This option enables use of the hardware floating point unit (FPU) if present on the target device. 
When enabled, the system will make use of the hardware FPU during context switching, and FPU registers 
will be preserved as part of the thread context. Applications can then use floating point operations 
in C code and assembly, provided the underlying hardware supports it.

Note: Enabling the FPU incurs additional overhead in interrupt and thread context switches due to 
saving and restoring the extra registers. Not all targets have a hardware FPU; enabling this on 
hardware without FPU support will have no effect or may cause build errors.

.. option:: CONFIG_NEWLIB_LIBC

``(bool)`` Build with the Newlib C library.

Build with the Newlib library. The Newlib library is expected to be part of the SDK in this case.


Sample configuration options
****************************

.. options-from-kconfig:: /../samples/common/Kconfig
   :show-type:
