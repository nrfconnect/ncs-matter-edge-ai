.. _matter_edge_ai_setup:

Requirements and setup
######################

.. contents::
   :local:
   :depth: 2

This page outlines the requirements that you need to meet before you start working with the |addon|.

Hardware requirements
*********************

To use the |addon|, you need a development kit that supports the Matter protocol and Edge AI capabilities:

.. table-from-sample-yaml::

Software requirements
*********************

For libraries and code for the |addon|, see the `Matter Edge AI add-on`_ repository.

To work with the |addon|, you need to install the |NCS|, including all its prerequisites and the |NCS| toolchain.
Follow the `Installing the nRF Connect SDK`_ instructions.

The |addon| is distributed as a Git repository and is managed through its own west manifest.
It also pulls in the compatible `Edge AI add-on`_ (version |edge-ai_version|) as a west module.

Getting the add-on code
========================

Assuming you have an existing |NCS| workspace in the :file:`ncs` folder, run the following commands:

1. Navigate to the workspace folder:

   .. code-block:: console

      cd ncs

#. Clone the add-on repository:

   .. code-block:: console

      git clone https://github.com/nrfconnect/ncs-matter-edge-ai

#. Set the manifest path to the add-on directory:

   .. code-block:: console

      west config manifest.path ncs-matter-edge-ai

#. Update the |NCS| modules:

   .. code-block:: console

      west update

#. Optionally, run these commands in case you need to go back to work on the nRF Connect SDK without the add-on:

   a. Configure the manifest path back to the nRF Connect SDK directory

      .. code-block:: console

         west config manifest.path nrf

   #. Update nRF Connect SDK modules

      .. code-block:: console

         west update
   
   #. Check the current manifest path with the following command:

      .. code-block:: console

         west config manifest.path

      The output should be:

      .. code-block:: console

         nrf

      This means that the current workspace is using the nRF Connect SDK.
