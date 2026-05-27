/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

// Please refer to https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces
constexpr const uint8_t kNamespaceCommon   = 7;
constexpr const uint8_t kNamespaceSwitches = 0x43;

constexpr const uint8_t kNamespaceCommonLevel = 5;
// Common Number Namespace: 5, tag 0 (Low)
constexpr const uint8_t kTagCommonLow = 0;
// Common Number Namespace: 5, tag 1 (Medium)
constexpr const uint8_t kTagCommonMedium = 1;
// Common Number Namespace: 5, tag 2 (High)
constexpr const uint8_t kTagCommonHigh = 2;

constexpr const uint8_t kNamespaceCommonNumber = 7;
// Common Number Namespace: 7, tag 0 (Zero)
constexpr const uint8_t kTagCommonZero = 0;
// Common Number Namespace: 7, tag 1 (One)
constexpr const uint8_t kTagCommonOne = 1;
// Common Number Namespace: 7, tag 2 (Two)
constexpr const uint8_t kTagCommonTwo = 2;

constexpr const uint8_t kNamespacePosition = 8;
// Common Position Namespace: 8, tag: 0 (Left)
constexpr const uint8_t kTagPositionLeft = 0;
// Common Position Namespace: 8, tag: 1 (Right)
constexpr const uint8_t kTagPositionRight = 1;
// Common Position Namespace: 8, tag: 2 (Top)
constexpr const uint8_t kTagPositionTop = 2;
// Common Position Namespace: 8, tag: 3 (Bottom)
constexpr const uint8_t kTagPositionBottom = 3;
// Common Position Namespace: 8, tag: 4 (Middle)
constexpr const uint8_t kTagPositionMiddle = 4;
// Common Position Namespace: 8, tag: 5 (Row)
constexpr const uint8_t kTagPositionRow = 5;
// Common Position Namespace: 8, tag: 6 (Column)
constexpr const uint8_t kTagPositionColumn = 6;

// Switches namespace: 0x43, tag = 0x03 (Up)
constexpr const uint8_t kTagSwitchesUp = 0x03;
// Switches namespace: 0x43, tag = 0x04 (Down)
constexpr const uint8_t kTagSwitchesDown = 0x04;
