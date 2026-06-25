.. |matter_name| replace:: Edge AI generic switch
.. |matter_type| replace:: sample
.. |matter_dks_thread| replace:: ``nrf54lm20dk/nrf54lm20b/cpuapp`` board target
.. |sample path| replace:: :file:`samples/generic_switch`
.. |matter_qr_code_payload| replace:: MT:K.K9042C00KA0648G00
.. |matter_pairing_code| replace:: 34970112332
.. |matter_qr_code_image| image:: /images/matter_qr_code_generic_switch.png
                          :width: 200px
                          :alt: QR code for commissioning the generic switch device

.. _matter_edge_ai_generic_switch_sample:

Matter Edge AI generic switch
#############################

.. contents::
   :local:
   :depth: 2

This sample demonstrates a voice-controlled Matter generic switch built with `Edge AI add-on`_ Axon-based inference.
It captures audio from a PDM microphone, detects a wakeword, recognizes keywords in a spoken command, and sends the corresponding Matter Generic Switch action to a Matter controller.

.. note::
   This sample in the |addon| for the |NCS| is provided as an experimental feature.
   See `Software maturity levels`_ in the |NCS| documentation for what experimental support means.

   The solution may not be power-optimized yet.

   Wakeword and keyword recognition are also experimental.
   For reliable testing, play synthetic audio recordings near the microphone instead of speaking live.
   Example recordings for the supported wakeword and keywords are available in the :file:`sounds` directory.

.. include:: /includes/sleep_thread_sed_sit.txt

Requirements
************

The sample supports the following development kit:

.. table-from-sample-yaml::

.. include:: /includes/thread.txt

Programming requirements
========================

To commission the device and control remote lights over a Matter network, you need a Matter controller `configured on PC or smartphone <Testing Matter in the NCS_>`_.
The application requires a PDM digital microphone connected according to the selected devicetree overlay.

For development, two microphone options are supported, each with a dedicated :file:`.dtsi` overlay in the project:

* `Adafruit PDM`_ — Adafruit PDM MEMS microphone module
* `lowPower PDM`_ — Low-power PDM MEMS microphone module that can be attached to the nRF54LM20 DK

Pin mapping
===========

The application supports the `Adafruit PDM`_ module.
It can be powered from the 1.8 V ``VDD:IO`` supply.
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

To use other microphones, adapt the PDM configuration parameters in :file:`src/dmic.c` and add the corresponding :file:`.dtsi` overlay file to the project.

Overview
********

The application runs a two-stage inference pipeline:

* Wakeword detection that arms command recognition.
* Keyword spotting that recognizes commands and maps them to Matter actions.

The wakeword phrase is ``OKAY NORDIC``.

After wakeword detection, the application opens a short keyword detection window.
The window duration is configured with :kconfig:option:`CONFIG_MATTER_EDGEAI_KEYWORD_DETECTION_TIMEOUT_S`.

Supported voice commands
========================

The following keywords are recognized after the wakeword:

* ``TOGGLE_LIGHT`` — Toggles a bound light by alternating short and long press actions on Endpoint 1.
* ``SCENE_ONE`` — Sends a short press action on Endpoint 2.
* ``SCENE_TWO`` — Sends a short press action on Endpoint 3.
* ``SCENE_THREE`` — Sends a short press action on Endpoint 4.
* ``SCENE_FOUR`` — Sends a short press action on Endpoint 5.

Switch endpoints
================

Commercial Matter ecosystems typically expose the Generic Switch cluster in momentary mode with short press, long press, and multiple press actions.
Each action can be assigned to a single steering function in the ecosystem app.

To support toggling a light from one voice command, the sample maps short press to *on* and long press to *off* on Endpoint 1.
The ``TOGGLE_LIGHT`` keyword alternates between those two actions.

The sample exposes five momentary switch endpoints:

* **Endpoint 1** — Short and long press actions for toggle on/off control.
* **Endpoint 2** — Short press action assignable to Scene 1.
* **Endpoint 3** — Short press action assignable to Scene 2.
* **Endpoint 4** — Short press action assignable to Scene 3.
* **Endpoint 5** — Short press action assignable to Scene 4.

For details on the wakeword and keyword spotting integration, see :ref:`matter_edge_ai_ww_kw_guide`.

User interface
**************

.. include:: /includes/interface/intro.txt

.. include:: /includes/interface/interface.txt

.. include:: /includes/interface/interface_table.txt

First LED:
   .. include:: /includes/interface/state_led.txt

First Button:
   .. include:: /includes/interface/main_button.txt

All LEDs
   When PWM support is available, the application uses a dimming effect to indicate the active keyword detection window after wakeword detection.

Second Button:
   Pressing this button toggles the bound light using the same short/long press mapping as the ``TOGGLE_LIGHT`` keyword.

UART30
   Prints runtime messages for state transitions and command execution.
   Typical messages include:

   * ``Waiting for wakeword...``
   * ``wakeword detected, Looking for keywords``

   The UART baud rate is set to ``115200``.

.. include:: /includes/interface/segger_usb.txt
.. include:: /includes/interface/nfc.txt

Configuration
*************

.. include:: /includes/configuration/intro.txt

The following microphone overlays are provided:

* :file:`boards/dmic_adafruit.dtsi` for the `Adafruit PDM`_ module.
* :file:`boards/dmic_low_power.dtsi` for the `lowPower PDM`_ module.

Building and running
********************

.. include:: /includes/building_and_running/intro.txt

|matter_ble_advertising_auto|

Select the microphone overlay with extra CMake arguments.

For example:

.. code-block:: console

   west build -b nrf54lm20dk/nrf54lm20b/cpuapp -- -DEXTRA_DTC_OVERLAY_FILE=boards/dmic_adafruit.dtsi

.. code-block:: console

   west build -b nrf54lm20dk/nrf54lm20b/cpuapp -- -DEXTRA_DTC_OVERLAY_FILE=boards/dmic_low_power.dtsi

Testing
*******

.. include:: /includes/testing/intro.txt

Synthetic test audio
====================

Instead of speaking the wakeword and keywords live, use the synthetic recordings in the :file:`sounds` directory at the repository root.
Play a file near the PDM microphone (for example, from a phone or laptop speaker at moderate volume).
Each recording contains the wakeword phrase followed by a supported keyword.

The following recordings are provided:

.. list-table::
   :header-rows: 1

   * - Recording
     - Keyword
   * - :file:`sounds/OkayNordicToggleLight.mp3`
     - ``TOGGLE_LIGHT``
   * - :file:`sounds/OkayNordicSceneOne.mp3`
     - ``SCENE_ONE``
   * - :file:`sounds/OkayNordicSceneTwo.mp3`
     - ``SCENE_TWO``
   * - :file:`sounds/OkayNordicSceneThree.mp3`
     - ``SCENE_THREE``
   * - :file:`sounds/OkayNordicSceneFour.mp3`
     - ``SCENE_FOUR``

Testing with CHIP Tool
======================

.. |node_id| replace:: 1

.. include:: /includes/testing/1_prepare_matter_network_thread.txt
.. include:: /includes/testing/2_prepare_dk.txt
.. include:: /includes/testing/3_commission_thread.txt

.. rst-class:: numbered-step

Play the synthetic wakeword command
-----------------------------------

Verify that the LEDs on the DK start blinking smoothly.

.. rst-class:: numbered-step

Play the synthetic keyword command
----------------------------------

Play one of the synthetic keyword commands.
Verify that the command is executed in the terminal logs and on the bound light device.

Testing with a commercial ecosystem
===================================

.. include:: /includes/testing/ecosystem.txt

Dependencies
************

In addition, this application uses the reusable wakeword and keyword spotting code in :file:`samples/common` and Edge AI models exported from Nordic Edge AI Lab.
See :ref:`matter_edge_ai_ww_kw_guide` for integration details.
