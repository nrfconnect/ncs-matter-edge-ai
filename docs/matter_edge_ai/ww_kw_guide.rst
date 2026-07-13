.. _matter_edge_ai_ww_kw_guide:

Wakeword and keyword spotting integration
#########################################

.. contents::
   :local:
   :depth: 2

The wakeword and keyword spotting (WW/KW) pipeline lets a Matter application capture audio from a digital microphone, detect a wakeword, recognize spoken keywords, and dispatch commands to application logic.
Reusable inference, state machine, and DMIC handling are located in :file:`samples/common`. 
Your application supplies exported Edge AI models, keyword class definitions, Matter actions, and board-specific setup.

Integration prerequisites
*************************

Before you integrate the WW/KW pipeline into a Matter application, make sure that the following prerequisites are completed:

* :ref:`Requirements and setup <matter_edge_ai_setup>` for the |addon|, including an |NCS| workspace with the Matter Edge AI west manifest.
* A supported development kit with a PDM microphone (see the :ref:`matter_edge_ai_generic_switch_sample` sample for a supported board target).
* A Matter application that you can extend with sample-specific source files and Kconfig options.
* Wakeword and keyword spotting models exported from `Nordic Edge AI Lab`_.
  Each export provides the generated C sources and headers described in :ref:`matter_edge_ai_ww_kw_add_models`.

Solution architecture
*********************

The WW/KW pipeline runs as a dedicated Zephyr thread (:cpp:class:`EdgeAITask`) in :file:`samples/common/nrf_edgeai_ww_kw_task.cpp`.
The thread captures PCM audio from the DMIC, runs wakeword inference until a wakeword is detected, opens a keyword detection window (duration set by the :kconfig:option:`CONFIG_MATTER_EDGEAI_KEYWORD_DETECTION_TIMEOUT_S` Kconfig option), and runs keyword spotting inference until a command is confirmed.

Generic code in :file:`samples/common` handles DMIC capture, model inference loops, and the wakeword-to-keyword state machine.
Your application provides:

* Exported Edge AI models (wakeword and keyword spotting).
* Keyword class definitions and per-class detection tuning.
* Matter actions and board-specific setup (PDM gain, LED feedback, and similar).

The following diagram shows the runtime data flow:

.. code-block:: none

   DMIC capture
        │
        ▼
   EdgeAITask thread (common)
        │
        ├─► ww_process() ──► wakeword_app_model_get() ──► generated WW model
        │         │
        │         └─► wakeword detected ──► EdgeAIWwKwOnWakewordDetected()
        │
        └─► kw_process() ──► keyword_app_model_get() ──► generated KWS model
                  │
                  └─► command confirmed ──► EdgeAIWwKwHandleKeyword(class)

Matter-facing work must be scheduled on the Matter thread (for example with ``SystemLayer().ScheduleLambda(...)``), as shown in the reference :file:`nrf_edgeai_ww_kw_impl.cpp`.

Integration steps
*****************

The WW/KW integration consists of the following steps:

1. :ref:`Adding exported models <matter_edge_ai_ww_kw_add_models>`
#. :ref:`Binding the wakeword model <matter_edge_ai_ww_kw_wakeword>`
#. :ref:`Defining keyword classes <matter_edge_ai_ww_kw_keyword_classes>`
#. :ref:`Implementing keyword application hooks <matter_edge_ai_ww_kw_keyword_impl>`
#. :ref:`Implementing task callbacks <matter_edge_ai_ww_kw_task_callbacks>`
#. :ref:`Enabling Kconfig options <matter_edge_ai_ww_kw_kconfig>`

These steps are described in the following sections.
The reference implementation is the :ref:`Matter Edge AI generic switch <matter_edge_ai_generic_switch_sample>` sample (:file:`samples/generic_switch`).

.. _matter_edge_ai_ww_kw_add_models:

.. rst-class:: numbered-step

Adding exported models
======================

Each application that uses WW/KW ships two models exported from `Nordic Edge AI Lab`_.
Place the generated files under your sample source tree (for example :file:`src/models/wakeword/` and :file:`src/models/kws/`).

Each export includes the following files:

* :file:`nrf_edgeai_user_model.h`
* :file:`nrf_edgeai_user_model.c`
* :file:`nrf_edgeai_user_model_axon.h`
* :file:`nrf_edgeai_user_types.h`

Add the generated sources to your application :file:`CMakeLists.txt` and include the generated headers only from the application binding files (:file:`wakeword_impl.c` and :file:`keyword_impl.c`), not from :file:`samples/common`.

.. _matter_edge_ai_ww_kw_wakeword:

.. rst-class:: numbered-step

Binding the wakeword model
==========================

The common :file:`wakeword.c` module does not call the generated functions directly.
It calls the application hook :c:func:`wakeword_app_model_get`, which you implement in :file:`wakeword_impl.c`.

Your generated wakeword header must expose at least:

.. code-block:: c

   nrf_edgeai_t *nrf_edgeai_user_model_ww(void);
   uint32_t nrf_edgeai_user_model_neuton_size_ww(void);

The reference :file:`wakeword_impl.c` returns the model pointer:

.. code-block:: c

   struct nrf_edgeai *wakeword_app_model_get(void)
   {
       return nrf_edgeai_user_model_ww();
   }

.. note::
   If your Lab export uses different function names (not ``nrf_edgeai_user_model_ww``), update :file:`wakeword_impl.c` accordingly.
   Only :file:`wakeword_impl.c` needs to know the generated function names.

.. _matter_edge_ai_ww_kw_keyword_classes:

.. rst-class:: numbered-step

Defining keyword classes
========================

Define the keyword class enum in :file:`keyword_app.h`.
Each enumerator index must match the output class index of your keyword spotting model exported from Nordic Edge AI Lab.

Example from the generic switch sample:

.. code-block:: c

   typedef enum keyword_labels_e {
       KEYWORD_LIGHT = 0,
       KEYWORD_OFF = 1,
       KEYWORD_ON = 2,
       KEYWORD_OTHER = 3,
       KEYWORD_SCENE_FOUR = 4,
       KEYWORD_SCENE_ONE = 5,
       KEYWORD_SCENE_THREE = 6,
       KEYWORD_SCENE_TWO = 7,
       KEYWORD_SILENCE = 8,
       KEYWORD_TOGGLE_LIGHT = 9,
       KEYWORDS_cnt
   } keyword_labels_t;

Use :file:`keyword_app.h` anywhere you need readable class constants (for example in :file:`nrf_edgeai_ww_kw_impl.cpp`).

.. _matter_edge_ai_ww_kw_keyword_impl:

.. rst-class:: numbered-step

Implementing keyword application hooks
======================================

Similarly, :file:`keyword.c` calls :c:func:`keyword_app_model_get` from your :file:`keyword_impl.c`.

The generated keyword spotting header must expose at least:

.. code-block:: c

   nrf_edgeai_t *nrf_edgeai_user_model_kws(void);
   uint32_t nrf_edgeai_user_model_neuton_size_kws(void);

Reference binding:

.. code-block:: c

   struct nrf_edgeai *keyword_app_model_get(void)
   {
       return nrf_edgeai_user_model_kws();
   }

Implement all :c:func:`keyword_app_` hooks in :file:`keyword_impl.c`.
Typical contents:

1. ``static const keyword_class_cfg_t s_keyword_classes_cfg[]`` — detection tuning per class:

   * ``name`` — debug label.
   * ``count_needed`` — consecutive stable frames required before accepting the class.
   * ``threshold_percent`` — minimum average probability (0 = disabled).

2. Model getter calling the generated ``nrf_edgeai_user_model_kws()``.

3. Rules for :c:func:`keyword_app_is_command_component` and :c:func:`keyword_app_is_actionable`.

Only classes for which :c:func:`keyword_app_is_actionable` returns ``true`` can produce a :c:func:`kw_process` return value of ``1``.

.. _matter_edge_ai_ww_kw_task_callbacks:

.. rst-class:: numbered-step

Implementing task callbacks
===========================

Implement the four callbacks declared in :file:`nrf_edgeai_ww_kw_task.h` in :file:`nrf_edgeai_ww_kw_impl.cpp`:

.. list-table::
   :header-rows: 1

   * - Callback
     - When called
     - Typical application work
   * - :c:func:`EdgeAIWwKwHandleKeyword`
     - Keyword spotting command confirmed
     - Map ``keyword_class`` to Matter actions
   * - :c:func:`EdgeAIWwKwOnWakewordDetected`
     - Wakeword accepted, keyword window opens
     - Start LED dimming or other user feedback
   * - :c:func:`EdgeAIWwKwOnKeywordSessionEnd`
     - Keyword window closes after a command
     - Stop feedback effects
   * - :c:func:`EdgeAIWwKwAppInit`
     - After DMIC trigger, before audio loop
     - PDM gain, dimming module init, board setup

.. _matter_edge_ai_ww_kw_kconfig:

.. rst-class:: numbered-step

Enabling Kconfig options
========================

Enable the following Kconfig options in your application project:

* :kconfig:option:`CONFIG_FPU` — Enable Floating Point Unit.
* :kconfig:option:`CONFIG_NEWLIB_LIBC` — Enable Newlib C library.
* :kconfig:option:`CONFIG_MATTER_EDGEAI` — Enable Matter Edge AI.
* :kconfig:option:`CONFIG_MATTER_EDGEAI_DMIC` — Enable Matter Edge AI DMIC.

For the full list of add-on and sample options, see :ref:`matter_edge_ai_samples_config`.

Configuration
*************

This section describes the configuration options for the sample.

|config|

Optional modules
================

The following optional modules in :file:`samples/common` extend the WW/KW pipeline:

* :kconfig:option:`CONFIG_MATTER_EDGEAI_DIMMING` — Enable dimming support once a keyword is detected.

Commonly tuned options
======================

The following options are commonly tuned in the application project:

* :kconfig:option:`CONFIG_MATTER_EDGEAI_KEYWORD_DETECTION_TIMEOUT_S` — Timeout for keyword detection in seconds after recognizing the wakeword.
* :kconfig:option:`CONFIG_MATTER_EDGEAI_TASK_THREAD_STACK_SIZE` — Stack size for the Edge AI wakeword/keyword task thread.
* :kconfig:option:`CONFIG_MATTER_EDGEAI_TASK_THREAD_PRIORITY` — Priority for the Edge AI wakeword/keyword task thread.
* :kconfig:option:`CONFIG_MATTER_EDGEAI_WW_KW_LOG_LEVEL` — Log level for the Edge AI wakeword/keyword task and model processing modules.

Wakeword tuning options
=======================

* :kconfig:option:`CONFIG_MATTER_EDGEAI_WAKEWORD_PROBABILITY_THRESHOLD` — Per-frame probability threshold (0–1000; default 990 corresponds to 0.99).
* :kconfig:option:`CONFIG_MATTER_EDGEAI_WAKEWORD_HISTORY_SIZE` — Sliding window length for detections.
* :kconfig:option:`CONFIG_MATTER_EDGEAI_WAKEWORD_COUNT_THRESHOLD` — Number of above-threshold frames required to confirm a wakeword.

Applications and samples
************************

The following sample demonstrates the WW/KW integration in the |addon|:

* :ref:`Matter Edge AI generic switch <matter_edge_ai_generic_switch_sample>` — Voice-controlled Matter generic switch using wakeword and keyword spotting (:file:`samples/generic_switch`).

Library support
***************

The WW/KW pipeline is implemented by shared sources under :file:`samples/common`:

* :file:`nrf_edgeai_ww_kw_task.cpp` / :file:`nrf_edgeai_ww_kw_task.h` — :cpp:class:`EdgeAITask` thread, DMIC trigger, and application callback dispatch.
* :file:`wakeword.c` / :file:`wakeword.h` — wakeword inference loop and detection state machine.
* :file:`keyword.c` / :file:`keyword.h` — keyword spotting inference and class confirmation.
* :file:`dmic.c` / :file:`dmic.h` — DMIC capture for the Edge AI audio pipeline.
* :file:`dimming_effect.cpp` / :file:`dimming_effect.h` — optional LED dimming feedback (requires :kconfig:option:`CONFIG_MATTER_EDGEAI_DIMMING`).

Include :file:`matter_edge_ai_common.cmake` from your sample :file:`CMakeLists.txt` to add these sources to the build when :kconfig:option:`CONFIG_MATTER_EDGEAI` is enabled.

Dependencies
************

The WW/KW integration depends on the following:

* `Edge AI add-on`_ — nRF Edge AI library and Axon-based inference used by the generated models.
* `Nordic Edge AI Lab`_ — training and export of wakeword and keyword spotting models.
* Matter support in the |NCS| — the host Matter application and threading model (see `Matter architecture`_).
