.. _matter_edge_ai_ww_kw_guide:

Wakeword and keyword spotting integration guide
#################################################

.. contents::
   :local:
   :depth: 2

This guide describes how to integrate the generic Matter Edge AI wakeword and keyword spotting (WW/KW) pipeline into a Matter application.
It explains the split between reusable code in :file:`samples/common` and application-specific glue in your sample directory.

The reference implementation is the :ref:`Matter Edge AI generic switch <matter_edge_ai_generic_switch_sample>` sample (:file:`samples/generic_switch`).

Overview
********

The WW/KW pipeline runs as a dedicated Zephyr thread (:cpp:class:`EdgeAITask`) that:

1. Captures PCM audio from the DMIC.
2. Runs wakeword inference until a wakeword is detected.
3. Opens a keyword detection window (duration set by :kconfig:option:`CONFIG_KEYWORD_DETECTION_TIMEOUT_S`).
4. Runs keyword spotting inference and dispatches recognized commands to the application.

Generic inference, state machine, and DMIC handling live in :file:`samples/common`.
Your application provides:

* Exported Edge AI models (wakeword and KWS).
* Keyword class definitions and detection tuning.
* Matter actions and board-specific setup (PDM gain, LED feedback, and similar).

Runtime data flow
******************

The following diagram shows the data flow through the WW/KW pipeline:

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

Models
******

Each application which uses WW/KW ships two models exported from `Nordic Edge AI Lab`_.
The models includes the followig files:

* :file:`nrf_edgeai_user_model.h`
* :file:`nrf_edgeai_user_model.c`
* :file:`nrf_edgeai_user_model_axon.h`
* :file:`nrf_edgeai_user_types.h`

Wakeword model
==============

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
   If your Lab export uses a different file names (not ``nrf_edgeai_user_model_ww``), update :file:`wakeword_impl.c` accordingly.
   Only :file:`wakeword_impl.c` needs to know the generated function names.

Keyword spotting model
======================

Similarly, :file:`keyword.c` calls :c:func:`keyword_app_model_get` from your :file:`keyword_impl.c`.

The generated KWS header file must expose at least:

.. code-block:: c

   nrf_edgeai_t *nrf_edgeai_user_model_kws(void);
   uint32_t nrf_edgeai_user_model_neuton_size_kws(void);

Reference binding:

.. code-block:: c

   struct nrf_edgeai *keyword_app_model_get(void)
   {
       return nrf_edgeai_user_model_kws();
   }

Application header: :file:`keyword_app.h`
=========================================

Define the keyword class enum in this header.
Each enumerator index must match the output class index of your KWS model exported from Nordic Edge AI Lab.

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

Application file: :file:`keyword_impl.c`
==========================================

Implement all :c:func:`keyword_app_` hooks in this file.
Typical contents:

1. ``static const keyword_class_cfg_t s_keyword_classes_cfg[]`` — detection tuning per class:

   * ``name`` — debug label.
   * ``count_needed`` — consecutive stable frames required before accepting the class.
   * ``threshold_percent`` — minimum average probability (0 = disabled).

2. Model getter calling the generated ``nrf_edgeai_user_model_kws()``.

3. Rules for :c:func:`keyword_app_is_command_component` and :c:func:`keyword_app_is_actionable`.

Only classes for which :c:func:`keyword_app_is_actionable` returns ``true`` can produce a :c:func:`kw_process` return value of ``1``.

Task integration (:file:`nrf_edgeai_ww_kw_impl.cpp`)
====================================================

Implement the four callbacks declared in :file:`nrf_edgeai_ww_kw_task.h`:

.. list-table::
   :header-rows: 1

   * - Callback
     - When called
     - Typical application work
   * - :c:func:`EdgeAIWwKwHandleKeyword`
     - KWS command confirmed
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

Configuration
*************

The following options are required to be enabled in the application project to support the WW/KW functionality:

* :kconfig:option:`CONFIG_FPU` - enable Floating Point Unit.
* :kconfig:option:`CONFIG_NEWLIB_LIBC` - enable Newlib C library.
* :option:`CONFIG_MATTER_EDGEAI` - enable Matter Edge AI.
* :option:`CONFIG_MATTER_EDGEAI_DMIC` - enable Matter Edge AI DMIC.

Optional modules
================

In the common directory, there are following optional modules:

* :option:`CONFIG_MATTER_EDGEAI_DIMMING` - enable dimming support once the keyword is detected.

Commonly tuned options
======================

The following options are commonly tuned in the application project:

* :option:`CONFIG_MATTER_EDGEAI_KEYWORD_DETECTION_TIMEOUT_S` - timeout for keyword detection in seconds after recognizing the wakeword.
* :option:`CONFIG_MATTER_EDGEAI_TASK_THREAD_STACK_SIZE` - stack size for the Edge AI wakeword/keyword task thread.
* :option:`CONFIG_MATTER_EDGEAI_TASK_THREAD_PRIORITY` - priority for the Edge AI wakeword/keyword task thread.
* :option:`CONFIG_MATTER_EDGEAI_WW_KW_LOG_LEVEL` - log level for the Edge AI wakeword/keyword task and model processing modules.

Wakeword tuning Kconfig options
================================

* :option:`CONFIG_MATTER_EDGEAI_WAKEWORD_PROBABILITY_THRESHOLD` — per-frame probability threshold (0-1000, default 990 makes it 0.99).
* :option:`CONFIG_MATTER_EDGEAI_WAKEWORD_HISTORY_SIZE` — sliding window length for detections.
* :option:`CONFIG_MATTER_EDGEAI_WAKEWORD_COUNT_THRESHOLD` — number of above-threshold frames required to confirm a wakeword.
