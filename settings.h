// WiFi Configuration
const char* ssid = "SSID_HERE";
const char* password = "PASSWORD_HERE";

// MQTT Cibfiguration
const char* mqtt_server = "192.168.1.233";
const int mqtt_port = 1883;
const char* mqtt_id = "Water_Flow_Sensor_Utility";
const char* mqtt_pub_topic_waterflow = "water/utility_flow_sensor/rate";
const char* mqtt_sub_topic_healthcheck = "water/utility_flow_sensor/heathcheck";
const char* mqtt_sub_topic_operation = "water/utility_flow_sensor/operation";

// Other params
const int update_time_sensors = 60000;
const int FlowSensorPin = 4;
