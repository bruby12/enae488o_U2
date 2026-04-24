#include <kilolib.h>

// This needs to be small to avoid communication range issues
uint8_t TOOCLOSE_DISTANCE = 30; // 30 mm
uint8_t DESIRED_DISTANCE = 46; // 46 mm

// declare motion variable type
typedef enum {
    STOP,
    FORWARD,
    LEFT,
    RIGHT
} motion_t;

// declare state variable type
typedef enum {
    ORBIT_TOOCLOSE,
    ORBIT_NORMAL,
} orbit_state_t;

motion_t cur_motion = STOP;
orbit_state_t orbit_state = ORBIT_NORMAL;
uint8_t cur_distance = 0;

// Message handling
message_t msg_tx;
message_t msg_rx;
uint8_t new_message = 0;
distance_measurement_t dist;

uint8_t orbit_target = 255;
uint8_t other_mover = 255;
uint8_t other_target = 255;
uint8_t orbit_cw = 1;

// function to set new motion
// copied from orbit-planet
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

// Copied from orbit-planet with modifications for CW/CCW switching
// Also inclued some colors so the kilobots look cool
void orbit_normal() {
    if (cur_distance < TOOCLOSE_DISTANCE) {
        orbit_state = ORBIT_TOOCLOSE;
    } 
    else {
        if (cur_distance < DESIRED_DISTANCE){
            set_color(RGB(1,0,0));
            if(orbit_cw)
                set_motion(LEFT);
            else
                set_motion(RIGHT);
        }
        else{
            set_color(RGB(0,1,0));
            if(orbit_cw)
                set_motion(RIGHT);
            else
                set_motion(LEFT);
        }
    }
}

// copied from orbit-planet
// Also inclued some colors so the kilobots look cools
void orbit_tooclose() {
    if (cur_distance >= DESIRED_DISTANCE)
        orbit_state = ORBIT_NORMAL;
    else{
        set_motion(FORWARD);
        set_color(RGB(1,1,0));
    }
}

void setup() {
    // We only need to transmit our kilo_uid
    msg_tx.type = NORMAL;
    msg_tx.data[0] = kilo_uid;
    msg_tx.crc = message_crc(&msg_tx);

    // Commands depending on kilo_uid
    if(kilo_uid == 0){
        orbit_target = 1;
        other_mover = 3;
        other_target = 2;
        orbit_cw = 1;
    }
    else if(kilo_uid == 3){
        orbit_target = 2;
        other_mover = 0;
        other_target = 1;
        orbit_cw = 0;
    }
    else{
        set_motion(STOP);
        set_color(RGB(0,0,1));
    }
}

uint8_t done = 0;
void loop() {
    // Update distance estimate with every message
    if (new_message) {
        new_message = 0;
        cur_distance = estimate_distance(&dist);
        
        if((kilo_uid == 1) || (kilo_uid == 2) || done){
            set_motion(STOP);
            return;
        }

        // Orbit if message is from our target
        if(msg_rx.data[0] == orbit_target){
            switch(orbit_state) {
                case ORBIT_NORMAL:
                    orbit_normal();
                    break;
                case ORBIT_TOOCLOSE:
                    orbit_tooclose();
                    break;
            }
        }

        // Stop moving once we get within 1 side length of the other moving robot
        else if(msg_rx.data[0] == other_target){
            if(cur_distance <= DESIRED_DISTANCE*1.414){ // sqrt(2)*side length should be diagonal length for a square
                set_motion(STOP);
                done = 1;
                set_color(RGB(0,0,1));
            }
        }
    }    
}

void message_rx(message_t *m, distance_measurement_t *d) {
    msg_rx = *m;
    dist = *d;
    new_message = 1;
}

message_t *message_tx(){
    return &msg_tx;
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
