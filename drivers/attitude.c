#include <math.h>
#include "attitude.h"

#define RAD_TO_DEG       57.2957795f
#define DEG_TO_RAD       0.0174532925f

#define FILTER_ALPHA     0.98f
#define MIN_ACCEL        1000.0f
#define MIN_COS_PITCH    0.20f



static float wrap_degrees(float angle)
{
    while (angle > 180.0f) {
        angle -= 360.0f;
    }

    while (angle < -180.0f) {
        angle += 360.0f;
    }

    return angle;
}


static int calculate_acc_angles(int16_t accel_x,
                                int16_t accel_y,
                                int16_t accel_z,
                                float *pitch,
                                float *roll)
{
    float x = (float)accel_x;
    float y = (float)accel_y;
    float z = (float)accel_z;

    float magnitude = sqrtf(x * x + y * y + z * z);

    if (magnitude < MIN_ACCEL) {
        return -1;
    }

    /*
     * Rotation around sensor Y axis.
     */
    *pitch = atan2f(z, x) * RAD_TO_DEG;

    /*
     * Rotation around sensor Z axis.
     */
    *roll = atan2f(-y, sqrtf(x * x + z * z)) * RAD_TO_DEG;

    return 0;
}


int attitude_init(attitude_t *attitude,
                  int16_t accel_x,
                  int16_t accel_y,
                  int16_t accel_z)
{
    float pitch;
    float roll;

    if (attitude == 0) {
        return -1;
    }

    if (calculate_acc_angles(accel_x,
                             accel_y,
                             accel_z,
                             &pitch,
                             &roll) != 0) {
        return -1;
    }

    attitude->pitch_zero_deg = pitch;
    attitude->roll_zero_deg = roll;

    attitude->pitch_deg = 0.0f;
    attitude->roll_deg = 0.0f;

    attitude->initialized = 1U;

    return 0;
}


int attitude_update(attitude_t *attitude,
                    int16_t accel_x,
                    int16_t accel_y,
                    int16_t accel_z,
                    float gyro_pitch_dps,
                    float gyro_roll_dps,
                    float dt)
{
    float pitch_acc;
    float roll_acc;

    float pitch_predicted;
    float roll_predicted;

    float absolute_pitch;
    float cos_pitch;

    float roll_rate;

    if ((attitude == 0) ||
        (attitude->initialized == 0U) ||
        (dt <= 0.0f)) {
        return -1;
    }

\
    if (dt > 0.05f) {
        dt = 0.05f;
    }


    if (calculate_acc_angles(accel_x,
                             accel_y,
                             accel_z,
                             &pitch_acc,
                             &roll_acc) != 0) {
        return -1;
    }

    pitch_acc = wrap_degrees(
        pitch_acc - attitude->pitch_zero_deg
    );

    roll_acc = wrap_degrees(
        roll_acc - attitude->roll_zero_deg
    );

\
    pitch_predicted =
        attitude->pitch_deg + gyro_pitch_dps * dt;

    absolute_pitch =
        pitch_predicted + attitude->pitch_zero_deg;

    cos_pitch = cosf(absolute_pitch * DEG_TO_RAD);

    if (fabsf(cos_pitch) < MIN_COS_PITCH) {
        cos_pitch = (cos_pitch < 0.0f)
                    ? -MIN_COS_PITCH
                    : MIN_COS_PITCH;
    }

    roll_rate = gyro_roll_dps / cos_pitch;

    roll_predicted =
        attitude->roll_deg + roll_rate * dt;

    attitude->pitch_deg = wrap_degrees(
        pitch_predicted +
        (1.0f - FILTER_ALPHA) *
        wrap_degrees(pitch_acc - pitch_predicted)
    );

    attitude->roll_deg = wrap_degrees(
        roll_predicted +
        (1.0f - FILTER_ALPHA) *
        wrap_degrees(roll_acc - roll_predicted)
    );

    return 0;
}