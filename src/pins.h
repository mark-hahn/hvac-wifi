#ifndef PINS_H
#define PINS_H

// esp32-cam pins
#define POWER_GPIO_NUM     2
#define FLASH_GPIO_NUM     4
#define BEEP_GPIO_NUM     12
#define BUTTON_GPIO_NUM   13 
#define I2C_SCL_GPIO_NUM  15 
#define I2C_SDA_GPIO_NUM  14 


// Pin definition for CAMERA_MODEL_AI_THINKER
// duplicated in src/ESP32CameraPins.h
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#endif // PINS_H
