// Define PWM pins for DRV8313 high-side inputs
#define INH_U 16
#define INH_V 18
#define INH_W 20
#define ENABLE_PIN 27

// Define I2C pins
#define SDA_PIN 4
#define SCL_PIN 5

// Gempad definitions
#define SEESAW_GPIO_BASE 0x01
#define SEESAW_GPIO_BULK 0x04
#define SEESAW_I2C_ADDR 0x50
#define SEESAW_GPIO_DIRCLR_BULK 0x03
#define SEESAW_GPIO_PULLENSET 0x0B
#define SEESAW_GPIO_BULK_SET 0x05

// AS6500 definitions
#define AS5600_ADDRESS 0x36
#define AS5600_ANGLE 0x0E

// Timer setup for angle read
#define HALL_READ_PERIOD_MS 10

// Timer setup for BLDC driver
#define BLDC_DRIVER_MS 100

#define BUTTON_X 6
#define BUTTON_Y 2
#define BUTTON_A 5
#define BUTTON_B 1
#define BUTTON_SELECT 0
#define BUTTON_START 16
// Button mask
#define BUTTON_MASK (      \
    (1 << BUTTON_X) |      \
    (1 << BUTTON_Y) |      \
    (1 << BUTTON_A) |      \
    (1 << BUTTON_B) |      \
    (1 << BUTTON_SELECT) | \
    (1 << BUTTON_START))

typedef struct
{
    bool x;
    bool y;
    bool a;
    bool b;
    bool select;
    bool start;
} ButtonStates;

#define PHASE_A_SET 1
#define PHASE_B_SET 2
#define PHASE_C_SET 3
#define MOTOR_ENABLED 4
#define MOTOR_STOP 5
#define AUTO_MODE 6

#define DEBOUNCE_SIZE 10

enum DriveMode
{
    OFF,     // 0
    PHASE_A, // 1
    PHASE_B, // 2
    PHASE_C, // 3
    AUTO     // 4
};
