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

Sample configuration options
****************************

.. options-from-kconfig:: /../samples/common/Kconfig
   :show-type:
