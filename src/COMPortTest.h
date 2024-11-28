#pragma once
/* COMPortTest.h : Header file for COMPortTest.cpp
 * Date: Nov 2024
 * Author:
 *         Gobind Matharu
 */

#include "COMPort.h"

/// @brief Tests the COMPort transmit and receive functionality.
/// @param txPortName The transmitter COM port name (e.g., "COM3").
/// @param rxPortName The receiver COM port name (e.g., "COM4").
void testCOMPort(const char* txPortName, const char* rxPortName);

