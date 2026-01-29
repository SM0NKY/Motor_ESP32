#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "GPS.h"

// micro-ROS
#include "micro_ros_serial_transport.hpp"
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/string.h>

// GPS UART2 pins
#define GPS_TX_PIN GPIO_NUM_17
#define GPS_RX_PIN GPIO_NUM_16
#define GPS_UART_PORT UART_NUM_2

// --- micro-ROS objects (static for callbacks) ---
static rcl_publisher_t gps_pub;
static std_msgs__msg__String msg;
static char msg_buffer[128];

static GPS* g_gps = nullptr;

// Timer callback: build + publish message
static void timer_callback(rcl_timer_t * timer, int64_t /*last_call_time*/)
{
    if (timer == NULL || g_gps == nullptr) return;

    // Update GPS parser (consume UART buffer)
    g_gps->process_data();
    Data_GPS current = g_gps->get_Data();

    int n = 0;
    if (current.isValid) {
        n = snprintf(
            msg_buffer, sizeof(msg_buffer),
            "Lat: %.6f | Lon: %.6f | Speed: %.2f km/h | RAW_STATUS: VALID",
            current.latitude,
            current.longitude,
            current.speed_kmh
        );
    } else {
        n = snprintf(
            msg_buffer, sizeof(msg_buffer),
            "GPS INVALID (no fix yet)"
        );
    }

    if (n < 0) return;
    if ((size_t)n >= sizeof(msg_buffer)) {
        // truncated, but still publish what fits
        n = sizeof(msg_buffer) - 1;
        msg_buffer[n] = '\0';
    }

    msg.data.size = (size_t)n;
    rcl_ret_t pub_rc = rcl_publish(&gps_pub, &msg, NULL);
    (void)pub_rc;
}

extern "C" void app_main(void)
{
    // --- GPS init ---
    static GPS gps_UART(GPS_TX_PIN, GPS_RX_PIN, GPS_UART_PORT);
    gps_UART.init();
    g_gps = &gps_UART;

    // --- micro-ROS transport init (UART0/USB) ---
    microros_serial_transport_init();

    // --- rclc init ---
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rclc_support_t support;
    rcl_ret_t rc = rclc_support_init(&support, 0, NULL, &allocator);
    if (rc != RCL_RET_OK) {
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // --- node ---
    rcl_node_t node;
    rc = rclc_node_init_default(&node, "esp32_gps_node", "", &support);
    if (rc != RCL_RET_OK) {
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // --- publisher ---
    rc = rclc_publisher_init_default(
        &gps_pub,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
        "gps/pose"   // (sin slash inicial suele ser más “ROS-style”)
    );
    if (rc != RCL_RET_OK) {
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // --- message memory ---
    std_msgs__msg__String__init(&msg);
    msg.data.data = msg_buffer;
    msg.data.capacity = sizeof(msg_buffer);
    msg.data.size = 0;
    msg_buffer[0] = '\0';

    // --- timer (1 Hz) ---
    rcl_timer_t timer;
    const uint64_t period_ns = RCL_MS_TO_NS(1000);
    rc = rclc_timer_init_default(&timer, &support, period_ns, timer_callback);
    if (rc != RCL_RET_OK) {
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // --- executor (1 handle: timer) ---
    rclc_executor_t executor;
    rc = rclc_executor_init(&executor, &support.context, 1, &allocator);
    if (rc != RCL_RET_OK) {
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    rc = rclc_executor_add_timer(&executor, &timer);
    if (rc != RCL_RET_OK) {
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // --- main loop: spin executor ---
    while (1) {
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(50));
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}