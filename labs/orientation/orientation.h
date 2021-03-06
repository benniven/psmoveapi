
 /**
 * PS Move API - An interface for the PS Move Motion Controller
 * Copyright (c) 2012 Thomas Perl <m@thp.io>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 **/


#include <QApplication>
#include <QThread>
#include "qglscenenode.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "psmove.h"
#include "psmove_calibration.h"

#include "MadgwickAHRS/MadgwickAHRS.h"

class Orientation : public QThread
{
    Q_OBJECT

    signals:
        void orientation(qreal a, qreal b, qreal c, qreal d);

    public:
        Orientation() : QThread() {}

        void run()
        {
            PSMove *move = psmove_connect();
            int quit = 0;

            if (move == NULL) {
                fprintf(stderr, "Could not connect to controller.\n");
                QApplication::quit();
            }

            PSMoveCalibration *calibration = psmove_calibration_new(move);
            psmove_calibration_load(calibration);
            assert(psmove_calibration_supports_method(calibration,
                        Calibration_USB));

            while (!quit) {
                int frame;
                int input[9];
                float output[9]; // ax, ay, az, gx, gy, gz, mx, my, mz
                int seq = psmove_poll(move);

                if (seq) {

                    if (psmove_get_buttons(move) & Btn_PS) {
                        quit = 1;
                        break;
                    }

                    if (psmove_get_buttons(move) & Btn_MOVE) {
                        q0 = 1.;
                        q1 = q2 = q3 = 0.;
                    }

                    psmove_get_magnetometer(move, &input[6], &input[7], &input[8]);

                    seq -= 1;

                    for (frame=0; frame<2; frame++) {
                        psmove_get_half_frame(move, Sensor_Accelerometer,
                                (enum PSMove_Frame)frame,
                                &input[0], &input[1], &input[2]);

                        psmove_get_half_frame(move, Sensor_Gyroscope,
                                (enum PSMove_Frame)frame,
                                &input[3], &input[4], &input[5]);

                        psmove_calibration_map(calibration, input, output, 9);

                        MadgwickAHRSupdate(output[3], output[4], output[5],
                                output[0], output[1], output[2],
                                output[6], output[7], output[8]);
                    }

                    emit orientation(q0, q1, q2, q3);
                }
            }

            psmove_calibration_destroy(calibration);
            psmove_disconnect(move);
            QApplication::quit();
        }
};

