/*
 *  camera.ino - One minute interval time-lapse Camera
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  This is a test app for the camera library.
 *  This library can only be used on the Spresense with the FCBGA chip package.
 */

//#include <Camera.h>
#include <arch/board/board.h>
#include "lcd.h"
#include "camera_command.h"

#define BAUDRATE     (115200)
/**
 * @brief Callback from Camera library
 */

/**
 * @brief Initialize camera
 */
void setup() {
    /* Open serial communications and wait for port to open */
    Serial.begin(BAUDRATE);
    while (!Serial) {
        ; /* wait for serial port to connect. Needed for native USB port only */
    }     

    //Initalize LCD
    init_lcd();

    Serial.println("Start Camera test");

}

/**
 * @brief No procedure
 */
void loop() {
    wait_command_input();
    //usleep(1);
}
