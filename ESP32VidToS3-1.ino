#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "esp_camera.h"

#define Y2_GPIO_NUM    5   // D0
#define Y3_GPIO_NUM    18  // D1
#define Y4_GPIO_NUM    19  // D2
#define Y5_GPIO_NUM    21  // D3
#define Y6_GPIO_NUM    36  // D4
#define Y7_GPIO_NUM    39  // D5
#define Y8_GPIO_NUM    34  // D6
#define Y9_GPIO_NUM    35  // D7
#define XCLK_GPIO_NUM  0   // XCLK
#define PCLK_GPIO_NUM  22  // PCLK
#define VSYNC_GPIO_NUM 25  // VSYNC
#define HREF_GPIO_NUM  23  // HREF
#define SIOD_GPIO_NUM  26  // SDA
#define SIOC_GPIO_NUM  27  // SCL
#define PWDN_GPIO_NUM  32  // PWDN
#define RESET_GPIO_NUM 33  // RESET

const char* wifiSSID = "Your WiFi SSID";
const char* wifiPassword = "Your WiFi Password";

const char* s3_bucket_url = "https://your-S3-bucket.s3.amazonaws.com/";
const char* image_filename = "image.jpg";

const char* root_ca = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";  //AWS Root CA Certificate. More info - https://www.amazontrust.com/repository/


void connectToWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(wifiSSID, wifiPassword);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
}

void sendImageToS3(camera_fb_t *fb) {
    WiFiClientSecure client;
    client.setCACert(root_ca);

    HTTPClient http;

    String url = String(s3_bucket_url) + image_filename;

    Serial.println("Uploading image to S3...");
    http.begin(client, url);
    http.addHeader("Content-Type", "image/jpeg");

    int httpResponseCode = http.PUT(fb->buf, fb->len);

    if (httpResponseCode > 0) {
        Serial.printf("Image uploaded successfully, HTTP Response code: %d\n", httpResponseCode);
    } else {
        Serial.printf("Error uploading image, HTTP Response code: %d, Error: %s\n", httpResponseCode, http.errorToString(httpResponseCode).c_str());
    }

    http.end();
}

void setupCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        config.frame_size = FRAMESIZE_QQVGA;
        config.jpeg_quality = 30;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
}

void setup() {
    Serial.begin(115200);
    connectToWiFi();
    setupCamera();
}

void loop() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }

    sendImageToS3(fb);
    esp_camera_fb_return(fb);

    delay(250); // 0.25sec refresh
}