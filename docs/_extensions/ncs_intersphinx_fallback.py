# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

"""Resolve unqualified :ref: targets against an NCS docset via Intersphinx."""

from __future__ import annotations

from sphinx.application import Sphinx
from sphinx.environment import BuildEnvironment
from sphinx.ext.intersphinx import missing_reference as intersphinx_missing_reference


def missing_reference(app: Sphinx, env: BuildEnvironment, node, contnode):
    fallback_docset = app.config.ncs_intersphinx_fallback_docset
    if not fallback_docset:
        return None

    if node.get("reftype") not in {"ref", "numref"}:
        return None

    target = node.get("reftarget", "")
    if not target or ":" in target:
        return None

    prefixed = node.deepcopy()
    prefixed["reftarget"] = f"{fallback_docset}:{target}"
    return intersphinx_missing_reference(app, env, prefixed, contnode)


def setup(app: Sphinx):
    app.add_config_value("ncs_intersphinx_fallback_docset", "nrf", True)
    app.connect("missing-reference", missing_reference)

    return {
        "version": "1.0",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
