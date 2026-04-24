#include <kilolib.h>
#include <stdlib.h>

// declare motion variable type
typedef enum {
    STOP,
    FORWARD,
    LEFT,
    RIGHT
} motion_t;

// Set one side kilobot to have ID 1, other one 3
// Set middle kilobot to have ID 2
// IDs in line: 1, 2, 3

// Set distance between 1 and 3 to be desired_side_dist

motion_t cur_motion = STOP;
uint8_t rx_kilo_id;
message_t msg;
uint8_t dist;
distance_measurement_t dist_val;
uint8_t middle_id = 2;
uint8_t desired_side_dist = 60; 
uint8_t last_dist1 = 0;
uint8_t last_dist2 = 0;
uint8_t tol = 8;
uint8_t message_sent;

uint8_t new_message = 0;

void set_motion(motion_t new_motion) {
    if (cur_motion != new_motion) {
        cur_motion = new_motion;
        switch(cur_motion) {
            case STOP:
                set_motors(0,0);
                break;
            case FORWARD:
                spinup_motors();
                set_motors(kilo_straight_left, kilo_straight_right);
                break;
            case LEFT:
                spinup_motors();
                set_motors(kilo_turn_left, 0); 
                break;
            case RIGHT:
                spinup_motors();
                set_motors(0, kilo_turn_right); 
                break;
        }
    }
}

void setup() {
    msg.type = NORMAL;
    msg.data[0] = kilo_uid;
    msg.crc = message_crc(&msg);
    if (kilo_uid == 2){
        set_motion(FORWARD);
    }
}

void loop() {
    if (message_sent) {
        message_sent = 0;
        set_color(RGB(1,0,0));
        delay(20);
        set_color(RGB(0,0,0));
    }

    if (kilo_uid == middle_id){
        // debug LED pattern
        if (last_dist1 > desired_side_dist) {
            set_color(RGB(0,1,0));
            delay(50);
            set_color(RGB(0,0,0));
        }
        if (last_dist2 > desired_side_dist) {
            set_color(RGB(0,0,1));
            delay(50);
            set_color(RGB(0,0,0));
        }

        if (abs(last_dist1 - desired_side_dist) < tol && abs(last_dist2 - desired_side_dist) < tol ){
            set_motion(STOP);
        }
    }
}

void message_rx(message_t *m, distance_measurement_t *d) {
    rx_kilo_id = m->data[0];
    new_message = 1;
    dist_val = *d;
    dist = estimate_distance(&dist_val);

    if (rx_kilo_id == 1){
        last_dist1 = dist;
    } else {
        last_dist2 = dist;
    }
}

message_t *message_tx(){
    return &msg;
}

void message_tx_success(){
    message_sent = 1;
}

int main() {
    kilo_init();
    kilo_message_rx = message_rx;
    kilo_message_tx = message_tx;
    kilo_message_tx_success = message_tx_success;
    kilo_start(setup, loop);

    return 0;
}
