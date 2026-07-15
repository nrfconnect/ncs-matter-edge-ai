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
It captures audio from a PDM microphone, detects a wakeword, recognizes keywords in a spoken command, and sends the corresponding Matter Switch action to a Matter controller.

.. note::
   This sample in the |addon| for the |NCS| is provided as an experimental feature.
   See `Software maturity levels`_ in the |NCS| documentation for details.

   Currently, the solution may not be power-optimized.
   Wakeword and keyword recognition are also experimental.
   For reliable testing, use the synthetic audio recordings described in the :ref:`Synthetic test audio <generic_switch_synthetic_test_audio>` section.

Requirements
************

The sample supports the following development kit:

.. table-from-sample-yaml::

.. include:: /includes/thread.txt

Hardware setup
==============

The application requires a PDM digital microphone connected according to the selected devicetree overlay.

For development, two microphone options are supported, each with a dedicated :file:`.dtsi` overlay in the project:

* `Adafruit PDM`_ — Adafruit PDM MEMS microphone module that you connect to the DK with jumper wires (see pin mapping below)
* `lowPower PDM`_ — Low-power PDM MEMS microphone shield that can be attached directly to the nRF54LM20 DK

Pin mapping
-----------

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

To use other microphones, adapt the PDM configuration parameters in :file:`samples/common/dmic.c` and add the corresponding :file:`.dtsi` overlay file to the project.

Overview
********

.. include:: /includes/sleep_thread_sed_sit.txt

The application runs a two-stage inference pipeline:

* Wakeword detection that arms command recognition.
* Keyword spotting that recognizes commands and maps them to Matter actions.

The wakeword phrase is ``OKAY NORDIC``.

After wakeword detection, the application opens a short keyword detection window.
The window duration is configured with :option:`CONFIG_MATTER_EDGEAI_KEYWORD_DETECTION_TIMEOUT_S`.

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

Commercial Matter ecosystems typically expose the Switch cluster in momentary mode with short press, long press, and multiple press actions.
Each action can be assigned to a single steering function in the ecosystem app.

.. note::
   This sample implements a *generic switch* (Switch cluster in momentary mode) rather than a Matter light switch with direct binding to a light bulb.
   Most commercial Matter ecosystems do not support binding a light switch endpoint directly to a light bulb device.
   The generic switch model lets you assign switch press actions to ecosystem automations instead.

To support toggling a light from one voice command, the sample maps short press to *on* and long press to *off* on Endpoint 1.
The ``TOGGLE_LIGHT`` keyword alternates between those two actions.

The sample exposes five momentary switch endpoints.
Endpoint *N* (where *N* > 1) maps to Scene *N* − 1 in the ecosystem:

* Endpoint 1 — Short and long press actions for toggle on/off control.
* Endpoint 2 — Short press action assignable to Scene 1.
* Endpoint 3 — Short press action assignable to Scene 2.
* Endpoint 4 — Short press action assignable to Scene 3.
* Endpoint 5 — Short press action assignable to Scene 4.

For details on the wakeword and keyword spotting integration, see :ref:`matter_edge_ai_ww_kw_guide`.

User interface
**************

.. include:: /includes/interface/intro.txt

LED 0:
   .. include:: /includes/interface/state_led.txt

Button 0:
   .. include:: /includes/interface/main_button.txt

All LEDs:
   When PWM support is available, the application uses a dimming effect to indicate the active keyword detection window after wakeword detection.

Button 1:
   Pressing this button toggles the bound light using the same short/long press mapping as the ``TOGGLE_LIGHT`` keyword.

UART30:
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

|config|

The following microphone overlays are provided:

* :file:`boards/dmic_adafruit.dtsi` for the `Adafruit PDM`_ module.
* :file:`boards/dmic_low_power.dtsi` for the `lowPower PDM`_ module.

You can adjust the keyword detection window duration with the :option:`CONFIG_MATTER_EDGEAI_KEYWORD_DETECTION_TIMEOUT_S` option.

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

.. _generic_switch_synthetic_test_audio:

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

Play a synthetic wakeword recording from the :file:`sounds` directory near the PDM microphone.
Verify that the LEDs on the DK start blinking smoothly and that the UART log shows ``wakeword detected, Looking for keywords``.

.. rst-class:: numbered-step

Play the synthetic keyword command
----------------------------------

Play one of the synthetic keyword recordings.
Verify that the command is executed in the terminal logs.

For ``TOGGLE_LIGHT``, bind Endpoint 1 to a light in your Matter ecosystem or observe the corresponding Switch action in CHIP Tool.
You can also press **the Button 1** to trigger the same toggle behavior without using voice commands.

Testing with a commercial ecosystem
===================================

.. include:: /includes/testing/ecosystem.txt

After commissioning, configure the switch endpoints in your ecosystem application.
The exact steps depend on the ecosystem UI, but the general workflow is:

1. Open the device settings for the commissioned switch in the ecosystem app.
2. Assign steering functions to the switch actions exposed by each endpoint—for example, short press, long press, and multiple press.
3. For Endpoint 1, bind short press to turn a light on and long press to turn it off (to support the ``TOGGLE_LIGHT`` keyword).
4. For Endpoints 2–5, assign short press actions to scenes or other automations as needed.

After configuration, test the setup:

1. Play a synthetic wakeword and keyword recording near the microphone, or press **Button 1** for the toggle action.
2. Verify that the bound light or scene responds as configured in the ecosystem app.

Dependencies
************

In addition, this application uses the reusable wakeword and keyword spotting code in :file:`samples/common` and Edge AI models exported from Nordic Edge AI Lab.
See :ref:`matter_edge_ai_ww_kw_guide` for integration details.
