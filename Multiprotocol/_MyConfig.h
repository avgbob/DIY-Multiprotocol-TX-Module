/*
 * Joe's Q X7 / iRangeX IRX4 Plus build profile.
 *
 * Goal:
 *   - Keep the current MultiModule AIR protocol set available.
 *   - Keep serial mode, telemetry, normal Bind workflow, bootloader support.
 *   - Add the experimental SAYZON F35 SGF22 subtype.
 *   - Drop only features not needed by a Q X7 using the external module bay.
 *
 * This intentionally mirrors the upstream STM32 AIR release strategy:
 * MULTI_AIR + serial-only (PPM input disabled) to fit the 128KB STM32F103CB.
 */

#define MULTI_AIR

// Q X7 talks to the external MultiModule using the serial protocol.
#undef ENABLE_PPM
