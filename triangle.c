#include <kilolib.h>

// declare motion variable type
typedef enum {
    STOP,
    FORWARD,
    LEFT,
    RIGHT
} motion_t;

motion_t cur_motion = STOP;
uint8_t rx_kilo_id;
message_t msg;
uint8_t dist;
distance_measurement_t dist_val;
uint8_t middle_id = 1;
uint8_t desired_side_dist = 100; 
uint8_t last_dist1 = 0;
uint8_t last_dist2 = 0;
uint8_t tol = 5;

new_message = 0;

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
}

void loop() {
    if (kilo_uid == middle_id){
        if (rx_kilo_id == 0){
            last_dist1 = dist;
        } else {
            last_dist2 = dist;
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
    dist = estimate_distance(&dist);
}

message_t *message_tx(){
    return &msg;
}

void message_tx_success(){

}

int main() {
    kilo_init();
    kilo_message_rx = message_rx;
    kilo_message_tx = message_tx;
    kilo_message_tx_success = message_tx_success;
    kilo_start(setup, loop);

    return 0;
}
