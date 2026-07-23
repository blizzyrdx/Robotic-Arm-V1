#include <Servo.h>
#include <stdio.h>

// ========================================
// Servo objects (only testing first four)
// ========================================

Servo rootServo;
Servo armA1;
Servo armA2;
Servo armB;

// ========================================
// Pin assignments
// ========================================

constexpr int ROOT_PIN = 3;
constexpr int ARM_A1_PIN = 4;
constexpr int ARM_A2_PIN = 5;
constexpr int ARM_B_PIN = 6;

// ========================================
// Root settings (misaligned Root therefore 40 degrees is "straight")
// ========================================

constexpr int ROOT_MIN = 0;
constexpr int ROOT_MAX = 180;
constexpr int ROOT_START = 40;

// ========================================
// Arm A1 and A2 settings (Needs more Calibration)
// ========================================

// Your calibrated positions:
// 130 = down
// 150 = straight up
// 180 = forward/final position
constexpr int ARM_A_MIN = 130;
constexpr int ARM_A_START = 150;
constexpr int ARM_A_MAX = 180;

// ========================================
// Arm B settings
// ========================================

// You discovered that 110° is Arm B's center.
// Start with a limited range for safety.
constexpr int ARM_B_MIN = 90;
constexpr int ARM_B_CENTER = 110;
constexpr int ARM_B_MAX = 130;

// ========================================
// Movement settings
// ========================================

constexpr unsigned long STEP_INTERVAL_MS = 20;
constexpr int STEP_SIZE = 1;

int currentRoot = ROOT_START;
int targetRoot = ROOT_START;

int currentArmA = ARM_A_START;
int targetArmA = ARM_A_START;

int currentArmB = ARM_B_CENTER;
int targetArmB = ARM_B_CENTER;

unsigned long previousMoveTime = 0;

// ========================================
// Serial buffer
// ========================================

char receiveBuffer[64];
size_t receivePosition = 0;

// ========================================
// Target calculations
// ========================================

void setRootTarget(int sliderAngle)
{
    targetRoot = constrain(
        sliderAngle,
        ROOT_MIN,
        ROOT_MAX
    );
}

void setArmATarget(int sliderAngle)
{
    sliderAngle = constrain(sliderAngle, 0, 180);

    // SDL slider 0–180 becomes servo angle 130–180.
    targetArmA = map(
        sliderAngle,
        0,
        180,
        ARM_A_MIN,
        ARM_A_MAX
    );

    targetArmA = constrain(
        targetArmA,
        ARM_A_MIN,
        ARM_A_MAX
    );
}

void setArmBTarget(int sliderAngle)
{
    sliderAngle = constrain(sliderAngle, 0, 180);

    // SDL slider 0–180 becomes servo angle 90–130.
    // Therefore, SDL slider 90 becomes servo angle 110.
    targetArmB = map(
        sliderAngle,
        0,
        180,
        ARM_B_MIN,
        ARM_B_MAX
    );

    targetArmB = constrain(
        targetArmB,
        ARM_B_MIN,
        ARM_B_MAX
    );
}

// ========================================
// Gradual movement helper
// ========================================

void moveToward(int& currentAngle, int targetAngle)
{
    if (currentAngle < targetAngle)
    {
        currentAngle += STEP_SIZE;

        if (currentAngle > targetAngle)
        {
            currentAngle = targetAngle;
        }
    }
    else if (currentAngle > targetAngle)
    {
        currentAngle -= STEP_SIZE;

        if (currentAngle < targetAngle)
        {
            currentAngle = targetAngle;
        }
    }
}

void moveServosGradually()
{
    if (millis() - previousMoveTime < STEP_INTERVAL_MS)
    {
        return;
    }

    previousMoveTime = millis();

    moveToward(currentRoot, targetRoot);
    moveToward(currentArmA, targetArmA);
    moveToward(currentArmB, targetArmB);

    rootServo.write(currentRoot);

    // Both Arm A servos receive the same angle.
    armA1.write(currentArmA);
    armA2.write(currentArmA);

    armB.write(currentArmB);
}

// ========================================
// Process commands from SDL3
// ========================================

void processCommand()
{
    receiveBuffer[receivePosition] = '\0';

    int rootValue;
    int armAValue;
    int armBValue;
    int wristAValue;
    int wristBValue;
    int gripperValue;

    const int valuesRead = sscanf(
        receiveBuffer,
        "%d,%d,%d,%d,%d,%d",
        &rootValue,
        &armAValue,
        &armBValue,
        &wristAValue,
        &wristBValue,
        &gripperValue
    );

    if (valuesRead == 6)
    {
        // First SDL slider.
        setRootTarget(rootValue);

        // Second SDL slider.
        setArmATarget(armAValue);

        // Third SDL slider.
        setArmBTarget(armBValue);

        Serial.print("OK:Root=");
        Serial.print(targetRoot);

        Serial.print(",ArmA=");
        Serial.print(targetArmA);

        Serial.print(",ArmB=");
        Serial.println(targetArmB);
    }
    else
    {
        Serial.println(
            "ERR:Expected six comma-separated angles"
        );
    }

    receivePosition = 0;
}

// ========================================
// Read serial commands
// ========================================

void readSerialCommands()
{
    while (Serial.available() > 0)
    {
        const char receivedCharacter =
            static_cast<char>(Serial.read());

        if (receivedCharacter == '\n')
        {
            if (receivePosition > 0)
            {
                processCommand();
            }
        }
        else if (receivedCharacter != '\r')
        {
            if (
                receivePosition <
                sizeof(receiveBuffer) - 1
            )
            {
                receiveBuffer[receivePosition] =
                    receivedCharacter;

                receivePosition++;
            }
            else
            {
                receivePosition = 0;
                Serial.println("ERR:Command too long");
            }
        }
    }
}

// ========================================
// Setup
// ========================================

void setup()
{
    Serial.begin(9600);

    // Store starting positions before attaching.
    rootServo.write(ROOT_START);

    armA1.write(ARM_A_START);
    armA2.write(ARM_A_START);

    armB.write(ARM_B_CENTER);

    rootServo.attach(ROOT_PIN);
    armA1.attach(ARM_A1_PIN);
    armA2.attach(ARM_A2_PIN);
    armB.attach(ARM_B_PIN);

    currentRoot = ROOT_START;
    targetRoot = ROOT_START;

    currentArmA = ARM_A_START;
    targetArmA = ARM_A_START;

    currentArmB = ARM_B_CENTER;
    targetArmB = ARM_B_CENTER;

    Serial.println("READY");
}

// ========================================
// Main loop
// ========================================

void loop()
{
    readSerialCommands();
    moveServosGradually();
}
