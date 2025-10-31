// ----------------------------------------------------------------------------------
// Arduino UNO Sketch: Bluetooth RC Car Controller (Test Platform)
// This code controls a 4WD chassis using an L298N motor driver and HC-05/HC-06 Bluetooth module.
// ----------------------------------------------------------------------------------

#include <SoftwareSerial.h>

// --- 1. MOTOR DRIVER PIN DEFINITIONS (L298N) ---
// Left Motor Group (Motors on one side)
const int IN1_LEFT = 4; // L298N Input 1 (Motor A)
const int IN2_LEFT = 5; // L298N Input 2 (Motor A)

// Right Motor Group (Motors on the other side)
const int IN3_RIGHT = 6; // L298N Input 3 (Motor B)
const int IN4_RIGHT = 7; // L298N Input 4 (Motor B)

// Note: ENA and ENB (Speed Control) are left HIGH for full speed during testing.

// --- 2. BLUETOOTH PIN DEFINITIONS (HC-05/HC-06) ---
// Using Digital Pins 10 and 11 for Software Serial to keep D0/D1 free.
const int BT_RX_PIN = 10; // Connects to HC-05 TX
const int BT_TX_PIN = 11; // Connects to HC-05 RX (Needs level shifting if 5V)
const long BT_BAUD = 9600;

SoftwareSerial bluetoothSerial(BT_RX_PIN, BT_TX_PIN); // RX, TX

char incomingCommand = 'S'; // Default state is Stop
char lastCommand = ' '; // New variable to track the last executed command

// ----------------------------------------------------------------------------------
// MOTOR CONTROL FUNCTIONS
// ----------------------------------------------------------------------------------

// (All motor control functions remain the same, but Serial.println is moved)

void moveForward() {
    digitalWrite(IN1_LEFT, HIGH);
    digitalWrite(IN2_LEFT, LOW);
    digitalWrite(IN3_RIGHT, HIGH);
    digitalWrite(IN4_RIGHT, LOW);
}

void moveBackward() {
    digitalWrite(IN1_LEFT, LOW);
    digitalWrite(IN2_LEFT, HIGH);
    digitalWrite(IN3_RIGHT, LOW);
    digitalWrite(IN4_RIGHT, HIGH);
}

void turnLeft() {
    digitalWrite(IN1_LEFT, LOW);
    digitalWrite(IN2_LEFT, HIGH);
    digitalWrite(IN3_RIGHT, HIGH);
    digitalWrite(IN4_RIGHT, LOW);
}

void turnRight() {
    digitalWrite(IN1_LEFT, HIGH);
    digitalWrite(IN2_LEFT, LOW);
    digitalWrite(IN3_RIGHT, LOW);
    digitalWrite(IN4_RIGHT, HIGH);
}

void stopCar() {
    digitalWrite(IN1_LEFT, LOW);
    digitalWrite(IN2_LEFT, LOW);
    digitalWrite(IN3_RIGHT, LOW);
    digitalWrite(IN4_RIGHT, LOW);
}

// ----------------------------------------------------------------------------------
// SETUP
// ----------------------------------------------------------------------------------

void setup() {
    // Initialize standard serial communication for debugging via USB
    Serial.begin(115200);
    Serial.println("Arduino RC Controller Starting...");

    // Initialize the Software Serial port for Bluetooth communication
    bluetoothSerial.begin(BT_BAUD);
    Serial.println("Bluetooth Serial Ready.");

    // Set motor control pins as outputs
    pinMode(IN1_LEFT, OUTPUT);
    pinMode(IN2_LEFT, OUTPUT);
    pinMode(IN3_RIGHT, OUTPUT);
    pinMode(IN4_RIGHT, OUTPUT);

    // Ensure all motors are initially stopped
    stopCar();
}

// ----------------------------------------------------------------------------------
// MAIN LOOP
// ----------------------------------------------------------------------------------

void loop() {
    // 1. Check if a command is available from the Bluetooth module
    if (bluetoothSerial.available() > 0) {
        incomingCommand = bluetoothSerial.read();
    }

    // 2. Execute the action based on the last received command
    switch (incomingCommand) {
        case 'F': 
            moveForward();
            if (lastCommand != 'F') Serial.println("Forward"); // Print only when movement changes
            break;
        case 'B': 
            moveBackward();
            if (lastCommand != 'B') Serial.println("Backward");
            break;
        case 'L': 
            turnLeft();
            if (lastCommand != 'L') Serial.println("Turn Left");
            break;
        case 'R': 
            turnRight();
            if (lastCommand != 'R') Serial.println("Turn Right");
            break;
        case 'S': 
            stopCar();
            if (lastCommand != 'S') Serial.println("Stop");
            break;
    }
    
    // 3. Update the last command executed
    // We only update lastCommand if we received a valid command (F, B, L, R, S)
    if (incomingCommand == 'F' || incomingCommand == 'B' || incomingCommand == 'L' || 
        incomingCommand == 'R' || incomingCommand == 'S') {
        lastCommand = incomingCommand;
    }

    // Small delay to keep loop running smoothly
    delay(50);
}

