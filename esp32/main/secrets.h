/*
Located at :\DocumentsArduino\libraries\secrets\
create a folder of the same name

Tutorial:
https://aws.amazon.com/blogs/compute/building-an-aws-iot-core-device-using-aws-serverless-and-an-esp32/
*/
#include <pgmspace.h>

#define SECRET
#define THINGNAME "ESP32-Mk1"

const char WIFI_SSID[] = "IllinoisNet_Guest";   //need to add MAC to IllinoisNet_Guest first
const char WIFI_PASSWORD[] = "";
const char AWS_IOT_ENDPOINT[] = ".iot.us-east-2.amazonaws.com";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
)KEY";
