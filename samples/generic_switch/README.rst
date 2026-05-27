.. |matter_name| replace:: Edge AI generic switch
.. |matter_type| replace:: sample
.. |matter_dks_thread| replace:: ``nrf54lm20dk/nrf54lm20b/cpuapp`` board target
.. |sample path| replace:: :file:`samples/generic_switch`

.. include:: /includes/matter/shortcuts.txt

.. _matter_edge_ai_generic_switch_sample:

Matter Edge AI generic switch
###########################

.. contents::
   :local:
   :depth: 2

This sample demonstrates a voice-controlled Matter switch built with Nordic's Edge AI Axon-based inference.
It captures audio from a PDM microphone, detects a wakeword, recognizes keywords in a command, and sends the corresponding Matter OnOff action to bound devices.

Application overview
********************

The application runs a two-stage inference pipeline:

* Wakeword detection that arms command recognition.
* Keyword spotting that recognizes commands and maps them to Matter actions.

The wakeword is ``OKAY NORDIC``.

After wakeword detection, the application opens a short keyword detection window (configured with :kconfig:option:`CONFIG_KEYWORD_DETECTION_TIMEOUT_S`).
Recognized commands trigger the following actions:

* ``TOGGLE`` - Toggles the all Switch endpoints.
# ``SCENE_ONE`` - Toggles the first Switch endpoint.
# ``SCENE_TWO`` - Toggles the second Switch endpoint.
# ``SCENE_THREE`` - Toggles the third Switch endpoint.
# ``SCENE_FOUR`` - Toggles the fourth Switch endpoint.

Requirements
************

The application supports the following development kit:

.. table-from-sample-yaml::

.. include:: /includes/matter/requirements/thread_wifi.txt

Programming requirements
========================

To commission the device and control remote lights over a Matter network, you need a Matter controller :ref:`configured on PC or smartphone <ug_matter_configuring>`.
The application requires a PDM digital microphone connected according to the selected overlay file.

Pin mapping
===========

The application was tested with an `Adafruit PDM MEMS Microphone`_ module.
It can be powered from 1.8V ``VDD:IO`` supply.
The following table shows how to connect this module to the DK:

.. list-table::
   :header-rows: 1

   * - Adafruit DMIC
     - nRF54LM20 DK
   * - ``3V``
     - ``VDD:IO``
   * - ``GND``
     - ``GND``
   * - ``SEL``
     - ``GND``
   * - ``CLK``
     - ``P1.4``
   * - ``DAT``
     - ``P1.5``

The ``SEL`` pin selects the audio channel.
Connecting ``SEL`` to ground selects the left channel.

To use other microphones, adapt configuration parameters of PDM in the :file:`src/dmic.c` file.

User interface
***************

LEDs:
   When PWM support is available, the application uses a dimming effect to indicate the active keyword detection window after wakeword detection.

UART30:
   Prints runtime messages for state transitions and command execution.
   Typical messages include:

   * ``Waiting for wakeword...``
   * ``wakeword detected, Looking for keywords``

   The UART baudrate is set to ``1000000``.

Configuration
*************

.. include:: /includes/matter/configuration/intro.txt

The following microphone overlays are provided:

* :file:`boards/dmic_adafruit.dtsi`
* :file:`boards/dmic_low_power.dtsi`

Advanced configuration options
==============================

.. include:: /includes/matter/configuration/advanced/intro.txt

Building and running
********************

.. include:: /includes/matter/building_and_running/intro.txt

|matter_ble_advertising_auto|

Select microphone overlay with extra CMake arguments.

For example:

.. code-block:: console

   west build -b nrf54lm20dk/nrf54lm20b/cpuapp -- -DEXTRA_DTC_OVERLAY_FILE=boards/dmic_adafruit.dtsi

.. code-block:: console

   west build -b nrf54lm20dk/nrf54lm20b/cpuapp -- -DEXTRA_DTC_OVERLAY_FILE=boards/dmic_low_power.dtsi

Testing
*******

.. include:: /includes/matter/testing/intro.txt

Testing with CHIP Tool
======================

#. |connect_kit|
#. |connect_terminal_kit|
#. In Light Switch mode, complete commissioning and binding with a compatible Matter light device.
#. Speak the wakeword phrase ``OKAY NORDIC`` and see if the LEDs on the DK start blinking smoothly.
#. Within the keyword detection window (until the LEDs stop blinking), speak one of the supported commands (for example, ``TOGGLE``).
#. Verify command execution in terminal logs and on the bound light device behavior.

Testing with commercial ecosystem
=================================

.. include:: /includes/matter/testing/ecosystem.txt

Dependencies
************

.. include:: /includes/matter/dependencies.txt

In addition, this application uses the following |EAI| component:

* :ref:`nrf_edgeai_lib`
